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

    protected:
        virtual void closeEvent ( QCloseEvent *event );
        virtual void wheelEvent ( QWheelEvent *event );
        virtual bool event ( QEvent *event );
        virtual void keyPressEvent (QKeyEvent * event);
    private slots:
        void showBoard();
        void showMenu();
    private:
        Board *m_board;
        QGridLayout *m_layout;
        bool pop_;
};
}
}
#endif // SIMSU_H
