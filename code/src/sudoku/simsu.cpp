#include <QButtonGroup>
#include <QGridLayout>
#include <QKeyEvent>
#include <QSettings>
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
    status_bar_(parent,  MENU |CONNECTION | BATTERY | MESSAGE | CLOCK | SCREEN_REFRESH) {
    connect(&status_bar_, SIGNAL(menuClicked()), this, SLOT(showMenu()));
    setWindowFlags(Qt::FramelessWindowHint);
    QSettings settings;
    // Create board
//     Square *square = new Square ( this );
    m_board = new Board ( this );
    m_board->setAutoSwitch(false);
//     square->setChild ( m_board );
    m_layout = new QGridLayout ( this );
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addWidget(m_board);
    m_layout->addWidget(&status_bar_);
    setLayout(m_layout);
    m_board->setAutoSwitch(false);
    showMaximized();
    m_board->cell(0,0)->setFocus();
    connect(m_board,SIGNAL(toShowBoard()),this, SLOT(showBoard()));
    onyx::screen::instance().enableUpdate ( true );
    onyx::screen::instance().setDefaultWaveform ( onyx::screen::ScreenProxy::GC );
}

Simsu::~Simsu()
{
}

void Simsu::closeEvent ( QCloseEvent *event ) {
    QSettings().setValue ( "Geometry", saveGeometry() );
    QWidget::closeEvent ( event );
}


bool Simsu::event ( QEvent *event )
{
    bool ret = QWidget::event ( event );
    if (event->type() == QEvent::UpdateRequest /*&& global_update*/)
    {
        QApplication::processEvents();
        onyx::screen::instance().updateWidget(0,onyx::screen::instance().defaultWaveform(),true, onyx::screen::ScreenCommand::WAIT_NONE);
        onyx::screen::instance().setDefaultWaveform(onyx::screen::ScreenProxy::GU);
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
    QApplication::processEvents();
    //TODO encapsulate each part as functions
    switch (event->key())
    {
    case Qt::Key_Escape:
        //use menudialog to quit
        qApp->quit();
        break;
    case Qt::Key_PageUp:
        m_board->moves()->undo();
        break;
    case Qt::Key_PageDown:
        m_board->moves()->redo();
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
    //    QWidget::keyPressEvent(event);
        break;
    }
}

void Simsu::showBoard()
{
    int column = m_board->getColumn();
    int row = m_board->getRow();

    if (!m_board->cell(column,row)->given())
    {
        MDialog *dialog = new MDialog(parentWidget());
        //TODO set position correctly
        //paitn rect
        m_board->cell(m_board->getColumn(),m_board->getRow())->setSelected( true );
        int res_c = column%3;
        int group_c = column/3;
        int res_r = row%3;
        int group_r = row/3;

        switch (group_c)
        {
        case 0:
            column++;
            break;
        case 1:
            column -= res_c;
            break;
        case 2:
        {
            column -= 4;
            column -= res_c;
            if (res_c == 2 ) {
                column++;
            } else if (res_c == 0 ) {
                column--;
            }
        }
        break;
        default:
            break;
        }
        switch (group_r) {
        case 0:
            row++;
            break;
        case 1:
        {
            if (group_c == 1) {
                if (res_r) {
                    row -=4;
                } else {
                    row++;
                }
            } else {
                row -= res_r;
                row--;
            }
        }
        break;
        case 2:
            row -= 4;
            break;
        default:
            break;
        }

        QPoint point = m_board->cell(column,row)->pos();
        dialog->move(point.x(),point.y());

        connect(dialog, SIGNAL(ActiveKey(int)), m_board, SLOT(setActiveKey(int)));
        connect(dialog, SIGNAL(ActiveModeKey(int)), m_board, SLOT(setActiveModeKey(int)));
        if (dialog->exec() == QDialog::Accepted) {
            m_board->cell(m_board->getColumn(),m_board->getRow())->updateValue();
        }
        delete dialog;
        m_board->cell(m_board->getColumn(),m_board->getRow())->setSelected( false );
        onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::INVALID);
    }
    return;
}

//TODO standard onyx UI menu
void Simsu::showMenu()
{
    PopupMenu menu(parentWidget());
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
        SudokuActionsType index = sudoku_actions_.selected();
        switch (index) {
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
    } else if (group == system_actions_.category()) {
        SystemAction system_action = system_actions_.selected();
        switch (system_action)
        {
        case RETURN_TO_LIBRARY:
        {
            qApp->quit();
        }
        break;
        case ROTATE_SCREEN:
        {
            sys::SysStatus::instance().rotateScreen();
            update();
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

