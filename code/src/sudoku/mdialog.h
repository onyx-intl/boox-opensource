#ifndef MDIALOG_H
#define MDIALOG_H

#include <QDialog>
#include "mtoolbutton.h"

class MDialog : public QDialog
{
    Q_OBJECT
    public:
        MDialog(QWidget* parent = 0);
    signals:
        void ActiveKey(int);
    private slots:
        void setActiveKey(int);
    protected:
        virtual void keyPressEvent(QKeyEvent* );
        virtual void mouseMoveEvent(QMouseEvent* );
        virtual bool event(QEvent *e);
    private:
        QList<MToolButton*> list;
        int current_button_;
};

#endif // MDIALOG_H
