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
QAbstractButton::mouseMoveEvent(event);
}

bool MToolButton::event(QEvent* e)
{
    int ret = QToolButton::event(e);
    if ( e->type() == QEvent::UpdateRequest )  {
        if ( onyx::screen::instance().isUpdateEnabled() ) {
            static int count = 0;

            if ( onyx::screen::instance().defaultWaveform() == onyx::screen::ScreenProxy::DW ) {
                qDebug ( "Explorer screen ScreenProxy::DW update %d", count++ );
                onyx::screen::instance().updateWidget ( this, onyx::screen::ScreenProxy::DW, true );
                onyx::screen::instance().setDefaultWaveform ( onyx::screen::ScreenProxy::GU );

            } else  {
                qDebug ( "Explorer screen full update %d", count++ );
                onyx::screen::instance().updateWidget ( this, onyx::screen::ScreenProxy::GU );
            }
        }
    }
    return ret;
}
