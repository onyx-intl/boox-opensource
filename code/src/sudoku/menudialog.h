#ifndef MENUDIALOG_H
#define MENUDIALOG_H

#include <QDialog>
#include "onyx/ui/buttons.h"
#include "onyx/screen/screen_proxy.h"
#include "mtoolbutton.h"
#include "onyx/screen/screen_proxy.h"
#include "onyx/ui/message_dialog.h"
using namespace ui;
class SidebarButton : public OnyxPushButton
{
public:
    SidebarButton (const QString &text, QWidget *parent = 0 );

protected:
    virtual void keyPressEvent(QKeyEvent* e)
    {
        if (e->key()==Qt::Key_Return)
        {
            click();
        }
        onyx::screen::instance().setDefaultWaveform(onyx::screen::ScreenProxy::DW);
        OnyxPushButton::keyPressEvent(e);
    }
};

class MenuDialog : public QDialog
{
    Q_OBJECT
public:
    MenuDialog(QWidget* parent = 0);
    virtual ~MenuDialog(){
        list_act_buttons.clear();
    }
signals:
    void toCheck();
private slots:
    void newGame();
    //TODO
   // void showDetails();
    void about();
    void toCheck(bool);
protected:
    virtual void keyPressEvent(QKeyEvent* );
    virtual void mouseMoveEvent(QMouseEvent* );
    virtual bool event(QEvent *e);
private:
    int current_button_;
    QButtonGroup* act_buttons_;
    QList<SidebarButton *> list_act_buttons;
};

#endif // MENUDIALOG_H
