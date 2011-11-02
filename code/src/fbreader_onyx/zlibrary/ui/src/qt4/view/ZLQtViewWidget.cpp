/*
* Copyright (C) 2004-2009 Geometer Plus <contact@geometerplus.com>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
* 02110-1301, USA.
*/

#include <algorithm>

#include <QtGui/QLayout>
#include <QtGui/QScrollBar>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QMouseEvent>

#include <ZLibrary.h>
#include <ZLLanguageUtil.h>
#include <ZLUnicodeUtil.h>

#include "ZLQtViewWidget.h"
#include "ZLQtPaintContext.h"
#include "ZLTextStyle.h"
#include "ZLTextView.h"
#include "../dialogs/ZLFileListDialog.h"
#include "../dialogs/ZLLinkInfoDialog.h"

#include "onyx/sys/platform.h"
#include "onyx/sys/sys.h"
#include "onyx/screen/screen_proxy.h"
#include "onyx/screen/screen_update_watcher.h"
#include "onyx/data/configuration.h"
#include "onyx/ui/tree_view_dialog.h"
#include "onyx/ui/brightness_dialog.h"
#include "onyx/cms/content_thumbnail.h"

using namespace cms;
using namespace vbf;

static const int BEFORE_SEARCH = 0;
static const int IN_SEARCHING  = 1;
static bool has_touch = true;

class MyQScrollBar : public QScrollBar {

public:
    MyQScrollBar(Qt::Orientation orientation, QWidget *parent) : QScrollBar(orientation, parent) {
    }

private:
    void mouseMoveEvent(QMouseEvent *event) {
        if (orientation() == Qt::Vertical) {
            const int y = event->y();
            if ((y <= 0) || (y >= height())) {
                return;
            }
        } else {
            const int x = event->x();
            if ((x <= 0) || (x >= width())) {
                return;
            }
        }
        QScrollBar::mouseMoveEvent(event);
    }
};

static int shift(const int step)
{
    static const int SHIFT = 10;
    return SHIFT;
}

ZLQtViewWidget::Widget::Widget(QWidget *parent, ZLQtViewWidget &holder) : QWidget(parent), myHolder(holder) {
    //setBackgroundMode(NoBackground);
}

QWidget * ZLQtViewWidget::addStatusBar()
{
    if (has_touch)
    {
#ifdef BUILD_WITH_TFT
        status_bar_ = new StatusBar(widget(), ui::MENU|PROGRESS|MESSAGE|CLOCK|BATTERY);
#else
        status_bar_ = new StatusBar(widget(), ui::MENU|PROGRESS|MESSAGE|CLOCK|BATTERY|SCREEN_REFRESH);
#endif
    }
    else
    {
        status_bar_ = new StatusBar(widget(), ui::MENU|PROGRESS|MESSAGE|CLOCK|BATTERY);
    }

    connect(status_bar_, SIGNAL(menuClicked()), this, SLOT(popupMenu()));
    connect(status_bar_, SIGNAL(progressClicked(const int, const int)),
        this, SLOT(onProgressClicked(const int, const int)));
    return status_bar_;
}

ZLQtViewWidget::ZLQtViewWidget(QWidget *parent, ZLApplication *application)
: ZLViewWidget((ZLView::Angle)application->AngleStateOption.value())
, myApplication(application)
, hyperlink_selected_(false)
, status_bar_(0)
, sys_status_(sys::SysStatus::instance())
, enable_text_selection_(false)
, conf_stored_(false)
, point_(0,0)
{
    has_touch = sys_status_.hasTouchScreen();

    myFrame = new QWidget(parent);
    QGridLayout *layout = new QGridLayout();
    layout->setMargin(0);
    layout->setSpacing(0);
    myFrame->setLayout(layout);
    myQWidget = new Widget(myFrame, *this);
    layout->addWidget(myQWidget, 0, 0);
    layout->addWidget(addStatusBar(), 1, 0);

    // Setup connection
    connect(&sys_status_, SIGNAL(sdCardChangedSignal(bool)), this, SLOT(onSdCardChanged(bool)));
    connect(&sys_status_, SIGNAL(mountTreeSignal(bool, const QString &)),
            this, SLOT(handleMountTreeEvent(bool, const QString &)));
    connect(&sys_status_, SIGNAL(aboutToShutdown()), this, SLOT(onAboutToShutdown()));
    connect(&sys_status_, SIGNAL(wakeup()), this, SLOT(onWakeup()));
    connect(&sys_status_, SIGNAL(musicPlayerStateChanged(int)),
            this, SLOT(onMusicPlayerStateChanged(int)));
    connect(&sys_status_, SIGNAL(volumeChanged(int, bool)), this, SLOT(onVolumeChanged(int, bool)));

    connect(&sys_status_, SIGNAL(mouseLongPress(QPoint, QSize)), this, SLOT(onMouseLongPress(QPoint, QSize)));
    connect(&sys_status_, SIGNAL(multiTouchPressDetected(QRect, QRect)), this, SLOT(onMultiTouchPressDetected(QRect, QRect)));
    connect(&sys_status_, SIGNAL(multiTouchReleaseDetected(QRect, QRect)), this, SLOT(onMultiTouchReleaseDetected(QRect, QRect)));

    // Load conf.
    loadConf();
}

ZLQtViewWidget::~ZLQtViewWidget()
{
    // Should release them as the parent widget can release the
    // resouce. So scoped_ptr should not release them again.
    dict_widget_.release();
    tts_widget_.release();
    search_widget_.release();
}

void ZLQtViewWidget::trackStylus(bool track) {
    myQWidget->setMouseTracking(track);
}

void ZLQtViewWidget::Widget::paintEvent(QPaintEvent*) {
    ZLQtPaintContext &context = (ZLQtPaintContext&)myHolder.view()->context();
    switch (myHolder.rotation()) {
        default:
            context.setSize(width(), height());
            break;
        case ZLView::DEGREES90:
        case ZLView::DEGREES270:
            context.setSize(height(), width());
            break;
    }
    myHolder.view()->paint();
    QPainter realPainter(this);
    switch (myHolder.rotation()) {
        default:
            realPainter.drawPixmap(0, 0, context.pixmap());
            break;
        case ZLView::DEGREES90:
            realPainter.rotate(270);
            realPainter.drawPixmap(1 - height(), -1, context.pixmap());
            break;
        case ZLView::DEGREES180:
            realPainter.rotate(180);
            realPainter.drawPixmap(1 - width(), 1 - height(), context.pixmap());
            break;
        case ZLView::DEGREES270:
            realPainter.rotate(90);
            realPainter.drawPixmap(-1, 1 - width(), context.pixmap());
            break;
    }
    if (myHolder.hasBookmark())
    {
        drawBookmark(realPainter);
    }

    // Store thumbnail if necessary.
    static bool checked = false;
    if (!checked && !myHolder.myApplication->has_thumbnail && myHolder.myApplication->content_ready)
    {
        checked = true;
        myHolder.myApplication->has_thumbnail = true;
        myHolder.storeThumbnail(context.pixmap());
    }
}

