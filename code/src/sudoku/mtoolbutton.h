#ifndef MTOOLBUTTON_H
#define MTOOLBUTTON_H

#include <QtGui/QToolButton>


class MToolButton : public QToolButton
{
    Q_OBJECT
    public:
        MToolButton(QWidget* parent = 0);
    protected:
        virtual void keyPressEvent(QKeyEvent* e);
        virtual void focusOutEvent ( QFocusEvent* event );
        virtual void focusInEvent ( QFocusEvent* event );
        virtual void mouseMoveEvent ( QMouseEvent* event );
        virtual bool event(QEvent *e);
};

#endif // MTOOLBUTTON_H
