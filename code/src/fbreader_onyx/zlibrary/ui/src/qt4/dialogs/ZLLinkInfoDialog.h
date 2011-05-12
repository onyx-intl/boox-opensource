#ifndef ZL_LINK_INFO_DIALOG_H_
#define ZL_LINK_INFO_DIALOG_H_

#include "onyx/base/base.h"
#include "onyx/ui/ui_global.h"
#include "onyx/ui/ui_utils.h"
#include "onyx/ui/catalog_view.h"

using namespace ui;

class ZLLinkInfoDialog: public QWidget
{
    Q_OBJECT

public:
    explicit ZLLinkInfoDialog(QWidget *parent, QVector<char *> list_of_links);
    ~ZLLinkInfoDialog();

    void popup();

protected Q_SLOTS:
    void onItemActivated(CatalogView *catalog, ContentView *item,
            int user_data);

private:
    void createLayout();
    void createLinks();
    void connectWithChildren();

    void linkClicked(int row);

private:
    QVBoxLayout big_layout_;

    CatalogView links_;
    ODatas links_datas_;

    QVector<char *> list_of_links_;
};

#endif
