#ifndef DS_APPLICATION_H_
#define DS_APPLICATION_H_

#include "onyx/base/base.h"
#include "onyx/ui/ui.h"
#include "onyx/base/dbus.h"

#include "ds_view.h"


class DSApplication : public QApplication
{
    Q_OBJECT;

public:
    DSApplication(int &argc, char **argv);
    ~DSApplication(void);


public Q_SLOTS:
    bool start();
    bool stop();
    bool execShellCommand(const QStringList & args);

private:
    DSView main_window_;
    NO_COPY_AND_ASSIGN(DSApplication);
};


class DSApplicationAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT;
    Q_CLASSINFO("D-Bus Interface", "com.onyx.interface.device_server");

public:
    DSApplicationAdaptor(DSApplication *application)
        : QDBusAbstractAdaptor(application)
        , app_(application)
    {
        QDBusConnection::systemBus().registerService("com.onyx.service.device_server");
        QDBusConnection::systemBus().registerObject("/com/onyx/object/device_server", app_);
    }

public Q_SLOTS:
    bool start() { return app_->start(); }
    bool stop() { return app_->stop(); }



private:
    DSApplication *app_;
    NO_COPY_AND_ASSIGN(DSApplicationAdaptor);
};



#endif
