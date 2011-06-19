#ifndef MDIALOG_H
#define MDIALOG_H

#include <QtGui/QDialog>

#include "mtoolbutton.h"
class QPainter;
class QGridLayout;
class QButtonGroup;
class MDialog : public QDialog
{
    Q_OBJECT
    public:
        MDialog(QWidget* parent = 0);
    //virtual ~MDialog(){/*list_key.clear();*/}
    signals:
        void ActiveKey(qint32);
        void ActiveModeKey(qint32);
    private slots:
        void setActiveKey(int);
        void setActiveModeKey(qint32);
    protected:
        virtual void keyPressEvent(QKeyEvent* );
        virtual void mouseMoveEvent(QMouseEvent* );
        virtual bool event(QEvent *e);
        virtual void paintEvent(QPaintEvent* );
    private:
        QList<MToolButton*> list_key;
        QGridLayout *layout_key;
        QButtonGroup *group_key;
};

#endif // MDIALOG_H
