#ifndef RECENT_BOOKS_H_
#define RECENT_BOOKS_H_

#include "onyx/ui/tree_view_dialog.h"

namespace ui
{

class RecentBooks : public TreeViewDialog
{
    Q_OBJECT
public:
    explicit RecentBooks(QWidget *parent);
    int i;
    ~RecentBooks(void);

public:
    int popup(const QString & title = "", const QModelIndex &idx = QModelIndex());
    int selectedInfo();
    vector<QString> books;

private:
    QStandardItemModel * getInfos();

private:
    QStandardItemModel item_model;
    NO_COPY_AND_ASSIGN(RecentBooks);
};

} //namespace ui

#endif  // ONYX_TIME_ZONE_DIALOG_H_
