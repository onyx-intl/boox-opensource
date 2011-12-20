#ifndef INFO_DIALOG_H
#define INFO_DIALOG_H

#include "onyx/ui/tree_view_dialog.h"

namespace ui
{

class InfoDialog : public TreeViewDialog
{
    Q_OBJECT
public:
    explicit InfoDialog(QWidget *parent);
    ~InfoDialog(void);

public:
    int popup(const QString & title = "", const QModelIndex &idx = QModelIndex());
    void addLine(QString text, bool header = false);

private:
    QStandardItemModel item_model;
    NO_COPY_AND_ASSIGN(InfoDialog);

    int item_cnt;
};

} //namespace ui

#endif  // INFO_DIALOG_H
