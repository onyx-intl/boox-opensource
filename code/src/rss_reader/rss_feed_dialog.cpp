#include "rss_feed_dialog.h"
#include "onyx/data/data_tags.h"
#include "onyx/ui/content_view.h"
#include "onyx/screen/screen_update_watcher.h"
#include "onyx/ui/ui_utils.h"

static const int LABEL_WIDTH = 150;
static int KEYBOARD_MENU_CANCLE = 10;

// A password line edit will be provided on default. Need to specify an OData
// for each line edit besides password.
// Sample:
//
//  User ID:
//  Password:
//
// needs an OData with TAG_TITLE/value ("User ID: ") inserted.
// Insert TAG_IS_PASSWD/value (true) to specify password field.
// Insert TAG_DEFAULT_VALUE/value ("123456") to specify default text for line edit.

RssFeedDialog::RssFeedDialog(const QString & str, QWidget *parent)
    : OnyxDialog(parent)
    , big_layout_(&content_widget_)
    , line_edit_layout_(0)
    , sub_menu_(0, this)
    , keyboard_(this)
    , title_(str)
{
    setModal(true);
    updateTitle(str);
    onyx::screen::watcher().addWatcher(this);
}

void RssFeedDialog::init()
{
    appendDefaultPasswordEdit();
    createLayout();
    connectWithChildren();
}

RssFeedDialog::~RssFeedDialog()
{
    foreach (ODatas *edit_datas, all_line_edit_datas_)
    {
        clearDatas(*edit_datas);
        delete edit_datas;
    }
    clearDatas(sub_menu_datas_);
}

void RssFeedDialog::appendDefaultPasswordEdit()
{
    title_data = new OData;
    title_data->insert(TAG_TITLE, tr("Title"));
    title_data->insert(TAG_DEFAULT_VALUE, feed_title_);
    edit_list_.append(title_data);

    url_data = new OData;
    url_data->insert(TAG_TITLE, tr("Url"));
    url_data->insert(TAG_DEFAULT_VALUE, feed_url_);
    edit_list_.append(url_data);
}

void RssFeedDialog::addLineEditsToGroup()
{
    foreach (CatalogView *edit_item, edit_view_list_)
    {
        LineEditView * line_edit = static_cast<LineEditView *>(
                edit_item->visibleSubItems().front());
        edit_view_group_.addEdit(line_edit);
    }
}

bool RssFeedDialog::popup()
{
    init();
    if (isHidden())
    {
        show();
    }
    resize(600, height());

    return exec();
}

QString RssFeedDialog::value(OData * d_index)
{
    CatalogView *target = edit_view_list_.back();
    if (0 != d_index)
    {
        int index = edit_list_.indexOf(ODataPtr(d_index));
        target = edit_view_list_.at(index);
    }

    LineEditView * item = static_cast<LineEditView *>(
            target->visibleSubItems().front());
    return item->innerEdit()->text();
}

void RssFeedDialog::createLayout()
{
    vbox_.setSpacing(0);
    content_widget_.setBackgroundRole(QPalette::Button);
    content_widget_.setContentsMargins(0, 0, 0, 0);
    big_layout_.setContentsMargins(2, 2, 2, 2);
    big_layout_.setSpacing(0);

    int sub_menu_width = defaultItemHeight()*5;
    int line_edit_width = 520;

    createLineEdits(line_edit_width);
    createSubMenu(sub_menu_width);

    int size = edit_list_.size();
    for (int i=0; i<size; i++)
    {
        ODataPtr data = edit_list_.at(i);
        QString label_text = data->value(TAG_TITLE).toString();
        OnyxLabel *label = new OnyxLabel(label_text);
        QFont font;
        font.setPointSize(16);
        font.setBold(true);
        label->setFont(font);
        label->setFixedWidth(70);
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        line_edit_layout_ = new QHBoxLayout;
        line_edit_layout_->addWidget(label);
        line_edit_layout_->addWidget(edit_view_list_.at(i));

        big_layout_.addLayout(line_edit_layout_);
    }
    QHBoxLayout * menu_layout = new QHBoxLayout;
    menu_layout->addWidget(&sub_menu_, 0, Qt::AlignRight);
    big_layout_.addLayout(menu_layout);
    big_layout_.addWidget(&keyboard_);
}

