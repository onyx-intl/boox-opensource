#include "menudialog.h"
#include "QKeyEvent"
#include "QGridLayout"
#include "QButtonGroup"
#include <QMessageBox>
#include <QSettings>
#include <QDebug>

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

MenuDialog::MenuDialog(QWidget* parent): QDialog(parent),current_button_(0) {
    onyx::screen::instance().enableUpdate ( true );
    onyx::screen::instance().setDefaultWaveform(onyx::screen::ScreenProxy::GC);
    setWindowModality(Qt::ApplicationModal);
    setWindowFlags(Qt::FramelessWindowHint);
    setFocus();
    QGridLayout *layout = new QGridLayout(this);
    SidebarButton *new_button = new SidebarButton (tr ( "New" ), this );
    new_button->setCheckable ( 0 );
    connect ( new_button, SIGNAL ( pressed()) , this, SLOT ( newGame() ) );
    SidebarButton *check_button = new SidebarButton (tr ( "Check" ), this );
    check_button->setCheckable ( 0 );
    connect ( check_button, SIGNAL ( clicked ( bool ) ), this, SLOT ( toCheck ( bool ) ) );
    SidebarButton *about_button = new SidebarButton (tr ( "About" ), this );
    about_button->setCheckable ( 0 );
    connect ( about_button, SIGNAL ( clicked ( bool ) ), this, SLOT ( about() ) );
    SidebarButton *quit_button = new SidebarButton (tr ( "Quit" ), this );
    quit_button->setCheckable ( 0 );
    connect ( quit_button, SIGNAL ( clicked ( bool ) ), qApp, SLOT ( quit() ) );
    act_buttons_ = new QButtonGroup ( this );
    act_buttons_->addButton(new_button,0);
    act_buttons_->addButton(check_button,1);
    act_buttons_->addButton(about_button,2);
    act_buttons_->addButton(quit_button,3);
    list_act_buttons.append(new_button);
    list_act_buttons.append(check_button);
    list_act_buttons.append(about_button);
    list_act_buttons.append(quit_button);
    layout->addWidget(new_button);
    layout->addWidget(check_button);
    layout->addWidget(about_button);
    layout->addWidget(quit_button);
    setLayout(layout);
}

void MenuDialog::keyPressEvent(QKeyEvent* event) {
    current_button_ =  list_act_buttons.indexOf(static_cast<SidebarButton*> (focusWidget()));
    switch (event->key()){
        case Qt::Key_Escape:
            close();
            break;
        case Qt::Key_Up:
            if (current_button_ > 0) --current_button_;
            break;
        case Qt::Key_Down:
            if (current_button_ < 3) ++current_button_;
            break;
        case Qt::Key_Left:
            if (current_button_ > 0) --current_button_;
            break;
        case Qt::Key_Right:
            if (current_button_ < 3) ++current_button_;
            break;
        default:
            break;
    }
    list_act_buttons.at(current_button_)->setFocus();
    QWidget::keyPressEvent ( event );
}

void MenuDialog::mouseMoveEvent(QMouseEvent* event) {
    QDialog::mouseMoveEvent(event);
}


void MenuDialog::newGame() {
    //TODO may reuse original one
    ///simply call a random game
//     m_board->newPuzzle ( qrand(), 9, int(qrand() / RAND_MAX)) ;
//     m_board->moveFocus(0,0,1,0);
}

/*****************************************************************************/
//TODO
// void MenuDialog::showDetails() {
//     QSettings settings;
//     QString symmetry = Pattern::name ( settings.value ( "Current/Symmetry" ).toInt() );
//     QString icon = Pattern::icon ( settings.value ( "Current/Symmetry" ).toInt() );
//     QString algorithm = settings.value ( "Current/Algorithm", 0 ).toInt() ? tr ( "Slice and Dice" ) : tr ( "Dancing Links" );
//     int seed = settings.value ( "Current/Seed" ).toInt();
//     MessageDialog details ( QMessageBox::NoIcon, tr ( "Details" ), tr ( "<p><b>Symmetry:</b> %1<br><b>Algorithm:</b> %L2<br><b>Seed:</b> %L3</p>" ).arg ( symmetry ).arg ( algorithm ).arg ( seed ), QMessageBox::Ok, this );
//     details.exec();
// }

/*****************************************************************************/

void MenuDialog::about() {
    MessageDialog about ( QMessageBox::Icon ( QMessageBox::Information ) , tr ( "About Simsu" ),
                          tr ( "<center><big><b>Simsu %1</b></big><br/>A basic Sudoku game<br/>\
                          <small>Copyright &copy; 2009 Graeme Gott</small></center>" ).arg ( qApp->applicationVersion() ) );
    about.exec();
}

void MenuDialog::toCheck(bool)
{
    emit toCheck();
}

bool MenuDialog::event(QEvent* e) {
    bool ret = QDialog::event ( e );
    //TODO just the buttons
    if (e->type() == QEvent::UpdateRequest)
    {
        onyx::screen::instance().updateWidget(focusWidget());

    }
    return ret;
}
