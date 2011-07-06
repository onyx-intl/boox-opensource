#include <QString>
#include "onyx/ui/menu.h"
#include "onyx/ui/ui_global.h"
#include "onyx/ui/ui_utils.h"
#include "onyx/screen/screen_update_watcher.h"
#include "onyx/screen/screen_proxy.h"
#include "rssFeedInfo.h"
#include "settings_view.h"
#include "rss_view.h"
#include "rss_feed_dialog.h"
#include "onyx/data/data_tags.h"

using namespace ui;

namespace rss_reader
{

extern QString TAG_POINTER ;
extern QString TAG_ROW;

enum {ADD, RESTORE, OK};

extern const int ROWS;
extern int defaultItemHeight();

const static QSize MENU_ITEM_SIZE = QSize(60, 60);

extern RssFactory factory;

SettingsView::SettingsView(QWidget *parent, QVector<CRSSFeedInfo * > & rss)
        : OnyxDialog(parent)
        , label_title_(tr("Choose 1 to 6 RSS feed from list or add your own RSS source"))
        , list_view_(&factory,this)
        , button_view_(&factory, this)
        , rss_feeds_(rss)
        , num_selected_(0)
{
    createLayout();
    createListView();
    createButtonView();

    this->setModal(true);

    connect(&button_view_,SIGNAL(itemActivated(CatalogView *, ContentView *, int)),
            this, SLOT(onItemActivated(CatalogView *, ContentView *, int)));
    connect(&list_view_,SIGNAL(itemActivated(CatalogView *, ContentView *, int)),
            this, SLOT(onItemActivated(CatalogView *, ContentView *, int)));
}

SettingsView::~SettingsView()
{
    list_data_.clear();
    list_view_.setData(list_data_);
}

void SettingsView::createLayout()
{
    updateTitleIcon(QPixmap());
    setFixedWidth(500);

    updateTitle(tr("Settings"));
    setContentsMargins(0, 0, 0, 0);

    v_layout_.setContentsMargins(10, 0, 10, 0);

    QFont f;
    f.setPixelSize(18);
    f.setBold(true);
    label_title_.setFont(f);

    label_title_.setFixedSize(400, 50);

    v_layout_.addWidget(&label_title_, 0);
    v_layout_.addWidget(&list_view_, 100);

    label_arrow_left_.setPixmap(QPixmap(":/rss_images/arrow_left.png"));
    label_arrow_right_.setPixmap(QPixmap(":/rss_images/arrow_right.png"));
    h_layout_.addWidget(&label_arrow_left_, 0);
    h_layout_.addStretch(0);
    h_layout_.addWidget(&label_arrow_right_, 0);

    v_layout_.addSpacing(10);
    v_layout_.addLayout(&h_layout_, 0);
    v_layout_.addSpacing(30);
    v_layout_.addStretch(0);
    v_layout_.addWidget(&button_view_, 0);
    v_layout_.addSpacing(20);

    content_widget_.setVisible(false);
    vbox_.addLayout(&v_layout_);

    label_title_.setFixedHeight(80);
    label_title_.setWordWrap(true);
}

void SettingsView::createListView()
{
    const int height = defaultItemHeight() + 2 * SPACING;
    list_view_.setSubItemType(EditView::type());
    list_view_.setPreferItemSize(QSize(-1, height));

    list_data_.clear();

    num_selected_ = 0; 
    for (int i = 0; i < rss_feeds_.size(); i++)
    {
        CRSSFeedInfo * info = rss_feeds_.at(i);

        info->GetAssociatedListItem()->insert(TAG_CHECKED, info->Selected);

        list_data_.push_back(info->GetAssociatedListItem());

        if (info->Selected)
        {
            num_selected_ ++;
        }
    }

    list_view_.setSpacing(2);
    list_view_.setData(list_data_);
    list_view_.setNeighbor(&button_view_, CatalogView::DOWN);
}

void SettingsView::createButtonView()
{
    const static QSize MENU_ITEM_SIZE = QSize(60, 60);

    button_view_.setSubItemType(ButtonView::type());
    // button_view_.setBackgroundColor(QColor(151, 151, 151));
    // button_view_.setSubItemBkColor(QColor(151, 151, 151));

    button_view_.setPreferItemSize(MENU_ITEM_SIZE);

    OData * dd = new OData;
    dd->insert(TAG_TITLE, tr("Add New"));
    dd->insert(TAG_ID, ADD);
    button_data_.push_back(dd);

    dd = new OData;
    dd->insert(TAG_TITLE, tr("Restore Factory"));
    dd->insert(TAG_ID, RESTORE);
    button_data_.push_back(dd);

    dd = new OData;
    dd->insert(TAG_TITLE, tr("OK"));
    dd->insert(TAG_ID, OK);
    button_data_.push_back(dd);

    button_view_.setSpacing(2);
    button_view_.setFixedGrid(1, 3);
    // button_view_.setFixedHeight(MENU_ITEM_SIZE.height() + 6 * SPACING);
    button_view_.setData(button_data_);
    button_view_.setSearchPolicy(CatalogView::NeighborFirst
            | CatalogView::AutoHorRecycle);
    button_view_.setNeighbor(&list_view_, CatalogView::RECYCLE_UP);

    //button_view_.setBackgroundColor(QColor(229, 229, 229));
}

void SettingsView::updateView()
{
    update();
}

void SettingsView::onItemActivated(CatalogView* catalog, ContentView* item, int user_data)
{
    OData * item_data = item->data();
    if (item_data->contains(TAG_ROW))
    {
        CRSSFeedInfo* feedInfo = (CRSSFeedInfo *) item_data->value(TAG_POINTER).toInt();
        if (user_data == EditView::CHECKBOX)
        {

            if (feedInfo->Selected)
            {
                num_selected_ ++;
            }
            else
            {
                num_selected_ --;
            }

            if (num_selected_ > ROWS)
            {
                QKeyEvent keyEvent(QEvent::KeyRelease, Qt::Key_Return, Qt::NoModifier);
                QApplication::sendEvent(item, &keyEvent);
            }
        }
        else if (user_data == EditView::EDIT)
        {
            RssFeedDialog dialog(tr("Edit Feed"), this);
            dialog.setTitle(feedInfo->Title);
            dialog.setUrl(feedInfo->Url.toString());
            int ret = dialog.popup();

            if (ret == QDialog::Accepted)
            {
                // TODO, not good
                feedInfo->Title = dialog.title();
                feedInfo->Title_for_save = dialog.title();
                feedInfo->Url = QUrl(dialog.url());

                OData *dd = feedInfo->GetAssociatedListItem();
                dd->insert(TAG_TITLE, feedInfo->Title);
                dd->insert(TAG_URL, feedInfo->Url.toString());
                item->updateView();
            }
        }
        else if (user_data == EditView::REMOVE)
        {
            DeleteRssDialog dialog(feedInfo->Title, this);
            int ret = dialog.exec();
            if (ret != QMessageBox::Yes)
            {
                return ;
            }

            for (int i = 0; i < rss_feeds_.size(); ++i) 
            {
                if (rss_feeds_.at(i) == feedInfo)
                {
                    emit removeFeed(i);
                    createListView();
                    if (i >= list_data_.size())
                    {
                        list_view_.select(list_data_.at(i-1));
                    }
                    else if(i > 0)
                    {
                        list_view_.select(list_data_.at(i));
                    }
                    break;
                }
           }
        }
    }
    else
    {
        int flag = item_data->value(::TAG_ID).toInt();
        if (flag == OK)
        {
                accept();
        }
        else if (flag == ADD)
        {
            RssFeedDialog dialog(tr("Add Feed"), this);
            int ret = dialog.popup();
            onyx::screen::instance().updateWidget(0, onyx::screen::ScreenProxy::GU);

            if (ret == QDialog::Accepted && !dialog.title().isEmpty() && !dialog.url().isEmpty())
            {
                // TODO, not good
                CRSSFeedInfo feedInfo;
                feedInfo.Title = dialog.title();
                feedInfo.Title_for_save = dialog.title();
                feedInfo.Url = QUrl(dialog.url());

                emit addFeed(feedInfo);
                createListView();
                list_view_.select(rss_feeds_.last()->GetAssociatedListItem());
            }
        }
        else if (flag == RESTORE)
        {
            emit reloadFeedList();
            createListView();
        }
    }
}

bool SettingsView::event(QEvent* event)
{
    bool ret = QDialog::event(event);

    if (event->type() == QEvent::UpdateRequest)
    {
        onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GU);
    }
    return ret;
}

