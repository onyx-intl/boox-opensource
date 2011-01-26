#ifndef SIMSU_H
#define SIMSU_H

#include <QWidget>
#include "onyx/base/base.h"
#include "onyx/screen/screen_proxy.h"
#include "onyx/ui/ui.h"
#include "sudokuactions.h"

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
        bool event ( QEvent *event );
        void keyPressEvent (QKeyEvent * event);
    private slots:
        void showBoard();
        void showMenu();
        void newGame();
        void checkGame();
        void about();
        void quit();
    private:
        Board *m_board;
        StatusBar status_bar_;
        SystemActions system_actions_;
        SudokuActions sudoku_actions_;
};
}
}
#endif // SIMSU_H
