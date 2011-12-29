#include "main_widget.h"
#include <QVBoxLayout>
#include <QStatusBar>
#include <QKeyEvent>
#include <qdebug.h>
#include <QApplication>

#include "onyx/screen/screen_update_watcher.h"
#include "onyx/ui/screen_rotation_dialog.h"
#include "onyx/sys/sys.h"

MainWidget::MainWidget(QWidget *parent)
    :QWidget(parent)
{
    setWindowTitle(QCoreApplication::tr("Gomoku"));
    setWindowFlags(Qt::FramelessWindowHint);
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);
    gomoku = new GomokuWidget(this);
    status_bar_ = new StatusBar(this, MENU |CONNECTION | BATTERY | MESSAGE | CLOCK | SCREEN_REFRESH);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->addWidget(gomoku);
    layout->addWidget(status_bar_);
    setLayout(layout);
    onyx::screen::watcher().addWatcher(this);
    status_bar_->setFocusPolicy(Qt::NoFocus);
    connect(status_bar_, SIGNAL(menuClicked()), this, SLOT(showMenu()));

    SysStatus & sys_status = SysStatus::instance();
    connect( &sys_status, SIGNAL( mouseLongPress(QPoint, QSize) ),
            gomoku, SLOT( onMouseLongPress(QPoint, QSize) ) );
    connect( gomoku, SIGNAL( popupMenu() ), this, SLOT( showMenu() ) );
}

void MainWidget::keyPressEvent(QKeyEvent *ke)
{
    ke->accept();
}

void MainWidget::keyReleaseEvent(QKeyEvent *ke)
{
    switch (ke->key())
    {
    case Qt::Key_Escape:
        ke->accept();
        close();
        break;

    case Qt::Key_Menu:
        ke->accept();
        showMenu();
        break;

    default:
        qApp->sendEvent(gomoku, ke);
        break;
    }
}

void MainWidget::showMenu()
{
    PopupMenu menu(this);
    gomoku_actions_.generateActions();
    menu.addGroup(&gomoku_actions_);
    std::vector<int> all;
    all.push_back(ROTATE_SCREEN);
    all.push_back(MUSIC);
    all.push_back(RETURN_TO_LIBRARY);
    system_actions_.generateActions(all);
    menu.setSystemAction(&system_actions_);
    if(menu.popup() != QDialog::Accepted)
    {
        QApplication::processEvents();
        repaint();
        onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GC);
        return;
    }

    QAction * group = menu.selectedCategory();
    if(group == gomoku_actions_.category())
    {
        GomokuActionsType index = gomoku_actions_.selected();
        switch(index)
        {
        case NEW:
            gomoku->newGame();
            update();
            break;
        case ABOUT:
            about();
            repaint();
            onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU, onyx::screen::ScreenCommand::WAIT_ALL);
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
                ScreenRotationDialog dialog(this);
                dialog.popup();
                repaint();
                onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU, onyx::screen::ScreenCommand::WAIT_ALL);
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
    repaint();
    onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GC, onyx::screen::ScreenCommand::WAIT_ALL);
}

void MainWidget::about()
{
    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf-8"));
    MessageDialog about(QMessageBox::Icon(QMessageBox::Information) , tr("About Gomoku"),
                        tr("<center>A basic gomoku game<br/>\
                          <small>Copyright &copy; 2011, Łukasz Świątkowski</small><br/>\
                          <small>www.lukesw.net</small></center><br/>"));
    about.exec();
}
