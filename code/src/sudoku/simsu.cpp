#include "simsu.h"
#include "board.h"
#include "pattern.h"
#include "square.h"
#include "cell.h"
#include "mdialog.h"

#include <QButtonGroup>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QShortcut>
#include <QSpinBox>
#include <QStackedWidget>
#include <QUndoStack>

#include <QVBoxLayout>
#include <QWheelEvent>
#include <QKeyEvent>
#include <onyx/ui/base_thumbnail.h>
#include "onyx/ui/ui.h"
#include <onyx/sys/sys.h>

static bool global_update = true;

static void setDefaultWaveform(onyx::screen::ScreenProxy::Waveform w)
{
    onyx::screen::instance().setDefaultWaveform(w);
}

using namespace ui;

namespace onyx {
namespace simsu {


/*****************************************************************************/

namespace
{
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
            setDefaultWaveform(onyx::screen::ScreenProxy::DW);
            OnyxPushButton::keyPressEvent(e);
        }
};

SidebarButton::SidebarButton (const QString &text, QWidget *parent )
    : OnyxPushButton ( text, parent ) {
    setIconSize ( QSize ( 16, 16 ) );
    setIcon ( QIcon () );
    //setToolButtonStyle ( Qt::ToolButtonTextOnly );
    setCheckable ( true );
    setFont(QFont("Serif",20, QFont::Bold));
    resize(40,40);
    setSizePolicy ( QSizePolicy::Minimum, QSizePolicy::Minimum );
}

}

/*****************************************************************************/


Simsu::Simsu ( QWidget *parent , Qt::WindowFlags f ) : QWidget ( parent, f ) {
    int screenWidth = QApplication::desktop()->screenGeometry().width();
    int screenHeight = QApplication::desktop()->screenGeometry().height();
    setStyleSheet("MDialog {border: 3px solid black; color: white;}");
    setWindowFlags(Qt::FramelessWindowHint);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    QSettings settings;
    togglePopMode(false);
//     QWidget *contents = new QWidget ( this );
    // Create board
    Square *square = new Square ( this );
    m_board = new Board ( square );
    square->setChild ( m_board );
    connect ( m_board, SIGNAL ( activeKeyChanged ( int ) ), this, SLOT ( activeKeyChanged ( int ) ) );

    new_button = new SidebarButton (tr ( "New" ), this );
    new_button->setCheckable ( 0 );
    connect ( new_button, SIGNAL ( pressed()) , this, SLOT ( newGame() ) );
    OnyxPushButton *undo_button = new SidebarButton (tr ( "Undo" ), this );
    undo_button->setCheckable ( 0 );
    connect ( undo_button, SIGNAL ( clicked ( bool ) ), m_board->moves(), SLOT ( undo() ) );
    OnyxPushButton *redo_button = new SidebarButton (tr ( "Redo" ),  this);
    redo_button->setCheckable ( 0 );
    connect ( redo_button, SIGNAL ( clicked ( bool ) ), m_board->moves(), SLOT ( redo() ) );
    OnyxPushButton *check_button = new SidebarButton (tr ( "Check" ), this );
    check_button->setCheckable ( 0 );
    connect ( check_button, SIGNAL ( clicked ( bool ) ), m_board, SLOT ( showWrong ( bool ) ) );
    OnyxPushButton *about_button = new SidebarButton (tr ( "About" ), this );
    about_button->setCheckable ( 0 );
    connect ( about_button, SIGNAL ( clicked ( bool ) ), this, SLOT ( about() ) );
    OnyxPushButton *quit_button = new SidebarButton (tr ( "Quit" ), this );
    quit_button->setCheckable ( 0 );
    connect ( quit_button, SIGNAL ( clicked ( bool ) ), qApp, SLOT ( quit() ) );

    ///m_act_layout;
    m_act_layout = new QGridLayout;
    m_act_buttons = new QButtonGroup ( this );
    m_keys_layout = new QGridLayout;
    m_key_buttons = new QButtonGroup ( this );
    m_act_layout->setMargin(0);
    m_act_buttons->addButton ( new_button, 1 );
    m_act_layout->addWidget ( new_button, 0, 0);
    m_act_list_buttons.append ( new_button );
    m_act_buttons->addButton ( undo_button, 2 );
    m_act_layout->addWidget ( undo_button, 0, 1);
    m_act_list_buttons.append ( undo_button );

    m_act_buttons->addButton ( redo_button, 3 );
    m_act_layout->addWidget ( redo_button, 0, 2);
    m_act_list_buttons.append ( redo_button );
    m_act_buttons->addButton ( check_button, 4 );
    m_act_layout->addWidget ( check_button, 1, 0);
    m_act_list_buttons.append ( check_button );

    m_act_buttons->addButton ( about_button, 5 );
    m_act_layout->addWidget ( about_button, 1, 1);
    m_act_list_buttons.append ( about_button );
    m_act_buttons->addButton ( quit_button, 6 );
    m_act_layout->addWidget ( quit_button, 1, 2);
    m_act_list_buttons.append ( quit_button );
    m_keys_layout->setMargin ( 0 );

    connect ( m_key_buttons, SIGNAL ( buttonClicked ( int ) ), m_board, SLOT ( setActiveKey ( int ) ) );

    key_button = new SidebarButton (QString::number(1), this );
    m_key_buttons->addButton ( key_button, 1 );
    m_keys_layout->addWidget ( key_button, 0, 0);
    m_keys_list_buttons.append (key_button);
    for ( int i = 2; i < 10; ++i ) {
        OnyxPushButton *key = new SidebarButton (QString::number(i), this );
        key->resize(40,40);
        m_keys_list_buttons.append (key);
        m_key_buttons->addButton ( key, i );
        m_keys_layout->addWidget ( key, (i - 1) / 3, (i - 1) % 3 );
    }

    m_key_buttons->button ( qBound ( 1, QSettings().value ( "Key", 1 ).toInt(), 10 ) )->click();

    mode_button = new SidebarButton (tr ( "Mode" ), this );
    connect ( mode_button, SIGNAL ( clicked(bool)), this, SLOT ( toggleMode(bool)));
    highlight_button = new SidebarButton (tr ( "Highlight" ), this );
    connect ( highlight_button, SIGNAL ( clicked ( bool ) ), m_board, SLOT (setHighlightActive (bool)));
    if ( settings.value ( "Highlight" ).toBool() ) {
        highlight_button->click();
    }
    dialog_button = new SidebarButton (tr ( "PopUp" ), this );
    connect ( dialog_button, SIGNAL ( clicked(bool)), this, SLOT ( togglePopMode(bool)));
    m_mode_layout = new QVBoxLayout;
    m_mode_layout->addWidget(mode_button);
    m_mode_layout->addWidget(highlight_button);
    m_mode_layout->addWidget(dialog_button);
    // Layout window
    m_layout = new QVBoxLayout ( this );
    m_layout->addWidget ( square, 1 );
    m_layout->addSpacing ( 6 );
    m_hlayout = new QHBoxLayout;
    m_hlayout->addSpacerItem(new  QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Minimum));
    m_hlayout->addLayout(m_mode_layout);
    m_hlayout->addSpacerItem(new  QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Minimum));
    m_hlayout->addLayout ( m_keys_layout );
    m_hlayout->addSpacerItem(new  QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Minimum));
    m_hlayout->addLayout(m_act_layout);
    m_hlayout->addSpacerItem(new  QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Minimum));
    m_layout->addLayout(m_hlayout);

    setLayout(m_layout);
    int degree = sys::SysStatus::instance().screenTransformation();

    if (degree == 90 || degree == 270) {
        toggleWidescreen(true);
    } else {
        toggleWidescreen(false);
    }
