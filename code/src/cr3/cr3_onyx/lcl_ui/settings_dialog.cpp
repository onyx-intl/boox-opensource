#include "onyx/screen/screen_update_watcher.h"
#include "onyx/screen/screen_proxy.h"
#include "onyx/ui/keyboard_navigator.h"
#include "onyx/sys/sys_status.h"
#include "onyx/ui/label.h"
#include "onyx/ui/ui_utils.h"
#include "settings_dialog.h"
#include "number_dialog.h"
#include "line_edit.h"

namespace ui
{

SettingsDialog::SettingsDialog(QWidget *parent)
    : OnyxDialog(parent)
    , ver_layout_(&content_widget_)
    , hor_layout_(0)
    , ok_(QApplication::tr("OK"), 0)
    , settings_dialog_group(0)
{
    save = false;
}

SettingsDialog::~SettingsDialog(void)
{
    /*
    std::vector<OnyxCheckBox*> button_v;
    std::vector<OnyxLabel*> label_v;
    std::vector<lcl_OnyxLineEdit*> lineedit_v;
    std::vector<QHBoxLayout*> hboxlayout_v;
    */
    {
        std::vector<OnyxCheckBox*>::iterator it = button_v.begin();
        while (it != button_v.end())
            delete *it++;
    }
    {
        std::vector<OnyxLabel*>::iterator it = label_v.begin();
        while (it != label_v.end())
            delete *it++;
    }
    {
        std::vector<lcl_OnyxLineEdit*>::iterator it = lineedit_v.begin();
        while (it != lineedit_v.end())
            delete *it++;
    }
    {
        std::vector<QHBoxLayout*>::iterator it = hboxlayout_v.begin();
        while (it != hboxlayout_v.end())
            delete *it++;
    }
    
    /*
    if (button_v.size() > 0)
        delete[] button_v[0];

    if (label_v.size() > 0)
        delete[] label_v[0];

    if (lineedit_v.size() > 0)
        delete[] lineedit_v[0];

    if (hboxlayout_v.size() > 0)
        delete[] hboxlayout_v[0];
        */
}

int SettingsDialog::exec()
{
    setModal(true);
    resize(400, 540);
    createLayout();

    shadows_.show(true);
    show();
    onyx::screen::instance().flush();
    onyx::screen::instance().updateWidgetRegion(
        0,
        outbounding(parentWidget()),
        onyx::screen::ScreenProxy::GC,
        false,
        onyx::screen::ScreenCommand::WAIT_ALL);
    return QDialog::exec();
}

void SettingsDialog::keyPressEvent(QKeyEvent *ke)
{
    ke->accept();
}

void SettingsDialog::keyReleaseEvent(QKeyEvent *ke)
{
    QWidget *wnd = 0;
    ke->accept();
    switch (ke->key())
    {
    case Qt::Key_Up:
        this->focusPreviousChild();
        break;
    case Qt::Key_Down:
        this->focusNextChild();
        break;
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_PageDown:
    case Qt::Key_PageUp:
        wnd = ui::moveFocus(&content_widget_, ke->key());
        if (wnd)
        {
            wnd->setFocus();
        }
        break;
    case Qt::Key_Return:
        break;
    case Qt::Key_Escape:
        reject();
        break;
    }
}

void SettingsDialog::createLayout()
{
    content_widget_.setBackgroundRole(QPalette::Button);

    updateTitle(QApplication::tr("CoolReader Settings"));

    // The big layout.
    ver_layout_.setContentsMargins(SPACING, 0, SPACING, 0);
    ver_layout_.addSpacing(10);

    OnyxCheckBox * btn;
#define add_button(_name_, _txt_) \
    btn = new OnyxCheckBox(_txt_, 0); \
    button_v.push_back(btn); \
    btn->selectOnClicked(false); \
    btn->setChecked(_name_); \
    connect(btn, SIGNAL(clicked(bool)), this, SLOT(onButtonChanged_##_name_(bool)), Qt::QueuedConnection); \
    settings_dialog_layout.addWidget(btn);

    OnyxLabel *le_label;
    lcl_OnyxLineEdit *le_obj;
    QHBoxLayout *le_layout;
#define add_lineedit(var, text, max) \
    le_label = new OnyxLabel(0); \
    label_v.push_back(le_label); \
    le_label->setMargin(0); \
    le_label->setText(text); \
    le_obj = new lcl_OnyxLineEdit(0); \
    lineedit_v.push_back(le_obj); \
    le_obj->setAlignment(Qt::AlignRight); \
    le_obj->setText(var); \
    le_obj->setName(text); \
    le_obj->setMaxVal(max); \
    connect(le_obj, SIGNAL(valueChanged(lcl_OnyxLineEdit*)), this, SLOT(onValueChanged_##var(lcl_OnyxLineEdit*)), Qt::QueuedConnection); \
    le_layout = new QHBoxLayout; \
    hboxlayout_v.push_back(le_layout); \
    le_layout->setContentsMargins(0, 0, 0, 0); \
    le_layout->addWidget(le_label); \
    le_layout->addWidget(le_obj); \
    settings_dialog_layout.addLayout(le_layout);

    add_lineedit(font_size, QCoreApplication::tr("Font Size"), 60);
    add_button(font_aa, QCoreApplication::tr("Font antialiasing"));
    add_button(status_line, QCoreApplication::tr("Show status line"));
    if (is97inch())
    {
        add_button(two_pages_landscape, QCoreApplication::tr("Two lanscape pages"));
    }
    add_button(display_inverse, QCoreApplication::tr("Inverse display"));
    add_lineedit(l_margin, QCoreApplication::tr("Left Margin"), 130);
    add_lineedit(r_margin, QCoreApplication::tr("Right Margin"), 130);
    add_lineedit(t_margin, QCoreApplication::tr("Top Margin"), 130);
    add_lineedit(b_margin, QCoreApplication::tr("Bottom Margin"), 130);

    ver_layout_.addLayout(&settings_dialog_layout);

    // OK cancel buttons.
    connect(&ok_, SIGNAL(clicked(bool)), this, SLOT(onOkClicked(bool)));

    ok_.useDefaultHeight();
    ok_.setCheckable(false);
    ok_.setFocusPolicy(Qt::TabFocus);
    hor_layout_.addStretch(0);
    hor_layout_.addWidget(&ok_);

    ver_layout_.addStretch(0);
    ver_layout_.addLayout(&hor_layout_);
}

#define onButtonChanged(_name_) \
void SettingsDialog::onButtonChanged_##_name_(bool) \
{ \
    _name_ = !_name_; \
}

onButtonChanged(status_line);
onButtonChanged(show_time);
onButtonChanged(font_bold);
onButtonChanged(font_aa);
onButtonChanged(two_pages_landscape);
onButtonChanged(display_inverse);

#define onValueChanged(var) \
void SettingsDialog::onValueChanged_##var(lcl_OnyxLineEdit *object) \
{ \
    var = object->text(); \
}

onValueChanged(l_margin);
onValueChanged(r_margin);
onValueChanged(t_margin);
onValueChanged(b_margin);
onValueChanged(font_size);

bool SettingsDialog::event(QEvent* qe)
{
    bool ret = QDialog::event(qe);
    if (qe->type() == QEvent::UpdateRequest
            && onyx::screen::instance().isUpdateEnabled())
    {
        // onyx::screen::instance().sync(&shadows_.hor_shadow());
        // onyx::screen::instance().sync(&shadows_.ver_shadow());
        onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::DW);
    }
    return ret;
}

void SettingsDialog::onOkClicked(bool)
{
    save = true;
    accept();
}

}   // namespace ui