void ZLQtViewWidget::Widget::drawBookmark(QPainter &painter)
{
    static QImage image(":/images/bookmark_flag.png");
    QPoint pt(rect().width()- image.width(), 0);
    painter.drawImage(pt, image);
}

void ZLQtViewWidget::Widget::mousePressEvent(QMouseEvent *event) {
    event->accept();
    last_pos_ = event->pos();

    // myHolder.view()->onStylusMove(x(event), y(event));
    // myHolder.view()->onStylusPress(x(event), y(event));
}

void ZLQtViewWidget::Widget::mouseReleaseEvent(QMouseEvent *event) {
    // myHolder.view()->onStylusRelease(x(event), y(event));
    stylusPan(event->pos(), last_pos_);
}

void ZLQtViewWidget::Widget::mouseMoveEvent(QMouseEvent *event) {
    event->accept();
    /*
    switch (event->buttons()) {
        case Qt::LeftButton:
            myHolder.view()->onStylusMovePressed(x(event), y(event));
            break;
        case Qt::NoButton:
            myHolder.view()->onStylusMove(x(event), y(event));
            break;
        default:
            break;
    }
    */
}

void ZLQtViewWidget::Widget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (!sys::isIRTouch())
    {
        myHolder.lookup();
    }
}

void ZLQtViewWidget::Widget::keyReleaseEvent(QKeyEvent *event)
{
    myHolder.view()->processKeyReleaseEvent(event->key());
}

int ZLQtViewWidget::Widget::x(const QMouseEvent *event) const {
    const int maxX = width() - 1;
    const int maxY = height() - 1;
    switch (myHolder.rotation()) {
        default:
            return std::min(std::max(event->x(), 0), maxX);
        case ZLView::DEGREES90:
            return maxY - std::min(std::max(event->y(), 0), maxY);
        case ZLView::DEGREES180:
            return maxX - std::min(std::max(event->x(), 0), maxX);
        case ZLView::DEGREES270:
            return std::min(std::max(event->y(), 0), maxY);
    }
}

int ZLQtViewWidget::Widget::y(const QMouseEvent *event) const {
    const int maxX = width() - 1;
    const int maxY = height() - 1;
    switch (myHolder.rotation()) {
        default:
            return std::min(std::max(event->y(), 0), maxY);
        case ZLView::DEGREES90:
            return std::min(std::max(event->x(), 0), maxX);
        case ZLView::DEGREES180:
            return maxY - std::min(std::max(event->y(), 0), maxY);
        case ZLView::DEGREES270:
            return maxX - std::min(std::max(event->x(), 0), maxX);
    }
}

void ZLQtViewWidget::enableTextSelection(bool enable)
{
    myApplication->enableTextSelection(enable);
}

bool ZLQtViewWidget::isTextSelectionEnabled()
{
    return myApplication->isTextSelectionEnabled();
}

void ZLQtViewWidget::Widget::stylusPan(const QPoint &now, const QPoint &old)
{
    int direction = sys::SystemConfig::direction(old, now);

    if (direction > 0)
    {
        myHolder.nextPage();
    }
    else if (direction < 0)
    {
        myHolder.prevPage();
    }
    else if (myHolder.view()->onStylusMove(now.x(), now.y()))
    {
        ZLTextView *ptr = static_cast<ZLTextView *>(myHolder.view().get());
        ptr->selectionModel().clear();
        myHolder.hyperlink_selected_ = false;    
        myHolder.view()->openInternalLink(now.x(), now.y());
        this->repaint();
        onyx::screen::instance().updateWidget(myHolder.widget(),
                        onyx::screen::ScreenProxy::GC);
    }
    else if (myHolder.isTextSelectionEnabled())
    {
        onyx::screen::ScreenProxy::Waveform w = onyx::screen::instance().defaultWaveform();
        onyx::screen::instance().setDefaultWaveform(onyx::screen::ScreenProxy::DW);
        bool ok = myHolder.view()->onStylusRelease(now.x(), now.y());
        onyx::screen::instance().setDefaultWaveform(w);
        if (ok)
        {
            myHolder.lookup();
        }
    }
    else
    {
        direction = sys::SystemConfig::whichArea(old, now);
        if (direction > 0)
        {
            myHolder.nextPage();
        }
        else if (direction < 0)
        {
            myHolder.prevPage();
        }
    }
}

void ZLQtViewWidget::repaint()
{
    widget()->repaint();
}

void ZLQtViewWidget::setScrollbarEnabled(ZLView::Direction direction, bool enabled) {
}

void ZLQtViewWidget::setScrollbarPlacement(ZLView::Direction direction, bool standard) {
}

void ZLQtViewWidget::setScrollbarParameters(ZLView::Direction direction, size_t full, size_t from, size_t to) {
    if (status_bar_ && direction == ZLView::VERTICAL)
    {
        full_ = full;
        from_ = from;
        to_ = to;
        page_step_ = to_ - from_;
        updateProgress(full, from, to);
    }

    /*
    QScrollBar *bar =
    (direction == ZLView::VERTICAL) ?
    (myShowScrollBarAtRight ? myRightScrollBar : myLeftScrollBar) :
    (myShowScrollBarAtBottom ? myBottomScrollBar : myTopScrollBar);
    bar->setMinimum(0);
    bar->setMaximum(full + from - to);
    bar->setValue(from);
    bar->setPageStep(to - from);
    */
}

QWidget *ZLQtViewWidget::widget() {
    return myFrame;
}

