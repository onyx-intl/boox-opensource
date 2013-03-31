#include <model/playlistitem.h>
#include <core/fileinfo.h>

#include "player_application.h"
#include "onyx/ui/screen_rotation_dialog.h"

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
    connect(&sys_status, SIGNAL(taskActivated(const QStringList &)), this, SLOT(onTaskActivated(const QStringList &)));

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
    hide_view_on_waking_up = false;
}


void PlayerApplication::onTaskActivated(const QStringList &list)
{
    qDebug() << "Path activated use activate main window in music player." << list;
    if (list.contains(path_))
    {
        // check if it's the document
        qDebug() << "Path detected, show window.";
        view_.show();
        view_.raise();
        view_.activateWindow();
    }
    else
    {
        qDebug() << "hide window.";
        view_.hide();
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
    QString selected_file;
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
        // check if the path's directories are the same.
        if (!path_.isEmpty())
        {
            QFileInfo src_info(path_);
            QFileInfo dst_info(path_name);
            if (src_info.dir().absolutePath() == dst_info.dir().absolutePath())
            {
                need_reload = false;
                selected_file = path_name;
                view_.stop();
            }
            else
            {
                // reload
                close(path_);
                path_ = path_name;
            }
        }
        else
        {
            // reload
            close(path_);
            path_ = path_name;
        }
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

        model_->sort(PlayListModel::FILENAME);
    }

    view_.activateWindow();

    // load audio files from media db
    model_->readMediaInfos();
    model_->sort(PlayListModel::FILENAME);
    model_->preparePlayState();

    sys::SysStatus::instance().setSystemBusy( false );
    onyx::screen::instance().enableUpdate(true);
    if(!hide_view_on_waking_up)
    {
        view_.show();
    }

    if (!selected_file.isEmpty())
    {
        view_.playFile(selected_file);
    }

    view_.songListView()->repaint();
    if(!hide_view_on_waking_up)
    {
        onyx::screen::instance().flush(&view_, onyx::screen::ScreenProxy::GC,
                false, onyx::screen::ScreenCommand::WAIT_ALL);
        onyx::screen::watcher().addWatcher(&view_);
    }
    view_.enableProgressBar(true);
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
    ui::ScreenRotationDialog dialog;
    dialog.popup();
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

void PlayerApplication::onTaskActivated(const QStringList & list)
{
  qDebug() << "Path activated use activate main window in fb reader." << list;
  if (list.contains(path_))
    {
      // check if it's the document
      qDebug() << "Path detected, show window.";
      view_.show();
      view_.raise();
      view_.activateWindow();
    }
  else
    {
      qDebug() << "hide window.";
      view_.hide();
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