#ifndef BUILD_FOR_ARM
    toggleWidescreen(1);
#endif
    showMaximized();
    m_board->setAutoSwitch(false);
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
    int id = m_key_buttons->checkedId();

    if ( event->delta() < 0 ) {
        id++;

        if ( id > 9 ) {
            id = 1;
        }

    } else {
        id--;

        if ( id < 1 ) {
            id = 9;
        }
    }

    m_key_buttons->button ( id )->click();
    QWidget::wheelEvent ( event );
}

/*****************************************************************************/

void Simsu::newGame() {
///simply call a random game
    m_board->newPuzzle ( qrand(), 9, int(qrand() / RAND_MAX)) ;
    m_board->moveFocus(0,0,1,0);
}

/*****************************************************************************/

void Simsu::showDetails() {
    QSettings settings;
    QString symmetry = Pattern::name ( settings.value ( "Current/Symmetry" ).toInt() );
    QString icon = Pattern::icon ( settings.value ( "Current/Symmetry" ).toInt() );
    QString algorithm = settings.value ( "Current/Algorithm", 0 ).toInt() ? tr ( "Slice and Dice" ) : tr ( "Dancing Links" );
    int seed = settings.value ( "Current/Seed" ).toInt();
    MessageDialog details ( QMessageBox::NoIcon, tr ( "Details" ), tr ( "<p><b>Symmetry:</b> %1<br><b>Algorithm:</b> %L2<br><b>Seed:</b> %L3</p>" ).arg ( symmetry ).arg ( algorithm ).arg ( seed ), QMessageBox::Ok, this );
    details.exec();
}

/*****************************************************************************/