void ZLQtViewWidget::updateActions()
{
    // Encoding actions.
    std::string encoding = myApplication->EncodingOption.value();
    if (encoding == "GBK")
    {
        encoding = "GB18030";
    }
    encoding_actions_.generateActions(encoding);

    // Font family.
    QFont font = currentFont();
    font_family_actions_.generateActions(font.family(), true);

    // size
    std::vector<int> size;
    font_actions_.generateActions(font, size, font.pointSize());

    // Reading tools
    std::vector<ReadingToolsType> tools;
    tools.push_back(SEARCH_TOOL);
    if (has_touch || sys::SysStatus::instance().isDictionaryEnabled())
    {
        tools.push_back(DICTIONARY_TOOL);
    }

    // TODO: John: add toc later.
    tools.push_back(TOC_VIEW_TOOL);
    reading_tool_actions_.generateActions(tools);

    // Reading style
    ZLIntegerOption &option = ZLTextStyleCollection::instance().baseStyle().LineSpacePercentOption;
    int index = STYLE_LINE_SPACING_10 + (option.value() - 100) / 10;
    reading_style_actions_.generateActions(static_cast<ReadingStyleType>(index));

    if (has_touch || sys::SysStatus::instance().isTTSEnabled())
    {
        tools.clear();
        tools.push_back(TEXT_TO_SPEECH);
        reading_tool_actions_.generateActions(tools, true);
    }

    // Reading tools of bookmark.
    tools.clear();
    tools.push_back(ADD_BOOKMARK);
    tools.push_back(DELETE_BOOKMARK);
    tools.push_back(SHOW_ALL_BOOKMARKS);
    reading_tool_actions_.generateActions(tools, true);
    bool has_bookmark = hasBookmark();
    reading_tool_actions_.action(ADD_BOOKMARK)->setEnabled(!has_bookmark);
    reading_tool_actions_.action(DELETE_BOOKMARK)->setEnabled(has_bookmark);



    // Reading tools of go to page.
    tools.clear();
    tools.push_back(GOTO_PAGE);
    tools.push_back(CLOCK_TOOL);
    reading_tool_actions_.generateActions(tools, true);
    int total = (full_ >> shift(page_step_));
    reading_tool_actions_.action(GOTO_PAGE)->setEnabled(total > 1);

    std::vector<int> all = std::vector<int>();
    all.push_back(ROTATE_SCREEN);
    if (this->status_bar_->isHidden())
    {
        all.push_back(EXIT_FULL_SCREEN);
    }
    else
    {
        all.push_back(FULL_SCREEN);
    }
    all.push_back(MUSIC);
#ifdef BUILD_WITH_TFT
    all.push_back(BACKLIGHT_BRIGHTNESS);
#endif
    all.push_back(RETURN_TO_LIBRARY);
    system_actions_.generateActions(all);
}

void ZLQtViewWidget::popupMenu()
{
    ui::PopupMenu menu(widget());
    updateActions();
    menu.addGroup(&font_family_actions_);
    menu.addGroup(&font_actions_);
    menu.addGroup(&reading_style_actions_);
    menu.addGroup(&encoding_actions_);
    menu.addGroup(&reading_tool_actions_);
    menu.setSystemAction(&system_actions_);
    if (menu.popup() != QDialog::Accepted)
    {
        return;
    }

    if (sys::isImx508())
    {
       // TODO: need to fix it in kernel driver, update timing issue.
#ifdef BUILD_FOR_ARM
        usleep(1000 * 200);
#endif
    }

    QAction * group = menu.selectedCategory();
    if (group == system_actions_.category())
    {
        SystemAction system = system_actions_.selected();
        if (system == RETURN_TO_LIBRARY)
        {
            quit();
        }
        else if (system == SCREEN_UPDATE_TYPE)
        {
            onyx::screen::instance().updateWidget(widget(), onyx::screen::ScreenProxy::GU);
            onyx::screen::instance().toggleWaveform();
        }
        else if (system == FULL_SCREEN)
        {
            status_bar_->hide();
            myApplication->refreshWindow();
        }
        else if (system == EXIT_FULL_SCREEN)
        {
            status_bar_->show();
            myApplication->refreshWindow();
        }
        else if (system == MUSIC)
        {
            // Start or show music player.
            onyx::screen::instance().flush(widget(), onyx::screen::ScreenProxy::GU);
            sys::SysStatus::instance().requestMusicPlayer(sys::START_PLAYER);
        }
        else if (system == ROTATE_SCREEN)
        {
            rotateScreen();
        }
        else if (system == BACKLIGHT_BRIGHTNESS)
        {
            ui::BrightnessDialog dialog(widget());
            dialog.exec();
        }
        return;
    }

    if (group == encoding_actions_.category())
    {
        onSetEncoding(encoding_actions_.encoding());
    }
    else if (group == font_family_actions_.category())
    {
        changeFontFamily(font_family_actions_.selectedFont().toStdString());
    }
    else if (group == font_actions_.category())
    {
        changeFont(font_actions_.selectedFont());
    }
    else if (group == reading_tool_actions_.category())
    {
        if (reading_tool_actions_.selectedTool() == DICTIONARY_TOOL)
        {
            startDictLookup();
        }
        else if (reading_tool_actions_.selectedTool() == TEXT_TO_SPEECH)
        {
            onyx::screen::instance().updateWidget(widget(), onyx::screen::ScreenProxy::GU);
            startTTS();
        }
        else if (reading_tool_actions_.selectedTool() == SEARCH_TOOL)
        {
            showSearchWidget();
        }
        else if (reading_tool_actions_.selectedTool() == GOTO_PAGE)
        {
            showGotoPageDialog();
        }
        else if (reading_tool_actions_.selectedTool() == CLOCK_TOOL)
        {
            onyx::screen::instance().updateWidget(0, onyx::screen::ScreenProxy::GU);
            status_bar_->onClockClicked();
        }
        else if (reading_tool_actions_.selectedTool() == TOC_VIEW_TOOL)
        {
            showTableOfContents();
        }
        else
        {
            processBookmarks(reading_tool_actions_);
        }
    }
    else if (group == reading_style_actions_.category())
    {
        ReadingStyleType s = reading_style_actions_.selected();
        if (STYLE_LINE_SPACING_8 <= s && STYLE_LINE_SPACING_20 >= s)
        {
            changeLineSpacing((s - STYLE_LINE_SPACING_8));
        }
        else if (STYLE_PAGE_MARGINS_SMALL <= s && STYLE_PAGE_MARGINS_LARGE >= s)
        {
            changePageMargins(s - STYLE_PAGE_MARGINS_SMALL);
        }
        else if (STYLE_BLACK_BACKGROUND <= s && STYLE_WHITE_BACKGROUND >= s)
        {
            changeReadingScheme(s);
        }
    }
}

bool ZLQtViewWidget::addBookmark()
{
    ZLTextView *ptr = static_cast<ZLTextView *>(view().get());
    std::string text;
    ptr->selectionModel().text(text, 100);
    QString title = QString::fromUtf8(text.c_str());

    vbf::Bookmark new_bookmark(title, from_);
    myApplication->addBookmark(new_bookmark);
    myApplication->refreshWindow();

    // Have to refresh again.
    myApplication->refreshWindow();
    return true;
}

bool ZLQtViewWidget::removeBookmarks()
{
    myApplication->removeBookmarks(from_, to_);
    myApplication->refreshWindow();

    // Have to refresh window.
    myApplication->refreshWindow();
    return true;
}

bool ZLQtViewWidget::clearBookmarks()
{
    myApplication->clearBookmarks();
    myApplication->refreshWindow();
    return true;
}

