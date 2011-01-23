#ifndef SIMSU_H
#define SIMSU_H

#include <QWidget>
#include "onyx/base/base.h"
#include "onyx/screen/screen_proxy.h"
#include "onyx/ui/ui.h"
#include "menudialog.h"

class QBoxLayout;
class QButtonGroup;
class QToolButton;
class QGridLayout;

using namespace ui;

namespace onyx {
namespace simsu {
class Board;
class Cell;
class Simsu : public QWidget {
        Q_OBJECT
    public:
        Simsu ( QWidget *parent = 0, Qt::WindowFlags f = 0 );
        ~Simsu(){};
    protected:
        void closeEvent ( QCloseEvent *event );
        void wheelEvent ( QWheelEvent *event );
        bool event ( QEvent *event );
        void keyPressEvent (QKeyEvent * event);
    private slots:
        void showBoard();
        void showMenu();
        void quit();
    private:
        Board *m_board;
        bool pop_;
};
}
}
#endif // SIMSU_H
