#include "mtoolbutton.h"
#include "QKeyEvent"
#include "onyx/screen/screen_proxy.h"

MToolButton::MToolButton(QWidget* parent): QToolButton(parent)
{
    setStyleSheet("border: 2px solid black");
    onyx::screen::instance().enableUpdate ( true );
    onyx::screen::instance().setDefaultWaveform(onyx::screen::ScreenProxy::GU);
    setFont(QFont("Serif", 36, QFont::Bold));
    setAutoRaise(true);
}

void MToolButton::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Return) {
        click();
    }
    QAbstractButton::keyPressEvent(e);
}


void MToolButton::focusInEvent ( QFocusEvent* event ) {
    setFont(QFont("Serif", 24, QFont::Light));
    QToolButton::focusInEvent ( event );
}

/*****************************************************************************/

void MToolButton::focusOutEvent ( QFocusEvent* event ) {
    setFont(QFont("Serif", 36, QFont::Bold));
    QToolButton::focusOutEvent ( event );
}

void MToolButton::mouseMoveEvent(QMouseEvent* event)
{
    setFocus();
    QToolButton::mouseMoveEvent(event);
}

bool MToolButton::event(QEvent* e)
{

    bool ret = QToolButton::event(e);
    if (e->type() == QEvent::UpdateRequest)
    {
        qDebug() << "MToolButton::event";
        onyx::screen::instance().updateWidget(this);
    }
    return ret;
}