void ZLQtViewWidget::bookmarkModel(QStandardItemModel & model,
                                   QModelIndex & selected)
{
    model.setColumnCount(2);
    Bookmarks & all = myApplication->bookmarks();

    BookmarksIter begin = all.begin();
    BookmarksIter end   = all.end();
    int row = 0;
    for(BookmarksIter iter  = begin; iter != end; ++iter, ++row)
    {
        QStandardItem *title = new QStandardItem(iter->title());
        title->setData(iter->data());
        title->setEditable(false);
        model.setItem(row, 0, title);

        int pos = iter->data().toInt();
        QString str(tr("%1"));
        pos = (pos >> shift(page_step_));
        if (pos <= 0) ++pos;
        str = str.arg(pos);
        QStandardItem *page = new QStandardItem(str);
        page->setTextAlignment(Qt::AlignCenter);
        page->setEditable(false);
        model.setItem(row, 1, page);
    }
}

void ZLQtViewWidget::showAllBookmarks()
{
    QStandardItemModel model;
    QModelIndex selected = model.invisibleRootItem()->index();
    bookmarkModel(model, selected);

    TreeViewDialog bookmark_view(widget());
    bookmark_view.setModel(&model);
    QVector<int> percentages;
    percentages.push_back(80);
    percentages.push_back(20);
    bookmark_view.tree().setColumnWidth(percentages);
    int ret = bookmark_view.popup(QCoreApplication::tr("Bookmarks"));

    // Returned from the bookmark view.
    onyx::screen::instance().flush();

    if (ret != QDialog::Accepted)
    {
        onyx::screen::instance().updateWidget(0);
        return;
    }

    int pos = model.data(bookmark_view.selectedItem(), Qt::UserRole + 1).toInt();
    onScrollbarMoved(ZLView::VERTICAL, full_, pos, pos + page_step_);
}

bool ZLQtViewWidget::hasBookmark()
{
    return myApplication->hasBookmark(from_, to_);
}

void ZLQtViewWidget::processBookmarks(ReadingToolsActions & actions)
{
    if (actions.selectedTool() == ADD_BOOKMARK)
    {
        addBookmark();
    }
    else if (actions.selectedTool() == DELETE_BOOKMARK)
    {
        removeBookmarks();
    }
    else if (actions.selectedTool() == SHOW_ALL_BOOKMARKS)
    {
        showAllBookmarks();
    }
}


void ZLQtViewWidget::onProgressClicked(const int percentage,
                                       const int value)
{
    from_ = (value << shift(page_step_));
    to_ = from_ + page_step_;

    // first page
    if (percentage <= 0)
    {
        from_ = 0;
    }

    // last page
    if (to_ >= full_)
    {
        to_ = full_;
    }
    // John: ask framework to update status bar, so we don't need to process it here.
    // updateProgress(full_, from_, to_);
    onScrollbarMoved(ZLView::VERTICAL, full_, from_, to_);
}

void ZLQtViewWidget::onSetEncoding(std::string encoding)
{
    // Fallback to GBK if necessary.
    if (encoding == "GB18030")
    {
        encoding = "GBK";
    }
    myApplication->EncodingOption.setValue(encoding);
    myApplication->doAction("changeEncoding");
}

void ZLQtViewWidget::changeFontFamily(const std::string & family)
{
    ZLStringOption &familyOption = ZLTextStyleCollection::instance().baseStyle().FontFamilyOption;
    familyOption.setValue(family);
    myApplication->doAction("updateOptions");
}

QFont ZLQtViewWidget::currentFont()
{
    QFont font;
    ZLStringOption &familyOption = ZLTextStyleCollection::instance().baseStyle().FontFamilyOption;
    font.setFamily(familyOption.value().c_str());

    ZLIntegerRangeOption &sizeOption = ZLTextStyleCollection::instance().baseStyle().FontSizeOption;
    font.setPointSize(sizeOption.value());

    ZLBooleanOption &italicOption = ZLTextStyleCollection::instance().baseStyle().ItalicOption;
    font.setItalic(italicOption.value());

    ZLBooleanOption &boldOption = ZLTextStyleCollection::instance().baseStyle().BoldOption;
    font.setBold(boldOption.value());
    return font;
}

void ZLQtViewWidget::changeFont(QFont font)
{
    ZLIntegerRangeOption &sizeOption = ZLTextStyleCollection::instance().baseStyle().FontSizeOption;
    sizeOption.setValue(font.pointSize());

    ZLBooleanOption &italicOption = ZLTextStyleCollection::instance().baseStyle().ItalicOption;
    italicOption.setValue(font.italic());

    ZLBooleanOption &boldOption = ZLTextStyleCollection::instance().baseStyle().BoldOption;
    boldOption.setValue(font.bold());
    myApplication->doAction("updateOptions");
}

void ZLQtViewWidget::changeReadingScheme(int type)
{
    if (type == STYLE_BLACK_BACKGROUND)
    {
        ZLColorOption &bk = ZLTextStyleCollection::instance().baseStyle().BackgroundColorOption;
        bk.setValue(ZLColor(0, 0, 0));
        ZLColorOption &fg = ZLTextStyleCollection::instance().baseStyle().RegularTextColorOption;
        fg.setValue(ZLColor(255, 255, 255));
    }
    else if (type == STYLE_WHITE_BACKGROUND)
    {
        ZLColorOption &bk = ZLTextStyleCollection::instance().baseStyle().BackgroundColorOption;
        bk.setValue(ZLColor(255, 255, 255));
        ZLColorOption &fg = ZLTextStyleCollection::instance().baseStyle().RegularTextColorOption;
        fg.setValue(ZLColor(0, 0, 0));
    }

    myApplication->doAction("updateOptions");
}

void ZLQtViewWidget::changeLineSpacing(int spacing)
{
    ZLIntegerOption &option = ZLTextStyleCollection::instance().baseStyle().LineSpacePercentOption;
    option.setValue(80 + spacing * 10);
    myApplication->doAction("updateOptions");
}

void ZLQtViewWidget::changePageMargins(int margins)
{
}

void ZLQtViewWidget::rotateScreen()
{
    sys::SysStatus::instance().rotateScreen();
}

void ZLQtViewWidget::quit()
{
    myApplication->doAction("quit");
}

void ZLQtViewWidget::loadConf()
{
    conf_stored_ = false;
    Configuration & conf = myApplication->conf();
    onyx::screen::ScreenProxy::Waveform type = static_cast<onyx::screen::ScreenProxy::Waveform>(conf.options[CONFIG_FLASH_TYPE].toInt());
    if (type == onyx::screen::ScreenProxy::GU)
    {
        onyx::screen::instance().setDefaultWaveform(type);
    }
    else
    {
        onyx::screen::instance().setDefaultWaveform(onyx::screen::ScreenProxy::GC);
    }
}

