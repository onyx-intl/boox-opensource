
#include <QtGui/QtGui>
#include "line_edit.h"
#include "onyx/screen/screen_update_watcher.h"
#include "number_dialog.h"

const QString LINE_EDIT_STYLE = "       \
QLineEdit                               \
{                                       \
    border: 2px solid dark;             \
    background: white;                  \
    selection-background-color: black;  \
    selection-color: white;             \
    font: 24px bold;                    \
    color: black;                       \
    border-width: 2px;                  \
    border-style: solid;                \
    border-radius: 0;                   \
    padding: 0px;                       \
    min-height: 32px;                   \
}                                       \
QLineEdit:disabled                      \
{                                       \
    border: 2px solid dark;             \
    background: white;                  \
    selection-background-color: black;  \
    selection-color: white;             \
    font: 24px bold;                    \
    color: dark;                       \
    border-width: 2px;                  \
    border-style: solid;                \
    border-radius: 0;                   \
    padding: 0px;                       \
    min-height: 32px;                   \
}";

namespace ui
{

lcl_OnyxLineEdit::lcl_OnyxLineEdit(QWidget *parent)
: QLineEdit(parent)
, out_of_range_(false)
{
    setStyleSheet(LINE_EDIT_STYLE);
    QApplication::setCursorFlashTime(0);
}

lcl_OnyxLineEdit::lcl_OnyxLineEdit(const QString & text, QWidget *parent)
: QLineEdit(text, parent)
, out_of_range_(false)
{
    setStyleSheet(LINE_EDIT_STYLE);
    QApplication::setCursorFlashTime(0);
}

lcl_OnyxLineEdit::~lcl_OnyxLineEdit()
{
}

void lcl_OnyxLineEdit::focusInEvent(QFocusEvent *e)
{
    QLineEdit::focusInEvent(e);
    emit getFocus(this);
}

void lcl_OnyxLineEdit::keyReleaseEvent(QKeyEvent *ke)
{
    if (ke->key() == Qt::Key_Escape)
    {
        ke->ignore();
        return;
    }

    if (out_of_range_ || (ke->key() == Qt::Key_Up || ke->key() == Qt::Key_Down))
    {
        out_of_range_ = false;
        qDebug("broadcast out of range signal.");
        ke->ignore();
        emit outOfRange(ke);
        return;
    }

    QLineEdit::keyReleaseEvent(ke);
    ke->accept();
}

void lcl_OnyxLineEdit::keyPressEvent(QKeyEvent * ke)
{
    if (ke->key() == Qt::Key_Enter || ke->key() == Qt::Key_Return)
    {
        NumberDialog dialog(0, name);
        if (dialog.popup(text().toInt(), maxval) == QDialog::Accepted) {
            int val = dialog.value();
            QString s_val;
            s_val = QString("%1").arg(val);
            setText(s_val);
    
            onyx::screen::instance().updateWidget(0, onyx::screen::ScreenProxy::GU);
            emit valueChanged(this);
        }
    }

    if ((ke->key() == Qt::Key_Left && cursorPosition() <= 0) ||
        (ke->key() == Qt::Key_Right && cursorPosition() >= text().size()))
    {
        out_of_range_ = true;
    }
    QLineEdit::keyPressEvent(ke);
    ke->accept();
    update();
    onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::DW, onyx::screen::ScreenCommand::WAIT_NONE);
}

void lcl_OnyxLineEdit::mouseReleaseEvent(QMouseEvent * event)
{
    event->accept();
    emit getFocus(this);
    emit setCheckByMouse(this);

    QKeyEvent key_event(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier, "virtual");
    QApplication::sendEvent(this, &key_event);
}

}
