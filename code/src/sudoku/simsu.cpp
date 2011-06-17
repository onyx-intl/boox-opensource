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


using namespace ui;

namespace onyx
{
namespace simsu
{

Simsu::Simsu(QWidget *parent , Qt::WindowFlags f) : QWidget(parent, f),
    dialog_(new MDialog(parentWidget())),
    status_bar_(parent,  MENU |CONNECTION | BATTERY | MESSAGE | CLOCK | SCREEN_REFRESH)
{
    connect(&status_bar_, SIGNAL(menuClicked()), this, SLOT(showMenu()));
    setWindowFlags(Qt::FramelessWindowHint);
    QSettings settings;
    // Create board
    QWidget *contents = new QWidget ( this );
    Square *square = new Square(contents );
    n_layout = new QHBoxLayout;
    n_layout->setMargin(0);
    m_board = new Board(square);
    m_board->setAutoSwitch(false);
    square->setChild(m_board);
    m_layout = new QVBoxLayout(contents);
    m_layout->addSpacing(3);
    m_layout->setContentsMargins(2, 2, 2, 2);
    n_layout->addWidget(square,1);
    m_layout->addSpacing(3);
    m_layout->addSpacerItem(new QSpacerItem(5, 5, QSizePolicy::Minimum, QSizePolicy::Minimum));

    m_layout->addLayout(n_layout);
    m_layout->addWidget(&status_bar_);
    m_layout->setDirection(QBoxLayout::TopToBottom);
    setLayout(m_layout);
    m_board->cell(0,0)->setFocus();

    connect(m_board,SIGNAL(toShowBoard()),this, SLOT(showBoard()));
    connect(m_board, SIGNAL(win()), this, SLOT(onWin()));
    connect(dialog_, SIGNAL(ActiveKey(qint32)), m_board, SLOT(setActiveKey(qint32)));
    connect(dialog_, SIGNAL(ActiveModeKey(qint32)), m_board, SLOT(setActiveModeKey(qint32)));
    dialog_->setModal(true);
    showMaximized();

#ifndef BUILD_FOR_FB
    onyx::screen::instance().enableUpdate(true);
    onyx::screen::instance().setDefaultWaveform(onyx::screen::ScreenProxy::GC);
#endif
}

Simsu::~Simsu()
{
    delete m_board;
    delete dialog_;
}

void Simsu::closeEvent(QCloseEvent *event)
{
    QSettings().setValue("Geometry", saveGeometry());
    QWidget::closeEvent(event);
}


bool Simsu::event(QEvent *event)
{
    bool ret = QWidget::event(event);
#ifndef BUILD_FOR_FB
    if(event->type() == QEvent::UpdateRequest /*&& global_update*/)
    {
        QApplication::processEvents();
        onyx::screen::instance().updateWidget(0,onyx::screen::instance().defaultWaveform(),true, onyx::screen::ScreenCommand::WAIT_NONE);
        onyx::screen::instance().setDefaultWaveform(onyx::screen::ScreenProxy::GU);
    }
#endif
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
    switch(event->key())
    {
    case Qt::Key_Escape:
    {
        event->accept();
    }
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
        QWidget::keyPressEvent(event);
        break;
    }
}

void Simsu::keyReleaseEvent (QKeyEvent * event)
{
    qint32 key = event->key();
    if (key == Qt::Key_Escape)
    {
        event->accept();
        //use menudialog to quit
        qApp->quit();
    }

    QWidget::keyReleaseEvent(event);
}

void Simsu::showBoard()
{
    qint32 column = m_board->getColumn();
    qint32 row = m_board->getRow();

    if(!m_board->cell(column,row)->given())
    {

        //TODO set position correctly
        //paitn rect
        dialog_->hide();
        m_board->cell(m_board->getColumn(),m_board->getRow())->setSelected(true);
        qint32 group_c = column/3;
        qint32 group_r = row/3;

        //if column 1, moves to (3,3)
        //esle if (group_r,group_c) = (1,1), moves to (6,3)
        //else moves to  (3,0)
        //
        if (group_c == 0)
        {
            column = 3;
            row = 3;
        } else if (group_r ==1 && group_c == 1)
        {
            column = 3;
            row = 6;
        } else {
            column = 0;
            row = 3;
        }
        QPoint point = m_board->cell(column,row)->pos();
        dialog_->move(point.x(),point.y());
        dialog_->show();
        if(dialog_->exec() == QDialog::Accepted)
        {
            m_board->cell(m_board->getColumn(),m_board->getRow())->updateValue();
        }
        dialog_->hide();
        m_board->cell(m_board->getColumn(),m_board->getRow())->setSelected(false);
#ifndef BUILD_FOR_FB
        onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::INVALID);
#endif
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
    if(menu.popup() != QDialog::Accepted)
    {
        QApplication::processEvents();
        return;
    }

    QAction * group = menu.selectedCategory();
    if(group == sudoku_actions_.category())
    {
        SudokuActionsType index = sudoku_actions_.selected();
        switch(index)
        {
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
    else if(group == system_actions_.category())
    {
        SystemAction system_action = system_actions_.selected();
        switch(system_action)
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
#ifndef BUILD_FOR_FB
                onyx::screen::instance().updateWidget(0, onyx::screen::ScreenProxy::GU);
                onyx::screen::instance().toggleWaveform();
#endif
            }
            break;
        case MUSIC:
            {
                // Start or show music player.
#ifndef BUILD_FOR_FB
                onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GU);
#endif
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
    MessageDialog about(QMessageBox::Icon(QMessageBox::Information) , tr("About Sudoku"),
                        tr("<center>A basic Sudoku game based on <b>Simsu 1.2.1</b><br/>\
                          <small>Copyright &copy; 2010-2011, onyx-international, Inc</small><br/>\
                          <small>Copyright &copy; 2009 Graeme Gott, author of Simsu 1.2.1</small></center><br/>"));
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

void Simsu::onWin()
{
    MessageDialog congr(QMessageBox::Icon(QMessageBox::Information) , tr("You win"),
                        tr("Thanks for playing Sudoku"));
    congr.exec();
}

}
}