void ZLQtViewWidget::saveConf()
{
    if (conf_stored_)
    {
        return;
    }

    conf_stored_ = true;
    Configuration & conf = myApplication->conf();
    QString progress("%1 / %2");
    int pos = (to_ >> shift(page_step_));
    if (pos <= 0)
    {
        pos = 1;
    }
    int total = (full_ >> shift(page_step_));
    if (total <= 0)
    {
        total = 1;
    }
    conf.info.mutable_progress() = progress.arg(pos).arg(total);
    conf.options[CONFIG_FLASH_TYPE] = onyx::screen::instance().defaultWaveform();
}

void ZLQtViewWidget::closeDocument()
{
    saveConf();
}

void ZLQtViewWidget::fastRefreshWindow(bool full_update)
{
    if (!full_update)
    {
        onyx::screen::ScreenProxy::Waveform w = onyx::screen::instance().defaultWaveform();
        onyx::screen::instance().setDefaultWaveform(onyx::screen::ScreenProxy::DW);
        myApplication->refreshWindow(false);
        onyx::screen::instance().setDefaultWaveform(w);
    }
    else
    {
        myApplication->refreshWindow(false);
    }
}

void ZLQtViewWidget::lookupAndUpdate()
{
    fastRefreshWindow(false);
    if (adjustDictWidget())
    {
        fastRefreshWindow(true);
    }
}

TTS & ZLQtViewWidget::tts()
{
    if (!tts_engine_)
    {
        tts_engine_.reset(new TTS(QLocale::system()));
        connect(tts_engine_.get(), SIGNAL(speakDone()), this , SLOT(onSpeakDone()));
    }
    return *tts_engine_;
}

TTSWidget & ZLQtViewWidget::ttsWidget()
{
    if (!tts_widget_)
    {
        tts_widget_.reset(new TTSWidget(widget(), tts()));
        tts_widget_->installEventFilter(widget()->parentWidget());
        connect(tts_widget_.get(), SIGNAL(closed()), this, SLOT(stopTTS()));
    }
    return *tts_widget_;
}

void ZLQtViewWidget::showGotoPageDialog()
{
    onyx::screen::instance().updateWidget(0, onyx::screen::ScreenProxy::GU);
    status_bar_->onMessageAreaClicked();
}

QStandardItem * ZLQtViewWidget::searchParent(const int index,
                                           std::vector<int> & entries,
                                           std::vector<QStandardItem *> & ptrs,
                                           QStandardItemModel &model)
{
    int indent = entries[index];
    for(int i = index - 1; i >= 0; --i)
    {
        if (entries[i] < indent)
        {
            return ptrs[i];
        }
    }
    return model.invisibleRootItem();
}

void ZLQtViewWidget::showTableOfContents()
{
    std::vector<int> paragraphs;
    std::vector<std::string> titles; 
    myApplication->loadTreeModelData(paragraphs,titles);

    std::vector<QStandardItem *> ptrs;
    QStandardItemModel model;
    QStandardItem *parent = model.invisibleRootItem();
    for (size_t i = 0;i < paragraphs.size();++i)
    {
        QStandardItem *item = new QStandardItem(QString::fromUtf8(titles[i].c_str()));
        item->setData(i,Qt::UserRole+100);
        item->setEditable(false);
        ptrs.push_back(item);

        // Get parent.
        parent = searchParent(i, paragraphs, ptrs, model);
        parent->appendRow(item);
    }

    TreeViewDialog dialog( widget() );
    dialog.setModel( &model);

    int ret = dialog.popup( tr("Table of Contents") );

    // Returned from the TOC view
    onyx::screen::instance().enableUpdate( false );
    QApplication::processEvents();
    onyx::screen::instance().enableUpdate( true );

    if (ret != QDialog::Accepted)
    {
        onyx::screen::instance().flush(widget(), onyx::screen::ScreenProxy::GC);
        return;
    }

    QModelIndex index = dialog.selectedItem();
    if ( !index.isValid() )
    {
        return;
    }
    int pos = model.data(index, Qt::UserRole + 100).toInt();
  
    myApplication->gotoParagraph(pos);
    repaint();
    onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GC);
}

bool ZLQtViewWidget::suggestTextFiles(const QString &file_path, bool forward)
{
    QRegExp reg_exp("\\.(txt)$", Qt::CaseInsensitive);
    QFileInfo file(file_path);
    QDir current_dir = file.dir();

    QStringList file_list = dirList(current_dir, &reg_exp);
    int index = file_list.indexOf(file.fileName());

    // don't suggest when only one file exists
    if (file_list.size() < 2)
    {
        return false;
    }

    ZLFileListDialog dialog(file_list, index, forward, 0);
    int ret = dialog.popup();
    if (QDialog::Accepted == ret)
    {
        QString selected = file_list.at(dialog.selectedFile());
        QString selected_path = current_dir.path() + "/" + selected;
        myApplication->openFile(selected_path.toUtf8().data());
        myFrame->update();
        return true;
    }
    return false;
}

void ZLQtViewWidget::triggerLargeScrollAction(const std::string &actionId)
{
    // for suggesting open next/previous text file
    QString current_file_path = myApplication->filePath();
    if (!current_file_path.isEmpty() && current_file_path.endsWith(".txt")
            && 0 != full_)
    {
        bool suggested = false;
        if ("largeScrollBackward" == actionId)
        {
            if (0 == from_)
            {
                suggested = suggestTextFiles(current_file_path, false);
            }
        }
        else if ("largeScrollForward" == actionId)
        {
            if (to_ >= full_)
            {
                suggested = suggestTextFiles(current_file_path, true);
            }
        }

        if (suggested)
        {
            // don't do the scroll action if suggested
            return;
        }
    }

    myApplication->doAction(actionId);
}

void ZLQtViewWidget::handleHyperlinks()
{
    ZLTextView *ptr = static_cast<ZLTextView *>(view().get());

    if (hyperlink_selected_)
    {
        hyperlink_selected_ = false;
        ptr->selectionModel().clear();
        view()->openInternalLink(point_.x(), point_.y());
        return;
    }

    int is_first = 0;
    if (!hyperlink_selected_)
    {
        std::string id;
        ZLTextElementArea start_area = ptr->selectionModel().wordArea();

        if (start_area.XStart > 0 && start_area.XEnd > 0)
        {
            id = view()->getInternalHyperlinkId(start_area.XStart,
                    start_area.YStart);
        }

        if (id.empty())
        {
            ptr->selectionModel().selectLastWord();
            ZLTextElementArea stop_area = ptr->selectionModel().wordArea();
            ptr->selectionModel().selectFirstWord();
            start_area = ptr->selectionModel().wordArea();
            is_first = 1;

            id = view()->getFirstInternalHyperlinkId(start_area.XStart,
                    start_area.YStart, stop_area.XStart, stop_area.YStart);
        }

        hyperlink_selected_ = !id.empty();
    }

    if (hyperlink_selected_)
    {
        if (!is_first)
        {
            ptr->selectionModel().selectPrevWord();
        }
        last_id_.clear();
        findHyperlink(true);
    } else
    {
        showGotoPageDialog();
    }
}

