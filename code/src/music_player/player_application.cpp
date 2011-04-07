#include <model/playlistitem.h>
#include <core/fileinfo.h>

#include "player_application.h"

namespace player
{

PlayerApplication::PlayerApplication(int &argc, char **argv)
    : QApplication(argc, argv)
    , view_(0)
    , current_time_(0)
    , hide_view_on_waking_up(false)
{
    setApplicationName(QCoreApplication::tr("Music Player"));
    setQuitOnLastWindowClosed(true);

    connect(&view_, SIGNAL(stateChanged(int)), SIGNAL(stateChanged(int)));
    connect(&view_, SIGNAL(testReload()), this, SLOT(onWakeUp()));

    SysStatus & sys_status = SysStatus::instance();
    connect( &sys_status, SIGNAL( mountTreeSignal( bool, const QString & ) ),
             this, SLOT( onMountTreeSignal( bool, const QString &) ) );
    connect( &sys_status, SIGNAL( sdCardChangedSignal( bool ) ), this, SLOT( onSDChangedSignal( bool ) ) );
    connect(&sys_status, SIGNAL(aboutToSuspend()), this, SLOT(onAboutToSuspend()));
    connect(&sys_status, SIGNAL(wakeup()), this, SLOT(onWakeUp()));

#ifdef Q_WS_QWS
    connect(qApp->desktop(), SIGNAL(resized(int)), this, SLOT(onScreenSizeChanged(int)), Qt::QueuedConnection);
#endif

    QString path;
    if (argc > 1)
    {
        path = QString::fromLocal8Bit(argv[1]);
    }
    open(path);
}

PlayerApplication::~PlayerApplication(void)
{
    close(path_);
}

void PlayerApplication::onTestReload()
{
    open(path_);
}

void PlayerApplication::onAboutToSuspend()
{
    hide_view_on_waking_up = view_.isHidden();
}

void PlayerApplication::onWakeUp()
{
    PlayListItem* current_item = model_->currentItem();
    QString path = current_item->fileInfo()->path();
    current_time_ = view_.elapsed();

    if (!path.isEmpty() && open(path))
    {
        QTimer::singleShot(300, this, SLOT(onResetTime()));
    }
}

void PlayerApplication::onResetTime()
{
    view_.setProgress(current_time_);
}

bool PlayerApplication::open(const QString &path_name)
{
    ui::loadTranslator(QLocale::system().name());
    bool need_reload = true;
    if (path_name.isEmpty() && path_.isEmpty())
    {
        path_ = PlayerUtils::lastSong();
    }
    else if (path_name.isEmpty() && !path_.isEmpty())
    {
        need_reload = false;
    }
    else //if (!path_name.isEmpty() && !path_.isEmpty())
    {
        // reload
        close(path_);
        path_ = path_name;
    }

    if (need_reload)
    {
        model_.reset(new PlayListModel);
        view_.attachModel(model_.get());

        if (path_.contains("://"))
        {
            model_->addFile(path_);
        }
        else if (QFile::exists(path_))
        {
            model_->addDirectory(path_);
        }
    }
    else
    {
        view_.activateWindow();
    }

    // if it is the first time rendering, set busy to be false
    //if (hide_view_on_waking_up)
    //{
    //    // reset the flag hide_view_on_waking_up for activating player view in normal cases.
    //    hide_view_on_waking_up = false;
    //}
    //else
    {
        view_.show();
        onyx::screen::watcher().addWatcher(&view_);
        sys::SysStatus::instance().setSystemBusy( false );
        onyx::screen::instance().enableUpdate(true);
        onyx::screen::instance().flush(&view_, onyx::screen::ScreenProxy::GC);
        qDebug("Flush player view");
        view_.enableProgressBar(true);
    }
    return true;
}

bool PlayerApplication::close(const QString &path_name)
{
    // stop the current playing and clear the model
    view_.stop();
    view_.deattachModel();
    return false;
}

void PlayerApplication::onRotateScreen()
{
    SysStatus::instance().rotateScreen();
}

void PlayerApplication::onScreenSizeChanged(int)
{
    view_.resize(qApp->desktop()->screenGeometry().size());
    if (view_.isActiveWindow())
    {
        onyx::screen::instance().flush(&view_, onyx::screen::ScreenProxy::GC);
    }
}

void PlayerApplication::onMountTreeSignal(bool inserted, const QString &mount_point)
{
    qDebug( "Mount point:%s %s",
            qPrintable( mount_point ),
            inserted ? "inserted" : "disconnect" );
    if (!inserted && path_.startsWith(mount_point))
    {
        emit stateChanged(STOP_PLAYER);
        QProcess::startDetached("unload_sound_module.sh");
        qApp->exit();
    }
}

void PlayerApplication::onSDChangedSignal(bool inserted)
{
    qDebug("SD %s", inserted ? "inserted" : "disconnect");
    if ( path_.startsWith( SDMMC_ROOT ) && !inserted )
    {
        emit stateChanged(STOP_PLAYER);
        qApp->exit();
    }
}

int PlayerApplicationAdaptor::state()
{
    PlayerUtils::State state = app_->view()->state();
    switch (state)
    {
    case PlayerUtils::Playing:
        return MUSIC_PLAYING;
    case PlayerUtils::Stopped:
        return MUSIC_STOPPED;
    case PlayerUtils::Paused:
        return MUSIC_PAUSED;
    default:
        break;
    }
    return MUSIC_QUIT;
}

void PlayerApplicationAdaptor::show(bool is_show)
{
    if (is_show)
    {
        if (app_->view()->isHidden())
        {
            app_->view()->show();
        }
    }
    else
    {
        if (!app_->view()->isHidden())
        {
            app_->view()->hide();
        }
    }
}

void PlayerApplicationAdaptor::onStateChanged(int curr)
{
    SysStatus::instance().reportMusicPlayerState(curr);
}

}

