#ifndef DJVU_APPLICATION_H_
#define DJVU_APPLICATION_H_

#include "djvu_utils.h"
#include "djvu_model.h"
#include "djvu_view.h"

using namespace ui;
using namespace vbf;

namespace djvu_reader
{

/// Djvu Application.
class DjvuApplication : public QApplication
{
    Q_OBJECT
public:
    DjvuApplication(int &argc, char **argv);
    ~DjvuApplication(void);

    const QString & currentPath() { return current_path_; }

public Q_SLOTS:
    bool open(const QString &path_name);
    bool close(const QString &path_name);
    bool isOpened();
    bool errorFound();
    bool onSuspend();

    void onWakeUp();
    void onUSBSignal(bool inserted);
    void onSDChangedSignal(bool inserted);
    void onMountTreeSignal(bool inserted, const QString &mount_point);
    void onConnectToPCSignal(bool connected);
    void onBatterySignal(const int, const int, bool);
    void onSystemIdleSignal();
    void onAboutToShutDown();

    void onCreateView(int type, MainWindow* main_window, QWidget*& result);
    void onAttachView(int type, QWidget* view, MainWindow* main_window);
    void onDeattachView(int type, QWidget* view, MainWindow* main_window);

    void onRotateScreen();
    void onScreenSizeChanged(int);

    bool flip(int);

    void onTaskActivated(const QStringList & list);
    void onReceivedTaskCloseRequest(const QStringList &);


private:
    MainWindow main_window_;
    DjvuModel  model_;            // Djvu model instance
    QString    current_path_;     // path of the current document

    NO_COPY_AND_ASSIGN(DjvuApplication);
};


class DjvuApplicationAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT;

    Q_CLASSINFO("D-Bus Interface", "com.onyx.interface.djvu_reader");

public:
    DjvuApplicationAdaptor(DjvuApplication *application)
        : QDBusAbstractAdaptor(application)
        , app_(application)
    {
        QDBusConnection::systemBus().registerService("com.onyx.service.djvu_reader");
        QDBusConnection::systemBus().registerObject("/com/onyx/object/djvu_reader", app_);
    }

public Q_SLOTS:
    /// Must be in dbus data type, otherwise these methods will not
    /// be exported as dbus methods. So you can not use std::string
    /// here.
    bool open(const QString & path) { return app_->open(path); }
    bool close(const QString & path) { return app_->close(path); }

    bool flip(int);

private:
    DjvuApplication *app_;
    NO_COPY_AND_ASSIGN(DjvuApplicationAdaptor);

};  // DjvuApplicationAdaptor

};

#endif