void ZLQtViewWidget::popupLinkInfoDialog()
{
    last_id_.clear();
    QVector<QPoint *> link_positions;
    onyx::screen::instance().enableUpdate(false);
    findAllHyperlinkPositions(link_positions);

    QVector<std::string> list_of_links;
    QVector<std::string> list_of_ids;
    int size = link_positions.size();
    for (int i = 0; i < size; i++)
    {
        QPoint *p = link_positions.at(i);
        std::string link_info;
        std::string link_id = view()->getLinkInfo(p->x(), p->y(), link_info);
        list_of_links.push_back(std::string(link_info));
        list_of_ids.push_back(std::string(link_id));
    }
    onyx::screen::instance().enableUpdate(true);
    ZLTextView *ptr = static_cast<ZLTextView *>(view().get());
    ptr->selectionModel().clear();
    repaint();

    if (size <= 0)
    {
        showGotoPageDialog();
        return;
    }

    ZLLinkInfoDialog link_info_dialog(0, list_of_links);

    int ret = link_info_dialog.popup();
    if (ret == QDialog::Accepted)
    {
        int selected_link = link_info_dialog.selectedLink();
        QPoint *p = link_positions.at(selected_link);
        view()->openInternalLink(p->x(), p->y());
    }
    else
    {
        myApplication->refreshWindow();
    }
}

void ZLQtViewWidget::reportUserBehavior(int current, int total)
{
    bool send_user_behavior = true;
    if (current == total && 1 == total)
    {
        send_user_behavior = false;
    }
    // report user behavior
    if (send_user_behavior && sys::collectUserBehavior())
    {
        onyx::data::UserBehavior behavior("FBReader",
                myApplication->filePath(), current,
                onyx::data::UserBehavior::PAGE_TURNING, QTime::currentTime());
        sys::SysStatus::instance().reportUserBehavior(behavior);
    }
}

void ZLQtViewWidget::processKeyReleaseEvent(int key)
{
    ZLTextView *ptr = static_cast<ZLTextView *>(view().get());
    if (myApplication->isTextSelectionEnabled())
    {
        switch (key)
        {
        case Qt::Key_Right:
            if (ptr->selectionModel().selectNextWord())
            {
                lookupAndUpdate();
            }
            break;
        case Qt::Key_Left:
            if (ptr->selectionModel().selectPrevWord())
            {
                lookupAndUpdate();
            }
            break;
        case Qt::Key_Up:
            if (ptr->selectionModel().selectPrevLineWord())
            {
                lookupAndUpdate();
            }
            break;
        case Qt::Key_Down:
            if (ptr->selectionModel().selectNextLineWord())
            {
                lookupAndUpdate();
            }
            break;
        case Qt::Key_Return:
            lookup();
            break;
        case Qt::Key_Escape:
            stopDictLookup();
            break;
        case Qt::Key_PageDown:
            triggerLargeScrollAction("largeScrollForward");
            break;
        case Qt::Key_PageUp:
            triggerLargeScrollAction("largeScrollBackward");
            break;
        }
    }
    else
    {
        if (!tts_widget_ || !tts_widget_->isVisible())
        {
            switch (key)
            {
            case Qt::Key_Return:
                {
//                    handleHyperlinks();
                    popupLinkInfoDialog();
                    break;
                }
            case Qt::Key_Escape:
                if (hyperlink_selected_)
                {
                    hyperlink_selected_ = false;    
                    ptr->selectionModel().clear();
                    myApplication->refreshWindow();
                }
                else
                {
                    myApplication->doAction("quit");
                }
                break;
            case Qt::Key_Left:
                if (hyperlink_selected_)
                {
                    findHyperlink(false);
                    break;
                }
            case Qt::Key_PageUp:
                {
                    hyperlink_selected_ = false;    
                    ptr->selectionModel().clear();
                    triggerLargeScrollAction("largeScrollBackward");

                    break;
                }
            case Qt::Key_Right:
                if (hyperlink_selected_)
                {
                    findHyperlink(true);
                    break;
                }
            case Qt::Key_PageDown:
                {
                    hyperlink_selected_ = false;    
                    ptr->selectionModel().clear();
                    triggerLargeScrollAction("largeScrollForward");

                    break;
                }
            case Qt::Key_Up:
            case Qt::Key_Down:
                {
                    if (hyperlink_selected_)
                    {
                        findHyperlink(key == Qt::Key_Down);
                    }
                    break;
                }
            }
        }
        else if (ttsWidget().isVisible())
        {
            switch (key)
            {
            case Qt::Key_Escape:
                {
                    stopTTS();
                    break;
                }
            case Qt::Key_PageUp:
                {
                    tts_engine_->stop();
                    triggerLargeScrollAction("largeScrollBackward");
                    startTTS();
                    break;
                }
            case Qt::Key_PageDown:
                {
                    tts_engine_->stop();
                    triggerLargeScrollAction("largeScrollForward");
                    startTTS();
                    break;
                }
            }
        }
    }
}


QStringList ZLQtViewWidget::dirList(QDir &qdir, QRegExp *filter,
        QDir::Filters ftype)
{
    QStringList list;

    qdir.setFilter(ftype);
    qdir.setSorting(QDir::Name);

    if (filter)
        list = qdir.entryList().filter((*filter));
    else
        list = qdir.entryList();

    return list;
}

void ZLQtViewWidget::updateProgress(size_t full, size_t from, size_t to)
{
    int current = (from >> shift(page_step_));
    if (current <= 0)
    {
        ++current;
    }
    int total = (full >> shift(page_step_));
    if (total <= 0)
    {
        ++total;
    }

    // The last page.
    if (to >= full)
    {
        current = total;
    }

    status_bar_->setProgress(current, total);

    reportUserBehavior(current, total);
}

bool ZLQtViewWidget::isWidgetVisible(QWidget * wnd)
{
    if (wnd)
    {
        return wnd->isVisible();
    }
    return false;
}

void ZLQtViewWidget::hideHelperWidget(QWidget * wnd)
{
    if (wnd)
    {
        wnd->hide();
    }
}