int SettingsView::popup()
{
    updateView();

    show();
    QApplication::processEvents();
    //centerWidgetOnScreen(this);

    button_view_.visibleSubItems().at(2)->setFocus();
    onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GU, false, onyx::screen::ScreenCommand::WAIT_ALL);
    int ret = exec();
    return ret;
}

void SettingsView::paintEvent(QPaintEvent * event)
{
    OnyxDialog::paintEvent(event);
}

void SettingsView::resizeEvent ( QResizeEvent * event ) 
{
    setFixedWidth(500);

    // list_view_.setFixedHeight(500);
    list_view_.setFixedGrid(4, 1);

    // button_view_.setSpacing(250);
    button_view_.setContentsMargins(0, 0, 0, 0);
    button_view_.setFixedHeight(40);
    // button_view_.setFixedWidth(450);
    // button_view_.visibleSubItems().at(0)->setFixedWidth(330);
    // button_view_.visibleSubItems().at(1)->setFixedWidth(70);
    ((QGridLayout *) button_view_.layout())->setColumnStretch(0, 150);
    ((QGridLayout *) button_view_.layout())->setColumnStretch(1, 180);
    ((QGridLayout *) button_view_.layout())->setColumnStretch(2, 80);
}

void SettingsView::mouseReleaseEvent(QMouseEvent * event)
{
    if (label_arrow_left_.isVisible() && label_arrow_left_.geometry().contains(event->pos()))
    {
        list_view_.goPrev();
        event->accept();
    }
    else if (label_arrow_right_.isVisible() && label_arrow_right_.geometry().contains(event->pos()))
    {
        list_view_.goNext();
        event->accept();
    }
}

void SettingsView::keyReleaseEvent(QKeyEvent *ke)
{
    ke->accept();
    switch(ke->key())
    {
        case Qt::Key_Escape:
        case ui::Device_Menu_Key:
            reject();
            break;
        case Qt::Key_PageUp:
            {
                list_view_.goPrev();
            }
            break;
        case Qt::Key_PageDown:
            {
                list_view_.goNext();
            }
            break;
        default:
            break;
    }
}

DeleteRssDialog::DeleteRssDialog(const QString & rssName,QWidget *parent)
: MessageDialog(QMessageBox::Warning,
                tr("Remove"),
                tr(""),
                QMessageBox::Yes|QMessageBox::No,
                parent)
{
    QString str(tr("Do you want to remove %1?"));
    str = str.arg(rssName);
    MessageDialog::updateInformation(str);
    button(QMessageBox::No)->setFocus();
}

DeleteRssDialog::~DeleteRssDialog(void)
{
}

}
