#ifdef BUILD_FOR_ARM
#include <QtGui/qwsdisplay_qws.h>
#include <QtGui/qscreen_qws.h>
#endif

#include "djvu_thumbnail_view.h"
#include "djvu_model.h"

using namespace ui;

namespace djvu_reader
{

RotateDegree getSystemRotateDegree()
{
    int degree = 0;
#ifdef BUILD_FOR_ARM
    degree = QScreen::instance()->transformOrientation();
#endif
    return static_cast<RotateDegree>(degree);
}

QSize shrinkImageSize(const QSize & size)
{
    return QSize(size.width() - 2, size.height() - 2);
}

ThumbnailView::ThumbnailView(QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint)
    , model_(0)
    , layout_()
    , sketch_proxy_(0)
    , display_pages_()
    , waiting_pages_()
    , cur_thumb_index_(0)
{
    initializePopupMenuActions();
}

ThumbnailView::~ThumbnailView(void)
{
}

void ThumbnailView::attachMainWindow(MainWindow *main_window)
{
    connect(this, SIGNAL(positionChanged(const int, const int)),
            main_window, SLOT(handlePositionChanged(const int, const int)));
    connect(this, SIGNAL(needFullScreen(bool)),
            main_window, SLOT(handleFullScreen(bool)));
    connect(this, SIGNAL(popupJumpPageDialog()),
            main_window, SLOT(handlePopupJumpPageDialog()));

    connect(main_window, SIGNAL(pagebarClicked(const int, const int)),
            this, SLOT(onPagebarClicked(const int, const int)));
    connect(main_window, SIGNAL(popupContextMenu()),
            this, SLOT(onPopupContextMenu()));

    onyx::screen::instance().setDefaultWaveform(onyx::screen::ScreenProxy::GU);
}

void ThumbnailView::deattachMainWindow(MainWindow *main_window)
{
    disconnect(this, SIGNAL(positionChanged(const int, const int)),
               main_window, SLOT(handlePositionChanged(const int, const int)));
    disconnect(this, SIGNAL(needFullScreen(bool)),
               main_window, SLOT(handleFullScreen(bool)));
    disconnect(this, SIGNAL(popupJumpPageDialog()),
               main_window, SLOT(handlePopupJumpPageDialog()));

    disconnect(main_window, SIGNAL(pagebarClicked(const int, const int)),
               this, SLOT(onPagebarClicked(const int, const int)));
    disconnect(main_window, SIGNAL(popupContextMenu()),
               this, SLOT(onPopupContextMenu()));
}

void ThumbnailView::attachSketchProxy(SketchProxy *sketch_proxy)
{
    sketch_proxy_ = sketch_proxy;
    sketch_proxy_->setDrawingArea(this);
}

void ThumbnailView::deattachSketchProxy()
{
    sketch_proxy_ = 0;
}

void ThumbnailView::setCurrentPage(int page_num)
{
    display_pages_.clear();
    waiting_pages_.clear();

    ThumbnailPages & pages = layout_.pages();

    // send render request
    QSize size = shrinkImageSize(pages[0].image_area.size());
    emit needThumbnailForNewPage(page_num, size);
}

void ThumbnailView::setThumbnail(ThumbPtr thumb)
{
    // Make sure the waiting pages are empty?
    //waiting_pages_.clear();

    // load sketches if it has
    if (sketch_proxy_ != 0)
    {
        QString page_key;
        page_key.setNum(thumb->key());
        sketch_proxy_->loadPage(model_->path(), page_key, QString());
    }

    waiting_pages_.push_back(thumb);

    // if it is the last screen, set ready to show
    if (reachedLastScreen(thumb))
    {
        readyToShow();
    }
    else
    {
        // request for next screen
        ThumbnailPages & pages = layout_.pages();
        QSize size = shrinkImageSize(pages[waiting_pages_.size()].image_area.size());
        emit needNextThumbnail(thumb->key(), size);
    }
}

void ThumbnailView::readyToShow()
{
    if (waiting_pages_.size() == 0)
    {
        return;
    }

    // copy the waiting pages to display pages
    if (waiting_pages_.size() < static_cast<size_t>(layout_.pages().size()) && reachedFirstScreen(waiting_pages_.front()))
    {
        int left_pages = qMin(layout_.pages().size() - waiting_pages_.size(), display_pages_.size());
        for (int i = 0; i < left_pages; ++i)
        {
            waiting_pages_.push_back(display_pages_.get_page(i));
        }
    }
    display_pages_ = waiting_pages_;

    if (cur_thumb_index_ >= static_cast<int>(display_pages_.size()))
    {
        cur_thumb_index_ = display_pages_.size() - 1;
    }

    // update page bar
    emit positionChanged(display_pages_.front()->key(), model_->getPagesTotalNumber());
    update(onyx::screen::ScreenProxy::GU);
}

bool ThumbnailView::reachedFirstScreen(ThumbPtr page)
{
    return page->key() <= 0;
}

bool ThumbnailView::reachedLastScreen(ThumbPtr page)
{
    return page->key() >= (model_->getPagesTotalNumber() - 1);
}

void ThumbnailView::nextScreen()
{
    ThumbPtr last_page = display_pages_.last();
    if (reachedLastScreen(last_page))
    {
        return;
    }

    waiting_pages_.clear();
    QSize size = shrinkImageSize(layout_.pages().front().image_area.size());
    emit needNextThumbnail(last_page->key(), size);
}

void ThumbnailView::prevScreen()
{
    ThumbPtr first_page = display_pages_.front();
    if (reachedFirstScreen(first_page))
    {
        return;
    }

    waiting_pages_.clear();
    QSize size = shrinkImageSize(layout_.pages().last().image_area.size());
    emit needPreviousThumbnail(first_page->key(), size);
}

void ThumbnailView::setNextThumbnail(ThumbPtr thumb)
{
    waiting_pages_.push_back(thumb);
    ThumbnailPages & pages = layout_.pages();
    if (reachedLastScreen(thumb) || waiting_pages_.size() == pages.size())
    {
        readyToShow();
    }
    else
    {
        QSize size = shrinkImageSize(pages[waiting_pages_.size()].image_area.size());
        emit needNextThumbnail(thumb->key(), size);
    }
}

void ThumbnailView::setPreviousThumbnail(ThumbPtr thumb)
{
    waiting_pages_.push_front(thumb);
    ThumbnailPages & pages = layout_.pages();
    if (reachedFirstScreen(thumb) || waiting_pages_.size() == pages.size())
    {
        readyToShow();
    }
    else
    {
        QSize size = shrinkImageSize(pages[pages.size() - waiting_pages_.size() - 1].image_area.size());
        emit needPreviousThumbnail(thumb->key(), size);
    }
}

QRect ThumbnailView::getPageDisplayArea(ThumbPtr page, int index)
{
    ThumbnailPages & layout_pages = layout_.pages();
    ThumbnailPage & page_layout = layout_pages[index];
    QPoint disp_pos(page_layout.image_area.topLeft());
    if (page_layout.image_area.width() > page->image()->width())
    {
        disp_pos.setX(disp_pos.x() + ((page_layout.image_area.width() - page->image()->width()) >> 1));
    }
    if (page_layout.image_area.height() > page->image()->height())
    {
        disp_pos.setY(disp_pos.y() + ((page_layout.image_area.height() - page->image()->height()) >> 1));
    }
    return QRect(disp_pos, page->image()->size());
}

void ThumbnailView::paintPage(QPainter &painter, ThumbPtr page, int index)
{
    QRect display_area = getPageDisplayArea(page, index);
    painter.drawImage(display_area.topLeft(), *(page->image()));
}

void ThumbnailView::paintSketches(QPainter &painter, ThumbPtr page, int index)
{
    if (sketch_proxy_ == 0)
    {
        qDebug("Cannot find sketch proxy");
        return;
    }

    // draw sketches in this page
    QRect display_area = getPageDisplayArea(page, index);
    sketch::PageKey page_key;
    page_key.setNum( page->key() );
    if ( !sketch_proxy_->isPageExisting( model_->path(), page_key ) )
    {
        return;
    }

    if ( !sketch_proxy_->loadPage( model_->path(), page_key, QString() ) )
    {
        return;
    }

    // make sure sketch on correct display context
    sketch_proxy_->setWidgetOrient( getSystemRotateDegree() );

    // TODO. Do NOT multiply the ZOOM_ACTUAL factor, keep consistent with other readers
    sketch_proxy_->setZoom( page->zoom() * ZOOM_ACTUAL );
    sketch_proxy_->setContentOrient( ROTATE_0_DEGREE );
    sketch_proxy_->updatePageDisplayRegion(model_->path(), page_key, display_area);

    // draw sketches in this page
    sketch_proxy_->paintPage(model_->path(), page_key, painter);
}

void ThumbnailView::paintTitle(QPainter &painter, ThumbPtr page, int index)
{
    // get the layout page by thumbnail
    ThumbnailPage& layout_page = getLayoutPage(index);

    // display comments in this area
    QFont f;
    f.setPixelSize(layout_page.text_area.height() >> 1);
    f.setStyleStrategy(QFont::ForceOutline);

    // Should use layout instead of using setText directly.
    // Prepare the layout.
    QTextLayout layout;
    layout.setFont(f);
    layout.setText(page->name());

    QTextOption opt = layout.textOption();
    opt.setAlignment(Qt::AlignHCenter);
    opt.setWrapMode(QTextOption::WrapAnywhere);
    layout.setTextOption(opt);
    layout.beginLayout();
    QTextLine line = layout.createLine();
    while (line.isValid())
    {
        line.setLineWidth(layout_page.text_area.width());
        line = layout.createLine();
    }
    layout.endLayout();

    // Draw layout to the painter.
    int y = layout_page.text_area.top();
    for(int i = 0; i < layout.lineCount(); ++i)
    {
        QTextLine line = layout.lineAt(i);
        line.draw(&painter, QPoint(layout_page.text_area.left(), y));
        y += static_cast<int>(line.height());
    }
}

void ThumbnailView::paintBoundingRect(QPainter &painter, const ThumbnailPage& thumb)
{
    painter.drawRect(thumb.image_area);
}

void ThumbnailView::paintEvent(QPaintEvent *pe)
{
    if (cur_thumb_index_ < 0 || cur_thumb_index_ >= layout_.pages().size())
    {
        return;
    }

    QPainter painter(this);

    // draw the rectangle of current page
    painter.fillRect(rect(), QBrush(Qt::white));
    if (display_pages_.size() > 0)
    {
        paintBoundingRect(painter, getLayoutPage(cur_thumb_index_));
    }

    // draw the thumbnail images
    int count = qMin(display_pages_.size(), static_cast<size_t>(layout_.pages().size()));
    for (int i = 0; i < count; ++i)
    {
        ThumbPtr cur_thumb = display_pages_.get_page(i);
        paintPage(painter, cur_thumb, i);
        paintTitle(painter, cur_thumb, i);
        paintSketches(painter, cur_thumb, i);
    }
}

void ThumbnailView::resizeEvent(QResizeEvent *re)
{
    layout_.setWidgetSize(re->size());
    RotateDegree degree = getSystemRotateDegree();
    sketch_proxy_->setWidgetOrient( degree );
    ThumbnailLayoutContext context = layout_.context();

    QSize desktop_size = re->size();
#ifdef Q_WS_QWS
    desktop_size = qApp->desktop()->screenGeometry().size();
#endif
    if (desktop_size.width() < desktop_size.height())
    {
        context.columns   = 2;
        context.rows      = 2;
        context.space     = 10;
        context.txt_ratio = 0.1f;
    }
    else
    {
        context.columns   = 2;
        context.rows      = 1;
        context.space     = 2;
        context.txt_ratio = 0.05f;
    }
    layout_.updateContext(context);
    if (display_pages_.size() > 0)
    {
        setCurrentPage(display_pages_.get_page(0)->key());
    }
}

void ThumbnailView::mousePressEvent(QMouseEvent *me)
{
    switch (me->button())
    {
    case Qt::LeftButton:
        {
            mouse_press_pos_ = me->pos();
        }
        break;
    default:
        break;
    }
    me->accept();
}

void ThumbnailView::mouseReleaseEvent(QMouseEvent *me)
{
    switch (me->button())
    {
    case Qt::LeftButton:
        {
            if (!mouse_press_pos_.isNull())
            {
                int sys_offset = sys::SystemConfig::direction(mouse_press_pos_, me->pos());
                if (sys_offset != 0)
                {
                    sys_offset > 0 ? nextScreen() : prevScreen();
                    me->accept();
                    return;
                }
            }

            int thumb_idx = 0;
            if (layout_.hitTest(me->pos(), thumb_idx) &&
                thumb_idx < static_cast<int>(display_pages_.size()))
            {
                sys::SysStatus::instance().setSystemBusy(true, false);
                emit returnToReading(display_pages_.get_page(thumb_idx)->key());
            }
        }
        break;
    case Qt::RightButton:
        break;
    default:
        break;
    }
    me->accept();
}

void ThumbnailView::moveCurrentPage(const int next_num)
{
    int num_layout = layout_.pages().size();
    if (next_num < 0)
    {
        prevScreen();
        cur_thumb_index_ = next_num + num_layout;
    }
    else if(next_num >= num_layout)
    {
        nextScreen();
        cur_thumb_index_ = next_num - num_layout;
    }
    else
    {
        cur_thumb_index_ = next_num;
        if (cur_thumb_index_ >= static_cast<int>(display_pages_.size()))
        {
            cur_thumb_index_ = display_pages_.size() - 1;
        }
        update(onyx::screen::ScreenProxy::DW);
    }
}

void ThumbnailView::keyReleaseEvent(QKeyEvent *ke)
{
    switch(ke->key())
    {
    case Qt::Key_PageDown:
        {
            nextScreen();
        }
        break;
    case Qt::Key_PageUp:
        {
            prevScreen();
        }
        break;
    case Qt::Key_Left:
        {
            int prev = cur_thumb_index_ - 1;
            moveCurrentPage(prev);
        }
        break;
    case Qt::Key_Right:
        {
            int next = cur_thumb_index_ + 1;
            moveCurrentPage(next);
        }
        break;
    case Qt::Key_Up:
        {
            int up = cur_thumb_index_ - layout_.context().columns;
            moveCurrentPage(up);
        }
        break;
    case Qt::Key_Down:
        {
            int down = cur_thumb_index_ + layout_.context().columns;
            moveCurrentPage(down);
        }
        break;
    case Qt::Key_F10:
    case ui::Device_Menu_Key:
        {
            onPopupContextMenu();
        }
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        {
            sys::SysStatus::instance().setSystemBusy(true, false);
            emit returnToReading(display_pages_.get_page(cur_thumb_index_)->key());
        }
        break;
    case Qt::Key_Escape:
        {
            sys::SysStatus::instance().setSystemBusy(true, false);
            emit returnToReading();
        }
        break;
    default:
        break;
    }
}

ThumbnailPage& ThumbnailView::getLayoutPage(const int key)
{
    ThumbnailPages & pages = layout_.pages();
    return pages[key];
}

bool ThumbnailView::event(QEvent * event)
{
    bool ret = QWidget::event(event);
    if (event->type() == QEvent::UpdateRequest)
    {
        onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GU);
    }
    return ret;
}

