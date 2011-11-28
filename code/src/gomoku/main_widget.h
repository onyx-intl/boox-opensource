#ifndef MAIN_WIDGET_H
#define MAIN_WIDGET_H

#include <QWidget>
#include "gomoku_widget.h"
#include "gomoku_actions.h"

#include "onyx/ui/ui.h"
#include "onyx/ui/status_bar.h"

class GomokuWidget;
class QStatusBar;
using namespace ui;

class MainWidget: public QWidget
{
    Q_OBJECT
public:
    MainWidget(QWidget *parent = 0);

protected:
    void keyPressEvent(QKeyEvent *ke);
    void keyReleaseEvent(QKeyEvent *ke);

private slots:
    void showMenu();

private:
    void about();

private:
    GomokuWidget *gomoku;
    StatusBar *status_bar_;

    SystemActions system_actions_;
    GomokuActions gomoku_actions_;
};

#endif // MAIN_WIDGET_H