void Simsu::about() {
    MessageDialog about ( QMessageBox::Icon ( QMessageBox::Information ) , tr ( "About Simsu" ),
                          tr ( "<center><big><b>Simsu %1</b></big><br/>A basic Sudoku game<br/>\
                   <small>Copyright &copy; 2009 Graeme Gott</small></center>" ).arg ( qApp->applicationVersion() ) );
    about.exec();
}

/*****************************************************************************/

void Simsu::activeKeyChanged ( int key ) {
    m_key_buttons->button ( key )->setChecked ( true );
}

/*****************************************************************************/

void Simsu::notesModeChanged ( bool mode ) {
//      m_mode_buttons->button(mode)->setChecked(true);
}

/*****************************************************************************/

void Simsu::toggleMode(bool mode) {
    m_board->setMode(mode);
}

/*****************************************************************************/


void Simsu::toggleWidescreen ( bool checked ) {
    QSettings().setValue ( "Widescreen", checked );
    int width = 0;

    if ( checked ) {
        m_hlayout->setDirection( QBoxLayout::TopToBottom);
        m_hlayout->addSpacerItem(new  QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Minimum));
        m_layout->setDirection ( QBoxLayout::LeftToRight );
    } else {
        m_hlayout->setDirection ( QBoxLayout::LeftToRight );
        m_layout->setDirection ( QBoxLayout::TopToBottom );
    }

    int int_min1 = qMax(m_key_buttons->button(1)->width(), m_key_buttons->button(1)->height());
    int int_min2 = qMax(m_act_buttons->button(1)->width(), m_act_buttons->button(1)->height());
    int int_min = qMax(int_min1, int_min2);
    for ( int i = 1; i < 10; ++i ) {
        m_key_buttons->button ( i )->resize (int_min, int_min);
    }
    for ( int i = 1; i < 6; ++i ) {
        m_act_buttons->button ( i )->resize (int_min, int_min);
    }
}