void ZLQtViewWidget::startDictLookup()
{
    enableTextSelection(true);

    if (!dicts_)
    {
        dicts_.reset(new DictionaryManager);
    }

    if (!dict_widget_)
    {
        dict_widget_.reset(new DictWidget(myQWidget, *dicts_, &tts()));
        connect(dict_widget_.get(), SIGNAL(keyReleaseSignal(int)), this, SLOT(processKeyReleaseEvent(int)));
        connect(dict_widget_.get(), SIGNAL(closeClicked()), this, SLOT(onDictClosed()));
    }

    hideHelperWidget(tts_widget_.get());
    hideHelperWidget(search_widget_.get());

    // When dictionary widget is not visible, it's necessary to update the text view.
    dict_widget_->lookup(selected_text_);
    dict_widget_->ensureVisible(selected_rect_, true);
}

void ZLQtViewWidget::stopDictLookup()
{
    onDictClosed();
    hideHelperWidget(dict_widget_.get());
    dict_widget_.reset(0);
}

void ZLQtViewWidget::showSearchWidget()
{
    if (!search_widget_)
    {
        search_widget_.reset(new OnyxSearchDialog(widget(), search_context_));
        connect(search_widget_.get(), SIGNAL(search(OnyxSearchContext &)),
            this, SLOT(onSearch(OnyxSearchContext &)));
        connect(search_widget_.get(), SIGNAL(closeClicked()), this, SLOT(onSearchClosed()));
        onyx::screen::watcher().addWatcher(search_widget_.get());
    }

    search_context_.userData() = BEFORE_SEARCH;
    hideHelperWidget(dict_widget_.get());
    hideHelperWidget(tts_widget_.get());
    search_widget_->showNormal();
}

bool ZLQtViewWidget::updateSearchCriteria()
{
    return true;
}

void ZLQtViewWidget::onSearch(OnyxSearchContext& context)
{
    if (search_context_.userData() <= BEFORE_SEARCH)
    {
        myApplication->SearchPatternOption.setValue(context.pattern().toUtf8().constData());
        myApplication->SearchForwardOption.setValue(context.forward());
        myApplication->doAction("search");
        search_context_.userData() = IN_SEARCHING;

        // TODO
        updateSearchWidget();
    }
    else
    {
        if (context.forward())
        {
            if (updateSearchWidget())
            {
                myApplication->doAction("findNext");
            }
        }
        else
        {
            if (updateSearchWidget())
            {
                myApplication->doAction("findPrevious");
            }
        }
    }
}

/// Return true if we can continue searching.
bool ZLQtViewWidget::updateSearchWidget()
{
    if (search_context_.forward())
    {
        if (!myApplication->action("findNext")->isEnabled())
        {
            search_widget_->noMoreMatches();
            return false;
        }
    }
    else
    {
        if (!myApplication->action("findPrevious")->isEnabled())
        {
            search_widget_->noMoreMatches();
            return false;
        }
    }
    return true;
}

void ZLQtViewWidget::lookup()
{
    if (!dict_widget_)
    {
        startDictLookup();
    }

    ZLTextView *ptr = static_cast<ZLTextView *>(view().get());
    selected_text_ = QString::fromUtf8(ptr->selectionModel().text().c_str());
    adjustDictWidget();
    dict_widget_->lookup(selected_text_);
}

bool ZLQtViewWidget::adjustDictWidget()
{
    ZLTextView *ptr = static_cast<ZLTextView *>(view().get());
    const ZLTextElementArea & area = ptr->selectionModel().wordArea();
    if (area.XEnd == area.XStart || area.YStart == area.YEnd)
    {
        return false;
    }
    selected_rect_.setCoords(area.XStart, area.YStart, area.XEnd, area.YEnd);
    return dict_widget_->ensureVisible(selected_rect_);
}

void ZLQtViewWidget::onDictClosed()
{
    enableTextSelection(false);
    dict_widget_.reset(0);
    myApplication->doAction("clearSelection");
}

void ZLQtViewWidget::onSearchClosed()
{
    myApplication->doAction("clearSearchResult");
}

void ZLQtViewWidget::nextPage()
{
    if (!isLastPage())
    {
        triggerLargeScrollAction("largeScrollForward");
    }
}

void ZLQtViewWidget::prevPage()
{
    triggerLargeScrollAction("largeScrollBackward");
}

bool ZLQtViewWidget::isLastPage()
{
    return (to_ >= full_);
}

void ZLQtViewWidget::onSdCardChanged(bool inserted)
{
    if (!inserted && myApplication->document_path.startsWith(SDMMC_ROOT))
    {
        quit();
    }
}

void ZLQtViewWidget::onWakeup()
{
    font_family_actions_.reloadExternalFonts();
}

/// Handle mount tree event including internal flash and sd card.
void ZLQtViewWidget::handleMountTreeEvent(bool inserted, const QString &mount_point)
{
    if (!inserted && myApplication->document_path.startsWith(mount_point))
    {
        quit();
    }
}

void ZLQtViewWidget::onAboutToShutdown()
{
    quit();
}

void ZLQtViewWidget::onMusicPlayerStateChanged(int state)
{
    if (state == sys::HIDE_PLAYER || state == sys::STOP_PLAYER)
    {
        onyx::screen::instance().flush();
        myApplication->refreshWindow();
    }
}


void ZLQtViewWidget::collectTTSContent()
{
    text_to_speak_.clear();
    tts_paragraph_index_ = 0;
    ZLTextView *ptr = static_cast<ZLTextView *>(view().get());
    std::vector<std::string> ps;
    ptr->selectionModel().text(ps);
    for(std::vector<std::string>::iterator i = ps.begin(); i != ps.end(); ++i)
    {
        text_to_speak_.push_back(QString::fromUtf8(i->c_str()));
    }
}

bool ZLQtViewWidget::hasPendingTTSContent()
{
    return (tts_paragraph_index_ < text_to_speak_.size());
}

void ZLQtViewWidget::startTTS()
{
    collectTTSContent();
    if (hasPendingTTSContent())
    {
        ttsWidget().ensureVisible();
        ttsWidget().speak(text_to_speak_.at(tts_paragraph_index_++));
    }
    else
    {
        onSpeakDone();
    }
}

void ZLQtViewWidget::onSpeakDone()
{
    if (!hasPendingTTSContent())
    {
        if (!isLastPage())
        {
            nextPage();
            startTTS();
        }
        else
        {
            ttsWidget().stop();
        }
    }
    else
    {
        tts().speak(text_to_speak_.at(tts_paragraph_index_++));
    }
}

void ZLQtViewWidget::stopTTS()
{
    ttsWidget().stop();
    hideHelperWidget(tts_widget_.get());
}

