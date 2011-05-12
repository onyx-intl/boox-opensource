#include "ZLLinkInfoDialog.h"
#include "onyx/data/data_tags.h"
#include "onyx/screen/screen_update_watcher.h"

static const QString TAG_ROW = "row";
static const int SUBSET_ITEM_HEIGHT = 36;

ZLLinkInfoDialog::ZLLinkInfoDialog(QWidget *parent,
        QVector<char *> list_of_links)
    : QWidget(parent, Qt::FramelessWindowHint)
    , big_layout_(this)
    , links_(0, this)
    , list_of_links_(list_of_links)
{
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Button);

    createLayout();
    connectWithChildren();
    onyx::screen::watcher().addWatcher(this);
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
    big_layout_.addWidget(&links_, 0, Qt::AlignBottom);
}

void ZLLinkInfoDialog::createLinks()
{
    links_.setFixedHeight(keyboardKeyHeight());
    links_.setSearchPolicy(CatalogView::NeighborFirst
            | CatalogView::AutoVerRecycle);

    ODataPtr item(new OData);
    if (list_of_links_.size() > 0)
    {
        item->insert(TAG_TITLE, list_of_links_.first());
    }
    else
    {
        item->insert(TAG_TITLE, "Here is link item 1");
    }

    links_datas_.push_back(item);

    links_.setData(links_datas_);
    links_.setFixedGrid(links_datas_.size(), 1);
}

void ZLLinkInfoDialog::connectWithChildren()
{
    connect(&links_, SIGNAL(itemActivated(CatalogView *, ContentView *, int)),
            this, SLOT(onItemActivated(CatalogView *, ContentView *, int)));
}

void ZLLinkInfoDialog::popup()
{
    if (isHidden())
    {
        show();
    }
    QWidget * widget = safeParentWidget(parentWidget());
        resize(widget->width(), 200);
}

void ZLLinkInfoDialog::linkClicked(int row)
{

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
