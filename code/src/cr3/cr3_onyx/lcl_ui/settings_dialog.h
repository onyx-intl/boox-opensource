#ifndef SETTINGS_DIALOG_H_
#define SETTINGS_DIALOG_H_

#include "onyx/base/base.h"
#include "onyx/sys/sys.h"
#include "onyx/ui/onyx_dialog.h"
#include "crqtutil.h"

namespace ui
{
class lcl_OnyxLineEdit;

class SettingsDialog : public OnyxDialog
{
    Q_OBJECT

public:
    SettingsDialog(QWidget *parent);
    ~SettingsDialog(void);

public:
    int exec();

    bool save;
    bool status_line;
    bool show_time;
    bool font_bold;
    bool font_aa;
    bool two_pages_landscape;
    bool display_inverse;

    QString l_margin, r_margin, t_margin, b_margin;
    QString font_size;

private:
    void createLayout();
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);
    bool event(QEvent*);

private Q_SLOTS:
    void onButtonChanged_status_line(bool);
    void onButtonChanged_show_time(bool);
    void onButtonChanged_font_bold(bool);
    void onButtonChanged_font_aa(bool);
    void onButtonChanged_two_pages_landscape(bool);
    void onButtonChanged_display_inverse(bool);

    void onValueChanged_l_margin(lcl_OnyxLineEdit *object);
    void onValueChanged_r_margin(lcl_OnyxLineEdit *object);
    void onValueChanged_t_margin(lcl_OnyxLineEdit *object);
    void onValueChanged_b_margin(lcl_OnyxLineEdit *object);
    void onValueChanged_font_size(lcl_OnyxLineEdit *object);

    void onOkClicked(bool);

private:
    typedef OnyxCheckBox * CheckBoxPtr;
    typedef std::vector<CheckBoxPtr> Buttons;
    typedef std::vector<CheckBoxPtr>::iterator ButtonsIter;

    QVBoxLayout ver_layout_;
    QHBoxLayout hor_layout_;
    OnyxPushButton ok_;

    QVBoxLayout settings_dialog_layout;
    QButtonGroup settings_dialog_group;

    std::vector<OnyxCheckBox*> button_v;
    std::vector<OnyxLabel*> label_v;
    std::vector<lcl_OnyxLineEdit*> lineedit_v;
    std::vector<QHBoxLayout*> hboxlayout_v;
};

}   // namespace ui

#endif      // SETTINGS_DIALOG_H_