/*****************************************************************************/
bool Simsu::event ( QEvent *event )
{
    bool ret = QWidget::event ( event );
    if (event->type() == QEvent::UpdateRequest /*&& global_update*/)
    {
       onyx::screen::instance().updateWidget(0);
       setDefaultWaveform(onyx::screen::ScreenProxy::GC);
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
        qApp->quit();
        break;

    case Qt::Key_PageUp:
        {
            //Go to mode_button if m_board is focused;
            if (QApplication::focusWidget() == m_board->focusWidget())
            {
                new_button->setFocus();
            }
            else if (QApplication::focusWidget() == mode_button)
            {
                if ( m_board->getColumn()>0)
                {
                    m_board->moveFocus(m_board->getColumn()-1,m_board->getRow(),1,0);
                }
                else if (m_board->getRow()>0)
                {
                    m_board->moveFocus(m_board->getColumn(),m_board->getRow()-1,0,1);
                }
                else
                {
                    m_board->moveFocus(m_board->getColumn()+1,m_board->getRow(),-1,0);
                }
            }
            else if (QApplication::focusWidget() == highlight_button)
            {
                mode_button->setFocus();
            }
            else if (QApplication::focusWidget() == dialog_button)
            {
                highlight_button->setFocus();
            }
            else
            {
                if (m_act_list_buttons.indexOf(static_cast<OnyxPushButton*>(QApplication::focusWidget())) != -1)
                {
                    m_keys_list_buttons.at(0)->setFocus();
                }
                if (m_keys_list_buttons.indexOf(static_cast<OnyxPushButton*>(QApplication::focusWidget())) != -1)
                {
                    dialog_button->setFocus();
                }
            }
        }
        setDefaultWaveform(onyx::screen::ScreenProxy::GU);
        break;
    case Qt::Key_PageDown:
        {
            if (QApplication::focusWidget() == m_board->focusWidget())
            {
                mode_button->setFocus();
            }
            else if (QApplication::focusWidget() == mode_button)
            {
                highlight_button->setFocus();
            }
            else if (QApplication::focusWidget() == highlight_button)
            {
                dialog_button->setFocus();
            }
            else if (QApplication::focusWidget() == dialog_button)
            {
                m_keys_list_buttons.at(0)->setFocus();
            }
            else
            {
                if (m_act_list_buttons.indexOf(static_cast<OnyxPushButton*>(QApplication::focusWidget())) != -1)
                {
                    if ( m_board->getColumn()>0)
                    {
                        qDebug()<<m_board->getColumn();
                        m_board->moveFocus(m_board->getColumn()-1,m_board->getRow(),1,0);
                    }
                    else if (m_board->getRow()>0)
                    {
                        m_board->moveFocus(m_board->getColumn(),m_board->getRow()-1,0,1);
                    }
                    else
                    {
                        m_board->moveFocus(m_board->getColumn()+1,m_board->getRow(),-1,0);
                    }
                }
                if (m_keys_list_buttons.indexOf(static_cast<OnyxPushButton*>(QApplication::focusWidget())) != -1)
                {
                    new_button->setFocus();
                }
            }
        }
        setDefaultWaveform(onyx::screen::ScreenProxy::GU);
        break;
    case Qt::Key_Up:
        {
            int index = m_act_list_buttons.indexOf(static_cast<OnyxPushButton*>(QApplication::focusWidget()));
            if (index != -1)
            {
                m_act_list_buttons.at((index+3)%6)->setFocus();
            }
            index = m_keys_list_buttons.indexOf(static_cast<OnyxPushButton*>(QApplication::focusWidget()));
            if (index != -1)
            {
                if (1<(index+6)%9<10)
                {
                    m_keys_list_buttons.at((index+6)%9)->setFocus();
                }
                else
                {
                    m_keys_list_buttons.at(0)->setFocus();
                }
            }
        }
        setDefaultWaveform(onyx::screen::ScreenProxy::DW);
        break;

    case Qt::Key_Down:
        {
            int index = m_act_list_buttons.indexOf(static_cast<OnyxPushButton*>(QApplication::focusWidget()));
            if (index != -1)
            {
                m_act_list_buttons.at((index+3)%6)->setFocus();
            }
            index = m_keys_list_buttons.indexOf(static_cast<OnyxPushButton*>(QApplication::focusWidget()));
            if (index != -1)
            {
                if (1<(index+3)%9<10)
                {
                    m_keys_list_buttons.at((index+12)%9)->setFocus();
                }
                else
                {
                    m_keys_list_buttons.at(0)->setFocus();
                }
            }
        }
        setDefaultWaveform(onyx::screen::ScreenProxy::DW);
        break;

    case Qt::Key_Left:
        {
            int index = m_act_list_buttons.indexOf(static_cast<OnyxPushButton*>(QApplication::focusWidget()));
            if (index != -1)
            {
                m_act_list_buttons.at((index+2)%6)->setFocus();
            }
            index = m_keys_list_buttons.indexOf(static_cast<OnyxPushButton*>(QApplication::focusWidget()));
            if (index != -1)
            {
                if (1<(index+8)%9<10)
                {
                    m_keys_list_buttons.at((index+8)%9)->setFocus();
                }
                else
                {
                    m_keys_list_buttons.at(0)->setFocus();
                }
            }
        }
        setDefaultWaveform(onyx::screen::ScreenProxy::DW);
        break;

    case Qt::Key_Right:
        {
            int index = m_act_list_buttons.indexOf(static_cast<OnyxPushButton*>(QApplication::focusWidget()));
            if (index != -1)
            {
                m_act_list_buttons.at((index+4)%6)->setFocus();
            }
            index = m_keys_list_buttons.indexOf(static_cast<OnyxPushButton*>(QApplication::focusWidget()));
            if (index != -1)
            {
                if (1<(index+1)%9<10)
                {
                    m_keys_list_buttons.at((index+1)%9)->setFocus();
                }
                else
                {
                    m_keys_list_buttons.at(0)->setFocus();
                }
            }
        }
        setDefaultWaveform(onyx::screen::ScreenProxy::DW);
        break;

    case Qt::Key_Menu:
        {
            //set focus to m_board from everywhere
            if ( m_board->getColumn()>0)
            {
                m_board->moveFocus(m_board->getColumn()-1,m_board->getRow(),1,0);
            }
            else if (m_board->getRow()>0)
            {
                m_board->moveFocus(m_board->getColumn(),m_board->getRow()-1,0,1);
            }
            else
            {
                m_board->moveFocus(m_board->getColumn()+1,m_board->getRow(),-1,0);
            }
        }
        setDefaultWaveform(onyx::screen::ScreenProxy::GU);
        break;

    case Qt::Key_Return:
        {
            if (getPopMode())
            {
                if (QApplication::focusWidget() == m_board->focusWidget())
                {
                    popUpdialog();
                }
            }
            else
            {
                int column = m_board->getColumn();
                int row = m_board->getRow();
                if (!m_board->cell(column,row)->given())
                {
                    m_board->cell(m_board->getColumn(),m_board->getRow())->updateValue();
                }
                setDefaultWaveform(onyx::screen::ScreenProxy::GU);
            }
            QWidget::keyPressEvent(event);
        }
        break;

    default:
        QWidget::keyPressEvent(event);
        break;
    }
}

/*****************************************************************************/
void Simsu::togglePopMode(bool pop) {
    pop_=pop;
}

/*****************************************************************************/
void Simsu::popUpdialog(){
    int column = m_board->getColumn();
    int row = m_board->getRow();
    QPoint point = m_board->pos();
    if (!m_board->cell(column,row)->given()){
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
        if (dialog->exec() == QDialog::Accepted){
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
}
}

