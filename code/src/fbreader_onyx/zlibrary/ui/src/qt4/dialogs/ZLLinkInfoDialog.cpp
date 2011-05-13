#include "ZLLinkInfoDialog.h"
#include "onyx/data/data_tags.h"
#include "onyx/screen/screen_update_watcher.h"
#include "text_wrap_view.h"

static const QString TAG_ROW = "row";
static const int LINK_ITEM_HEIGHT = 60;

static TextWrapViewFactory text_wrap_view_factory;

ZLLinkInfoDialog::ZLLinkInfoDialog(QWidget *parent,
        QVector<std::string> &list_of_links)
    : QDialog(parent, Qt::FramelessWindowHint)
    , big_layout_(this)
    , links_(&text_wrap_view_factory, this)
    , list_of_links_(list_of_links)
    , link_selected_(-1)
{
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Button);

    createLayout();
    connectWithChildren();
}

ZLLinkInfoDialog::~ZLLinkInfoDialog()
{
    clearDatas(links_datas_);
}

void ZLLinkInfoDialog::createLayout()
{
    createLinks();

    big_layout_.setContentsMargins(0, 2, 0, 2);
    big_layout_.setSpacing(2);
    big_layout_.addSpacing(5);
    big_layout_.addWidget(&links_, 0, Qt::AlignBottom);
}

void ZLLinkInfoDialog::createLinks()
{
    links_.setSubItemType(TextWrapView::type());
    links_.setPreferItemSize(QSize(-1, LINK_ITEM_HEIGHT));
    links_.setSearchPolicy(CatalogView::NeighborFirst
            | CatalogView::AutoVerRecycle);

    int size = list_of_links_.size();
    for (int i=0; i<size; i++)
    {
        ODataPtr item(new OData);
        item->insert(TAG_TITLE, QString::fromUtf8(list_of_links_.at(i).data()));
        item->insert(TAG_ROW, i);

        int flag = Qt::AlignTop | Qt::AlignLeft;
        item->insert(TAG_ALIGN, flag);

        links_datas_.push_back(item);
    }

    links_.setData(links_datas_);
    int data_size = links_datas_.size();
    links_.setFixedGrid(data_size, 1);
    links_.setFixedHeight(data_size*LINK_ITEM_HEIGHT+10);
}

void ZLLinkInfoDialog::connectWithChildren()
{
    connect(&links_, SIGNAL(itemActivated(CatalogView *, ContentView *, int)),
            this, SLOT(onItemActivated(CatalogView *, ContentView *, int)));
}

int ZLLinkInfoDialog::popup()
{
    if (isHidden())
    {
        show();
    }
    QWidget * widget = safeParentWidget(parentWidget());
    setFixedWidth(widget->width());
    move(0, widget->height()-height());

    onyx::screen::watcher().addWatcher(this);
    int ret = this->exec();
    onyx::screen::watcher().removeWatcher(this);
    return ret;
}

void ZLLinkInfoDialog::keyPressEvent(QKeyEvent *ke)
{
    if (ke->key() == Qt::Key_Escape)
    {
        ke->accept();
        return;
    }
    QDialog::keyPressEvent(ke);
}

void ZLLinkInfoDialog::keyReleaseEvent(QKeyEvent *ke)
{
    if (ke->key() == Qt::Key_Escape)
    {
        ke->accept();
        reject();
        update();
        onyx::screen::watcher().enqueue(0, onyx::screen::ScreenProxy::GC);
        return;
    }
    QDialog::keyReleaseEvent(ke);
}

void ZLLinkInfoDialog::linkClicked(int row)
{
    if (row >= 0 && row < list_of_links_.size())
    {
        link_selected_ = row;
        accept();
    }
}

void ZLLinkInfoDialog::onItemActivated(CatalogView *catalog,
                                   ContentView *item,
                                   int user_data)
{
    if (!item && !item->data())
    {
        return;
    }

    OData *data = item->data();
    if (data->contains(TAG_ROW))
    {
        bool ok;
        int row = data->value(TAG_ROW).toInt(&ok);
        if (ok)
        {
            linkClicked(row);
        }
    }
}
