#include "onyx/ui/languages.h"
#include "dict_application.h"
#include "onyx/screen/screen_update_watcher.h"

namespace dict_tool
{

DictApplication::DictApplication(int &argc, char **argv)
    : QApplication(argc, argv)
    , tts_engine_(QLocale::system())
    , dict_mgr_()
{
    ui::loadTranslator(QLocale::system().name());
    frame_.reset(new OnyxDictFrame(0, dict_mgr_, &tts_engine_));
    connect(frame_.get(), SIGNAL(closeClicked()), this, SLOT(close()));
}

DictApplication::~DictApplication(void)
{
    close();
}

bool DictApplication::open()
{
    connect( &sys::SysStatus::instance(), SIGNAL( taskActivated(const QStringList &)), this, SLOT(onTaskActivated(const QStringList &)));
    connect( &sys::SysStatus::instance(), SIGNAL(taskCloseRequest(const QStringList &)), this, SLOT(onReceivedTaskCloseRequest(const QStringList &)));


    // set system busy to false before showing windows.
#ifdef BUILD_WITH_TFT
    sys::SysStatus::instance().setSystemBusy(false);
#endif

    sys::SysStatus::instance().setSystemBusy(false);

    frame_->show();
    frame_->setDefaultFocus();
    onyx::screen::watcher().addWatcher(frame_.get());

    processEvents();
    onyx::screen::watcher().enqueue(0, onyx::screen::ScreenProxy::GU);

    return true;
}

bool DictApplication::close()
{
    quit();
    return true;
}

bool DictApplication::isOpened()
{
    return true;
}

bool DictApplication::errorFound()
{
    return false;
}

bool DictApplication::suspend()
{
    return false;
}

void DictApplication::onWakeUp()
{
}

void DictApplication::onUSBSignal(bool inserted)
{
}

void DictApplication::onSDChangedSignal(bool inserted)
{
}

void DictApplication::onMountTreeSignal(bool inserted, const QString &mount_point)
{
}

void DictApplication::onConnectToPCSignal(bool connected)
{
    qDebug("\n\nPC %s\n\n", connected ? "connected" : "disconnected");
    if ( connected )
    {
        qApp->exit();
    }
}

void DictApplication::onBatterySignal(const int, const int, bool)
{
}

void DictApplication::onSystemIdleSignal()
{
}

void DictApplication::onAboutToShutDown()
{
}

void DictApplication::onRotateScreen()
{
}

void DictApplication::onScreenSizeChanged(int)
{
}

static const QString appName = "dict_tool";
void DictApplication::onTaskActivated(const QStringList & list)
{
    if (list.contains(appName))
    {
        frame_->show();
        frame_->raise();
        frame_->activateWindow();
    }
    else
    {
        frame_->lower();
    }
}

void DictApplication::onReceivedTaskCloseRequest(const QStringList &)
{
    if (list.contains(appName))
    {
        qApp->exit();
    }
}


}
