#include <QButtonGroup>
#include <QDialog>
#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMessageBox>
#include <QSettings>

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

Simsu::Simsu ( QWidget *parent , Qt::WindowFlags f ) : QWidget ( parent, f ),
status_bar_(0,  MENU | PROGRESS|CONNECTION | BATTERY | MESSAGE | CLOCK | SCREEN_REFRESH){
    connect(&status_bar_, SIGNAL(menuClicked()), this, SLOT(showMenu()));
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
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addWidget(m_board,0,0);
    m_layout->addItem(new QSpacerItem(1,1,QSizePolicy::Maximum,QSizePolicy::Minimum),0,1);
    m_layout->addItem(new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Maximum),1,0);
    m_layout->addItem(new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Minimum),1,1);
    m_layout->addWidget(&status_bar_,2,0,1,2);
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


bool Simsu::event ( QEvent *event )
{
    bool ret = QWidget::event ( event );
    if (event->type() == QEvent::UpdateRequest /*&& global_update*/)
    {
        onyx::screen::instance().updateWidget(0);
    }
    onyx::screen::instance().setDefaultWaveform(onyx::screen::ScreenProxy::GU);
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
        }
        delete dialog;
        onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::INVALID);
    }
    return;
}
//TODO standard onyx UI menu
void Simsu::showMenu()
{
    PopupMenu menu(this);
    sudoku_actions_.generateActions();
    menu.addGroup(&sudoku_actions_);
    std::vector<int> all;
    all.push_back(ROTATE_SCREEN);
    all.push_back(MUSIC);
    all.push_back(RETURN_TO_LIBRARY);
    system_actions_.generateActions(all);
    menu.setSystemAction(&system_actions_);
    if (menu.popup() != QDialog::Accepted)
    {
        QApplication::processEvents();
        return;
    }

    QAction * group = menu.selectedCategory();
    if (group == sudoku_actions_.category())
    {
        int index = sudoku_actions_.selected();
        switch(index) {
            case NEW:
                newGame();
                break;
            case CHECK:
                checkGame();
                break;
            case ABOUT:
                about();
                break;
            default:
                break;
        }
    }
    else if (group == system_actions_.category())
    {
        SystemAction system_action = system_actions_.selected();
        switch (system_action)
        {
            case RETURN_TO_LIBRARY:
            {
                close();
            }
            break;
            case ROTATE_SCREEN:
            {
                sys::SysStatus::instance().rotateScreen();
            }
            break;
            case SCREEN_UPDATE_TYPE:
            {
                onyx::screen::instance().updateWidget(0, onyx::screen::ScreenProxy::GU);
                onyx::screen::instance().toggleWaveform();
            }
            break;
            case MUSIC:
            {
                // Start or show music player.
                onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GU);
                sys::SysStatus::instance().requestMusicPlayer(sys::START_PLAYER);
            }
            break;
            default:
                break;
        }
    }
}

void Simsu::quit()
{
    qApp->quit();
}

void Simsu::about()
{
    MessageDialog about ( QMessageBox::Icon ( QMessageBox::Information ) , tr ( "About Sudoku" ),
                          tr ( "<center>A basic Sudoku game based on <b>Simsu 1.2.1</b><br/>\
                          <small>Copyright &copy; 2010-2011, onyx-international, Inc</small><br/>\
                          <small>Copyright &copy; 2009 Graeme Gott, author of Simsu 1.2.1</small></center><br/>" ));
    about.exec();
}

void Simsu::checkGame()
{
    m_board->showWrong();
}

void Simsu::newGame()
{
    // Board::newPuzzle ( int seed, int symmetry, int algorithm, bool load )
    //TODO show game configuration dialog
    m_board->newPuzzle();
}

}
}