void ThumbnailView::onPagebarClicked(const int percent, const int value)
{
    if (value >= 0 && value <= model_->getPagesTotalNumber())
    {
        setCurrentPage(value);
    }
}

/// Initialize the actions of popup menu
void ThumbnailView::initializePopupMenuActions()
{
    std::vector<int> sys_actions;
    sys_actions.push_back(ROTATE_SCREEN);
    sys_actions.push_back(SCREEN_UPDATE_TYPE);
    sys_actions.push_back(RETURN_TO_LIBRARY);
    system_actions_.generateActions(sys_actions);
}

/// Popup menu
void ThumbnailView::popupMenu()
{
    ui::PopupMenu menu(this);
    menu.setSystemAction(&system_actions_);

    if (menu.popup() != QDialog::Accepted)
    {
        return;
    }

    QAction * group = menu.selectedCategory();
    if (group == system_actions_.category())
    {
        SystemAction system_action = system_actions_.selected();
        switch (system_action)
        {
        case RETURN_TO_LIBRARY:
            sys::SysStatus::instance().setSystemBusy(true, false);
            emit returnToReading();
            break;
        case SCREEN_UPDATE_TYPE:
            onyx::screen::instance().toggleWaveform();
            update(onyx::screen::ScreenProxy::GU);
            break;
        case ROTATE_SCREEN:
            rotate();
            break;
        default:
            break;
        }
    }
}

void ThumbnailView::onPopupContextMenu()
{
    popupMenu();
}

void ThumbnailView::rotate()
{
#ifndef Q_WS_QWS
    RotateDegree prev_degree = getSystemRotateDegree();
    if (prev_degree == ROTATE_0_DEGREE)
    {
        resize(800, 600);
    }
    else
    {
        resize(600, 800);
    }
#endif
    SysStatus::instance().rotateScreen();
}

void ThumbnailView::update(onyx::screen::ScreenProxy::Waveform waveform)
{
    onyx::screen::ScreenUpdateWatcher::instance().enqueue(parentWidget() != 0 ? parentWidget() : this, waveform);
    QWidget::update();
}

}
