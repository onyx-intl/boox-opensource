#ifndef RSSFEEDDIALOG_H
#define RSSFEEDDIALOG_H

#include "onyx/base/base.h"
#include "onyx/ui/ui_global.h"
#include "onyx/ui/onyx_dialog.h"
#include "onyx/ui/onyx_keyboard.h"
#include "onyx/ui/line_edit_view_group.h"

using namespace ui;

class RssFeedDialog: public OnyxDialog
{
    Q_OBJECT

public:
    RssFeedDialog(const QString & str, QWidget *parent);
    ~RssFeedDialog();

public:
    bool popup();
    QString value(OData * d_index = 0);
    QString title();
    QString url();
    void setTitle(const QString & title);
    void setUrl(const QString & url);

protected Q_SLOTS:
    void onItemActivated(CatalogView *catalog, ContentView *item,
            int user_data);

private:
    void appendDefaultPasswordEdit();
    void addLineEditsToGroup();

    void createLayout();
    void createLineEdits(const int &line_edit_width);
    void createSubMenu(const int &sub_menu_width);
    void connectWithChildren();

    CatalogView * createEditItem(OData *data, int index, ODatas *edit_datas,
            const int &line_edit_width);

    void clearClicked();

    void keyPressEvent(QKeyEvent *event);
    void init();

private:
    QVBoxLayout big_layout_;
    QHBoxLayout *line_edit_layout_;

    CatalogView sub_menu_;
    QVector<CatalogView *> edit_view_list_;
    LineEditViewGroup edit_view_group_;

    ODatas sub_menu_datas_;
    QVector<ODatas *> all_line_edit_datas_;

    OnyxKeyboard keyboard_;
    QString title_;
    ODatas edit_list_;

    ODataPtr title_data;
    ODataPtr url_data;
    QString feed_title_;
    QString feed_url_;
};

#endif // RSSFEEDDIALOG_H