void ZLQtViewWidget::storeThumbnail(const QPixmap & pixmap)
{
    QFileInfo info(myApplication->document_path);
    cms::ContentThumbnail thumbdb(info.absolutePath());
    thumbdb.storeThumbnail(info.fileName(), cms::THUMBNAIL_LARGE, pixmap.scaled(thumbnailSize(), Qt::IgnoreAspectRatio).toImage());
}

void ZLQtViewWidget::onVolumeChanged(int new_volume, bool is_mute)
{
    if (tts_engine_.get())
    {
        tts().sound().setVolume(new_volume);
    }
}

void ZLQtViewWidget::findHyperlink(bool next)
{
    ZLTextView *ptr = static_cast<ZLTextView *>(view().get());

    bool b = (next ? ptr->selectionModel().selectNextWord() : ptr->selectionModel().selectPrevWord());
    if (!b)
    {
        b = next ?  ptr->selectionModel().selectFirstWord() : ptr->selectionModel().selectLastWord();
    }
    else
    {
        next ?  ptr->selectionModel().selectPrevWord() : ptr->selectionModel().selectNextWord();
    }

    for(; b; b = (next ? ptr->selectionModel().selectNextWord() : ptr->selectionModel().selectPrevWord()))
    {
        // find start area
        ZLTextElementArea  start_area = ptr->selectionModel().wordArea();

        if (view()->onStylusMove(start_area.XStart, start_area.YStart))
        {
            std::string s = view()->getInternalHyperlinkId(start_area.XStart, start_area.YStart);
            if (s.empty() || s == last_id_)
            {
                continue;
            }
            last_id_ = s;

            // find end area
            const  ZLTextElementArea * stop_area = & start_area;
            while((next ? ptr->selectionModel().selectNextWord() : ptr->selectionModel().selectPrevWord()))
            {

                const  ZLTextElementArea  & tmp =  ptr->selectionModel().wordArea();
                stop_area = & tmp;

                if (!view()->onStylusMove(tmp.XStart, tmp.YStart))
                {
                    next ? ptr->selectionModel().selectPrevWord() : ptr->selectionModel().selectNextWord();
                    break;
                }
                std::string s = view()->getInternalHyperlinkId(tmp.XStart, tmp.YStart);
                if (s != last_id_)
                {
                    next ? ptr->selectionModel().selectPrevWord() : ptr->selectionModel().selectNextWord();
                    break;
                }

            }

            point_.rx() = start_area.XStart;
            point_.ry() = start_area.YStart;

            onyx::screen::ScreenProxy::Waveform w = onyx::screen::instance().defaultWaveform();
            onyx::screen::instance().setDefaultWaveform(onyx::screen::ScreenProxy::DW);

            if (start_area.XStart == stop_area->XStart && start_area.YStart == stop_area->YStart)
            {
                view()->onStylusRelease(start_area.XStart ,start_area.YStart);
            }
            else 
            {
                if (start_area.XStart <= stop_area->XStart || start_area.YStart <= stop_area->YStart)
                {
                    ptr->selectionModel().activate(start_area.XStart ,start_area.YStart);
                    ptr->selectionModel().extendTo(stop_area->XStart, stop_area->YStart);
                }
                else
                {
                    ptr->selectionModel().activate(stop_area->XStart, stop_area->XStart);
                    ptr->selectionModel().extendTo(start_area.XStart,start_area.YStart);
                }
                myApplication->refreshWindow();
            }
            onyx::screen::instance().setDefaultWaveform(w);

            break;
        }

    }

}

void ZLQtViewWidget::findAllHyperlinkPositions(QVector<QPoint *> &link_positions)
{
    ZLTextView *ptr = static_cast<ZLTextView *>(view().get());
    bool next = true;
    bool b = (next ? ptr->selectionModel().selectNextWord() : ptr->selectionModel().selectPrevWord());
    if (!b)
    {
        b = next ?  ptr->selectionModel().selectFirstWord() : ptr->selectionModel().selectLastWord();
    }
    else
    {
        next ?  ptr->selectionModel().selectPrevWord() : ptr->selectionModel().selectNextWord();
    }

    for(; b; b = (next ? ptr->selectionModel().selectNextWord() : ptr->selectionModel().selectPrevWord()))
    {
        // find start area
        ZLTextElementArea  start_area = ptr->selectionModel().wordArea();

        if (view()->onStylusMove(start_area.XStart, start_area.YStart))
        {
            std::string s = view()->getInternalHyperlinkId(start_area.XStart, start_area.YStart);
            if (s.empty() || s == last_id_)
            {
                continue;
            }
            last_id_ = s;

            // find end area
            const  ZLTextElementArea * stop_area = & start_area;
            while((next ? ptr->selectionModel().selectNextWord() : ptr->selectionModel().selectPrevWord()))
            {

                const  ZLTextElementArea  & tmp =  ptr->selectionModel().wordArea();
                stop_area = & tmp;

                if (!view()->onStylusMove(tmp.XStart, tmp.YStart))
                {
                    next ? ptr->selectionModel().selectPrevWord() : ptr->selectionModel().selectNextWord();
                    break;
                }
                std::string s = view()->getInternalHyperlinkId(tmp.XStart, tmp.YStart);
                if (s != last_id_)
                {
                    next ? ptr->selectionModel().selectPrevWord() : ptr->selectionModel().selectNextWord();
                    break;
                }

            }

            QPoint *link_point = new QPoint;
            link_point->rx() = start_area.XStart;
            link_point->ry() = start_area.YStart;
            link_positions.push_back(link_point);
        }
    }

}

bool ZLQtViewWidget::isHyperlinkSelected()
{
    return hyperlink_selected_;
}

void ZLQtViewWidget::onMouseLongPress(QPoint, QSize)
{
    if (myQWidget->isActiveWindow())
    {
        popupMenu();
    }
}

void ZLQtViewWidget::onMultiTouchPressDetected(QRect r1, QRect r2)
{
    rect_pressed_ = r1.united(r2);
}

void ZLQtViewWidget::onMultiTouchReleaseDetected(QRect r1, QRect r2)
{
    QRect r = r1.united(r2);

    float diagonal_length_changed = sqrt(static_cast<float>(r.width() * r.width() + r.height() * r.height())) - sqrt(static_cast<float>(rect_pressed_.width() * rect_pressed_.width() + rect_pressed_.height() * rect_pressed_.height()));
    float diagonal_length_per_fontsize = 100 / 2; // 100 pixel for 2 fontsize 

    ZLIntegerRangeOption &sizeOption = ZLTextStyleCollection::instance().baseStyle().FontSizeOption;
    int fontSizeChanged = (int) (diagonal_length_changed / diagonal_length_per_fontsize);

    long size = sizeOption.value() + fontSizeChanged;
    sizeOption.setValue(size);

    myApplication->doAction("updateOptions");
}