CatalogView * RssFeedDialog::createEditItem(OData *data, int index,
        ODatas *edit_datas, const int &line_edit_width)
{
    const int height = defaultItemHeight();
    CatalogView * edit_item = new CatalogView(0, this);
    edit_item->setSubItemType(LineEditView::type());
    edit_item->setPreferItemSize(QSize(height, height));

    ODataPtr dd(new OData);

    // copy the TAG_IS_PASSWD property
    if (data->contains(TAG_IS_PASSWD))
    {
        dd->insert(TAG_IS_PASSWD, data->value(TAG_IS_PASSWD).toBool());
    }

    // copy the TAG_DEFAULT_VALUE property to TAG_TITLE
    if (data->contains(TAG_DEFAULT_VALUE))
    {
        dd->insert(TAG_TITLE, data->value(TAG_DEFAULT_VALUE).toString());
    }

    // copy the TAG_DISABLED property
    if (data->contains(TAG_DISABLED))
    {
        dd->insert(TAG_DISABLED, data->value(TAG_DISABLED).toBool());
    }

    edit_datas->push_back(dd);

    edit_item->setFixedGrid(1, 1);
    edit_item->setMargin(OnyxKeyboard::CATALOG_MARGIN);
    edit_item->setFixedHeight(defaultItemHeight()+2*SPACING);
    edit_item->setFixedWidth(line_edit_width);
    edit_item->setData(*edit_datas);
    return edit_item;
}

void RssFeedDialog::createLineEdits(const int &line_edit_width)
{
    int size = edit_list_.size();
    int default_checked = 0;
    for (int i=0; i<size; i++)
    {
        ODataPtr data = edit_list_.at(i);

        ODatas * edit_datas = new ODatas;
        CatalogView * edit_item = createEditItem(data, i, edit_datas,
                line_edit_width);
        all_line_edit_datas_.push_back(edit_datas);

        // set default checked edit item
        if (default_checked == i)
        {
            ODataPtr td = edit_item->data().front();
            if (td)
            {
                if (!td->contains(TAG_DISABLED) || !td->value(TAG_DISABLED).toBool())
                {
                    // set the TAG_CHECKED property
                    td->insert(TAG_CHECKED, true);
                    default_checked = -1;
                }
                else
                {
                    default_checked++;
                }
            }
        }

        if (!edit_view_list_.isEmpty())
        {
            edit_item->setNeighbor(edit_view_list_.back(), CatalogView::UP);
        }
        edit_item->setNeighbor(&sub_menu_, CatalogView::DOWN);
        edit_item->setNeighbor(&sub_menu_, CatalogView::RIGHT);
        edit_view_list_.push_back(edit_item);
    }
    if (!edit_view_list_.isEmpty())
    {
        edit_view_list_.front()->setNeighbor(keyboard_.menu(),
                CatalogView::RECYCLE_DOWN);
    }
}

void RssFeedDialog::createSubMenu(const int &sub_menu_width)
{
    const int height = defaultItemHeight();
    sub_menu_.setPreferItemSize(QSize(height, height));

    ODataPtr dd(new OData);
    dd->insert(TAG_TITLE, tr("OK"));
    dd->insert(TAG_MENU_TYPE, OnyxKeyboard::KEYBOARD_MENU_OK);
    sub_menu_datas_.push_back(dd);

    ODataPtr b(new OData);
    b->insert(TAG_TITLE, tr("Cancle"));
    b->insert(TAG_MENU_TYPE, KEYBOARD_MENU_CANCLE);
    sub_menu_datas_.push_back(b);

    sub_menu_.setFixedGrid(1, 2);
    sub_menu_.setMargin(OnyxKeyboard::CATALOG_MARGIN);
    sub_menu_.setFixedHeight(defaultItemHeight()+2*SPACING);
    sub_menu_.setFixedWidth(sub_menu_width);
    sub_menu_.setData(sub_menu_datas_);
    sub_menu_.setNeighbor(keyboard_.top(), CatalogView::RIGHT);
    sub_menu_.setNeighbor(keyboard_.top(), CatalogView::DOWN);
}

void RssFeedDialog::connectWithChildren()
{
    connect(&sub_menu_, SIGNAL(itemActivated(CatalogView *, ContentView *, int)),
            this, SLOT(onItemActivated(CatalogView *, ContentView *, int)));
}

void RssFeedDialog::keyPressEvent(QKeyEvent *event)
{
    event->accept();
    if (0 == edit_view_group_.editList().size())
    {
        addLineEditsToGroup();
    }

    int key = event->key();
    if (Qt::Key_Up != key
            && Qt::Key_Down != key
            && Qt::Key_Left != key
            && Qt::Key_Right != key
            && Qt::Key_Return != key
            && Qt::Key_Enter != key)
    {
        QApplication::sendEvent(edit_view_group_.checkedEdit(), event);
    }
}

void RssFeedDialog::onItemActivated(CatalogView *catalog,
                                   ContentView *item,
                                   int user_data)
{
    OData * item_data = item->data();
    if (item_data->contains(TAG_MENU_TYPE))
    {
        int menu_type = item->data()->value(TAG_MENU_TYPE).toInt();
        if(OnyxKeyboard::KEYBOARD_MENU_OK == menu_type)
        {
            accept();
        }
        else if(KEYBOARD_MENU_CANCLE == menu_type)
        {
            this->reject();
        }
    }

}

QString RssFeedDialog::title()
{
    return value(title_data);
}

QString RssFeedDialog::url()
{
    return value();
}

void RssFeedDialog::setTitle(const QString & title)
{
    feed_title_ = title;
}

void RssFeedDialog::setUrl(const QString & url)
{
    feed_url_ = url;
}
