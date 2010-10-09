#ifndef DICT_APPLICATION_H_
#define DICT_APPLICATION_H_

#include "dictionary/dict_frame.h"

using namespace ui;

namespace dict_tool
{

/// Naboo Application.
class DictApplication : public QApplication
{
    Q_OBJECT
public:
    DictApplication(int &argc, char **argv);
    ~DictApplication(void);

public Q_SLOTS:
    bool open();
    bool close();
    bool isOpened();
    bool errorFound();
    bool suspend();

    void onWakeUp();
    void onUSBSignal(bool inserted);
    void onSDChangedSignal(bool inserted);
    void onMountTreeSignal(bool inserted, const QString &mount_point);
    void onConnectToPCSignal(bool connected);
    void onBatterySignal(const int, const int, bool);
    void onSystemIdleSignal();
    void onAboutToShutDown();

    void onRotateScreen();
    void onScreenSizeChanged(int);

private:
    tts::TTS                tts_engine_;
    DictionaryManager       dict_mgr_;
    scoped_ptr<DictFrame>   frame_;
    NO_COPY_AND_ASSIGN(DictApplication);
};


class DictApplicationAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT;

    Q_CLASSINFO("D-Bus Interface", "com.onyx.interface.dict_tool");

public:
    DictApplicationAdaptor(DictApplication *application)
        : QDBusAbstractAdaptor(application)
        , app_(application)
    {
        QDBusConnection::systemBus().registerService("com.onyx.service.dict_tool");
        QDBusConnection::systemBus().registerObject("/com/onyx/object/dict_tool", app_);
    }

public Q_SLOTS:
    /// Must be in dbus data type, otherwise these methods will not
    /// be exported as dbus methods. So you can not use std::string
    /// here.
    bool open() { return app_->open(); }
    bool close() { return app_->close(); }
    bool suspend() { return app_->suspend(); }

private:
    DictApplication *app_;
    NO_COPY_AND_ASSIGN(DictApplicationAdaptor);

};  // DictApplicationAdaptor

};

#endif
