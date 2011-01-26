#include "mtoolbutton.h"
#include <QtGui/QKeyEvent>
#include "onyx/screen/screen_proxy.h"

MToolButton::MToolButton(QWidget* parent): QToolButton(parent)
{
    setStyleSheet("\
    border-width: 3px;                  \
    border-color: black;                \
    border-style: solid;                \
    border-radius: 5;                   \
    background-color: white;\
    color: black;\
    padding: 0px;");
    setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    setFont(QFont("Serif", 36, QFont::Bold));
    setMaximumSize(44,44);
    setMinimumSize(44,44);
}

void MToolButton::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Return) {
        click();
    }
    QAbstractButton::keyPressEvent(e);
}


void MToolButton::focusInEvent ( QFocusEvent* event ) {
    //TODO
    setStyleSheet("\
    border-width: 6px;                  \
    border-color: black;                \
    border-style: solid;                \
    border-radius: 6;                   \
    background-color: black;\
    color: white;\
    padding: 0px;");
    //QToolButton::focusInEvent ( event );
}

/*****************************************************************************/

void MToolButton::focusOutEvent ( QFocusEvent* event ) {
    setStyleSheet("\
    border-width: 3px;                  \
    border-color: black;                \
    border-style: solid;                \
    border-radius: 5;                   \
    background-color: white;\
    color: black;\
    padding: 0px;");
   // QToolButton::focusOutEvent ( event );
}

void MToolButton::mouseMoveEvent(QMouseEvent* event)
{
    setFocus();
//     QToolButton::mouseMoveEvent(event);
}

bool MToolButton::event(QEvent* e)
{

    bool ret = QToolButton::event(e);
//     if (e->type() == QEvent::UpdateRequest)
//     {
//         qDebug() << "MToolButton::event";
//         onyx::screen::instance().updateWidget(this);
//     }
    return ret;
}
