#include "onyx/sys/sys.h"
#include "onyx/screen/screen_proxy.h"
#include "info_dialog.h"

namespace ui
{

InfoDialog::InfoDialog(QWidget *parent)
    : TreeViewDialog(parent), item_cnt(0)
{
    item_model.clear();
    item_model.setColumnCount(1);
}

InfoDialog::~InfoDialog(void)
{
}

int InfoDialog::popup(const QString &title,
                          const QModelIndex &index)
{
    setModel(&item_model);

    title_label_.setText(title);
    status_bar_.setProgress(tree_.currentPage(), tree_.pages());

    showMaximized();
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);
    return exec();
}

void InfoDialog::addLine(QString text, bool header)
{
    QStandardItem *item = new QStandardItem(text);
    item_model.setItem(item_cnt++ , 0, item);

    if (header)
        item->setTextAlignment(Qt::AlignHCenter);
}

} // namespace ui
