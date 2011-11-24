#include "onyx/sys/sys.h"
#include "onyx/screen/screen_proxy.h"
#include "recent_books.h"

namespace ui
{

RecentBooks::RecentBooks(QWidget *parent)
    : TreeViewDialog(parent)
{
}

RecentBooks::~RecentBooks(void)
{
}

int RecentBooks::selectedInfo()
{
    int idx;

    QModelIndex index = selectedItem();
    if (index.isValid())
    {
        QStandardItemModel * time_zones = &item_model;
        QStandardItem *item = time_zones->itemFromIndex( index );
        idx = item->data().toInt();
    }

    return idx;
}

int RecentBooks::popup(const QString &title,
                          const QModelIndex &index)
{
    QStandardItemModel * time_zones = getInfos();
    setModel(time_zones);

    title_label_.setText(title);
    status_bar_.setProgress(tree_.currentPage(), tree_.pages());

    showMaximized();
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);
    return exec();
}

QStandardItemModel* RecentBooks::getInfos()
{
    item_model.clear();
    item_model.setColumnCount(1);

    QStandardItem *item;
    int i;
    for (i = 0; i < books.size(); i++) {
        item = new QStandardItem(books.at(i));
        item->setData( i );
        item_model.setItem( i, 0, item );
    }

    //item_model.setHeaderData(0, Qt::Horizontal, QVariant::fromValue(QCoreApplication::tr("Test")), Qt::DisplayRole);
    return &item_model;
}

} // namespace ui
