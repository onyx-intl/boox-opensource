#include "onyx/ui/keyboard_navigator.h"
#include "number_dialog.h"
#include "onyx/screen/screen_proxy.h"

namespace ui
{

const int MARGINS = 5;

NumberDialog::NumberDialog(QWidget *parent, QString title)
    : OnyxDialog(parent)
    , value_(0)
    , total_(0)
    , validator_(this)
    , number_edit_(this)
    , number_widget_(this)
{
    OnyxDialog::updateTitle(title);
    setModal(true);

    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Button);

    connect(&number_widget_,
            SIGNAL(numberClicked(const int)),
            this,
            SLOT(onNumberClicked(const int)));
    connect(&number_widget_,
            SIGNAL(okClicked()),
            this,
            SLOT(onOKClicked()));
    connect(&number_widget_,
            SIGNAL(backspaceClicked()),
            this,
            SLOT(onBackspaceClicked()));

    createLayout();
}

NumberDialog::~NumberDialog(void)
{
}

/// Set the number.
void NumberDialog::setValue(const int value)
{
    value_ = value;
    QString text;
    text.setNum(value_);
    number_edit_.setText(text);
}

int NumberDialog::popup(const int value, const int total)
{
    total_ = total;

    validator_.setRange(1, total);
    number_edit_.setValidator(&validator_);

    shadows_.show(true);
    show();

    number_edit_.selectAll();
    int w = contentsRect().width() - 2 * MARGINS;
    number_edit_.setFixedWidth(w);
    onyx::screen::instance().flush();
    onyx::screen::instance().updateWidgetRegion(
        0,
        outbounding(parentWidget()),
        onyx::screen::ScreenProxy::GC,
        false,
        onyx::screen::ScreenCommand::WAIT_ALL);
    return QDialog::exec();
}

void NumberDialog::createLayout()
{
    // Create number label.
    //number_edit_.setFrameShadow(QFrame::Raised);
    number_edit_.setAlignment(Qt::AlignRight);

    // label.
    // setValue(value_);

    // Add to layout.
    vbox_.setSizeConstraint(QLayout::SetDefaultConstraint);
    vbox_.setContentsMargins(MARGINS, MARGINS, MARGINS, MARGINS);
    number_edit_.setFocusPolicy(Qt::NoFocus);
    vbox_.addWidget(&number_edit_, 0, Qt::AlignRight|Qt::AlignVCenter);
    vbox_.addSpacing(MARGINS);
    vbox_.addWidget(&number_widget_, 0, Qt::AlignCenter);
}

void NumberDialog::onOKClicked()
{
    value_ = number_edit_.text().toInt();
    if (value_ > 0 && value_ <= total_)
    {
        done(QDialog::Accepted);
    }
    else
    {
        done(QDialog::Rejected);
    }
}

void NumberDialog::keyPressEvent(QKeyEvent *e)
{
    e->accept();
}

void NumberDialog::keyReleaseEvent(QKeyEvent *e)
{
    //   copy from QDialog::keyPressEvent
    //   Calls reject() if Escape is pressed. Simulates a button
    //   click for the default button if Enter is pressed. Move focus
    //   for the arrow keys. Ignore the rest.
#ifdef Q_WS_MAC
    if(e->modifiers() == Qt::ControlModifier && e->key() == Qt::Key_Period)
    {
        reject();
    }
    else
#endif
    // Check the current selected type.
    e->accept();
    switch (e->key())
    {
    case Qt::Key_Up:
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Down:
        {
            QWidget *wnd = ui::moveFocus(this, e->key());
            if (wnd)
            {
                wnd->setFocus();
            }
        }
        break;
    case Qt::Key_Escape:
    case Device_Menu_Key:
        reject();
        break;
    case Qt::Key_Return:
        break;
    }
}

bool NumberDialog::event(QEvent * event)
{
    bool ret = OnyxDialog::event(event);
    if (event->type() == QEvent::UpdateRequest && onyx::screen::instance().isUpdateEnabled())
    {
        onyx::screen::instance().updateWidget(
            this,
            onyx::screen::ScreenProxy::DW,
            false,
            onyx::screen::ScreenCommand::WAIT_NONE);
    }
    return ret;
}

void NumberDialog::onCloseClicked()
{
    onyx::screen::instance().enableUpdate(false);
    reject();
    QApplication::processEvents();
    onyx::screen::instance().enableUpdate(true);
    onyx::screen::instance().updateWidget(0, onyx::screen::ScreenProxy::GC);
}

void NumberDialog::onNumberClicked(const int number)
{
    // Disable the parent widget to update screen.
    QString text("%1");
    text = text.arg(number);
    QKeyEvent * key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_0 + number, Qt::NoModifier, text);
    QApplication::postEvent(&number_edit_, key_event);

    update();
    onyx::screen::instance().updateWidget(
        &number_edit_,
        onyx::screen::ScreenProxy::DW,
        false,
        onyx::screen::ScreenCommand::WAIT_ALL);
}

void NumberDialog::onBackspaceClicked()
{
    QKeyEvent * key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier, "");
    QApplication::postEvent(&number_edit_, key_event);

    update();
    onyx::screen::instance().updateWidget(
        &number_edit_,
        onyx::screen::ScreenProxy::DW,
        false,
        onyx::screen::ScreenCommand::WAIT_ALL);
}

}   // namespace ui
