#include <QButtonGroup>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMessageBox>
#include <QSettings>
#include <QSpinBox>
#include <QUndoStack>

#include <QVBoxLayout>
#include <QWheelEvent>
#include <QKeyEvent>

#include <onyx/ui/base_thumbnail.h>
#include "onyx/ui/ui.h"
#include <onyx/sys/sys.h>

#include "simsu.h"
#include "board.h"
#include "pattern.h"
#include "square.h"
#include "cell.h"
#include "mdialog.h"

using namespace ui;

namespace onyx {
namespace simsu {

Simsu::Simsu ( QWidget *parent , Qt::WindowFlags f ) : QWidget ( parent, f ) {
    int screenWidth = QApplication::desktop()->screenGeometry().width();
    int screenHeight = QApplication::desktop()->screenGeometry().height();
    setWindowFlags(Qt::FramelessWindowHint);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    QSettings settings;
    // Create board
    Square *square = new Square ( this );
    m_board = new Board ( square );
    m_board->setAutoSwitch(false);
    square->setChild ( m_board );
    QGridLayout *m_layout = new QGridLayout ( this );
    m_layout->addWidget(m_board,0,0);
    setLayout(m_layout);
    showMaximized();
    onyx::screen::instance().enableUpdate ( true );
    onyx::screen::instance().setDefaultWaveform ( onyx::screen::ScreenProxy::GC );
}

/*****************************************************************************/

void Simsu::closeEvent ( QCloseEvent *event ) {
    QSettings().setValue ( "Geometry", saveGeometry() );
    QWidget::closeEvent ( event );
}

/*****************************************************************************/

void Simsu::wheelEvent ( QWheelEvent *event ) {

}



bool Simsu::event ( QEvent *event )
{
    bool ret = QWidget::event ( event );
    if (event->type() == QEvent::UpdateRequest /*&& global_update*/)
    {
        onyx::screen::instance().updateWidget(0);
        onyx::screen::instance().setDefaultWaveform(onyx::screen::ScreenProxy::GC);
    }
    return ret;
}



/*****************************************************************************/
void Simsu::keyPressEvent(QKeyEvent* event)
{
    /**
    Menu    Qt::Key_Menu
    Escape/Back     Qt::Key_Escape
    PageUp  Qt::Key_PageUp
    PageDown    Qt::Key_PageDown
    Left    Qt::Key_Left
    Right   Qt::Key_Right
    Up  Qt::Key_Up
    Down    Qt::Key_Down
    OK  Qt::Key_Return
    */
    //TODO encapsulate each part as functions
    switch (event->key())
    {
    case Qt::Key_Escape:
        //use menudialog to quit
       // qApp->quit();
        break;
    case Qt::Key_PageUp:
        break;

    case Qt::Key_Menu:
    {
        //TODO open menudialog
        showMenu();
    }
    break;

    case Qt::Key_Return:
    {
        showBoard();
    }
    break;

    default:
        QWidget::keyPressEvent(event);
        break;
    }
}

void Simsu::showBoard() {
    int column = m_board->getColumn();
    int row = m_board->getRow();
    QPoint point = m_board->pos();
    if (!m_board->cell(column,row)->given()) {
        MDialog *dialog = new MDialog(parentWidget());
        int screenWidth = QApplication::desktop()->screenGeometry().width();
        int screenHeight = QApplication::desktop()->screenGeometry().height();
        int bordersize = qMin(screenHeight, screenWidth);
        if (column < 4.5) {
            column += 1;
        } else if (column > 4.5) {
            column -= 3;
        }

        if (row < 4.5) {
            row += 1;
        } else if (row > 4.5) {
            row -= 3;
        }
        dialog->setGeometry(point.x() + (column)*(bordersize)/9,
                            point.y() + (row)*(bordersize)/9,bordersize/3,bordersize/3);
        connect(dialog, SIGNAL(ActiveKey(int)), m_board, SLOT(setActiveKey(int)));
        if (dialog->exec() == QDialog::Accepted) {
            m_board->cell(m_board->getColumn(),m_board->getRow())->updateValue();
            delete dialog;
            onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::INVALID);
        } else {
            //if rejected
            delete dialog;
            onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::INVALID);
        }
        //onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::INVALID);
    }
    return;
}
void Simsu::showMenu()
{
    MenuDialog* menudialog = new MenuDialog(parentWidget());
    int screenWidth = QApplication::desktop()->screenGeometry().width();
    int screenHeight = QApplication::desktop()->screenGeometry().height();
    connect(menudialog, SIGNAL(toCheck()),m_board,SLOT(showWrong()));
    connect(menudialog, SIGNAL(askQuit()),this,SLOT(quit()));
    menudialog->move(screenWidth/2-20,screenHeight/2-40);
    if (menudialog->exec())
    menudialog->deleteLater();
}

void Simsu::quit()
{
    qApp->quit();
}

}
}

