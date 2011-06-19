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
    frame_->show();
    onyx::screen::watcher().addWatcher(frame_.get());
    frame_->setDefaultFocus();
    sys::SysStatus::instance().setSystemBusy(false);
;    return true;
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

}
