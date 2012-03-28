#ifndef SETTING_MARGIN_DIALOG_H
#define SETTING_MARGIN_DIALOG_H

#include "onyx/base/base.h"
#include "onyx/sys/sys.h"
#include "onyx/ui/onyx_dialog.h"
#include "onyx/ui/catalog_view.h"

using namespace ui;

class MarginSettingDialog : public OnyxDialog
{
    Q_OBJECT

public:
    MarginSettingDialog(QWidget *parent=0);
    ~MarginSettingDialog(void);

    int getMarginValue() { return margin_;}

public:
    int exec();

private:
    void createLayout();
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);
    bool event(QEvent*);

private Q_SLOTS:
    void onButtonChanged(CatalogView *catalog, ContentView *item, int user_data);
    void onCancelClicked();

private:
    QVBoxLayout ver_layout_;
    CatalogView buttons_;

    int margin_;

    QHBoxLayout hor_layout_;
    CatalogView cancel_;
};

#endif // SETTING_MARGIN_DIALOG_H
