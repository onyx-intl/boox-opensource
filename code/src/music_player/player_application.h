#ifndef PLAYER_APPLICATION_H_
#define PLAYER_APPLICATION_H_

#include <utils/player_utils.h>
#include "onyx_player_view.h"

using namespace ui;

namespace player
{

/// Naboo Application.
class PlayerApplication : public QApplication
{
    Q_OBJECT
public:
    PlayerApplication(int &argc, char **argv);
    ~PlayerApplication(void);

    OnyxPlayerView* view() { return &view_; }

Q_SIGNALS:
    void stateChanged(int curr);

public Q_SLOTS:
    bool open(const QString &path_name);
    bool close(const QString &path_name);

    void onRotateScreen();
    void onScreenSizeChanged(int);
    void onMountTreeSignal(bool inserted, const QString &mount_point);
    void onSDChangedSignal(bool inserted);
    void onAboutToSuspend();
    void onWakeUp();

private Q_SLOTS:
    void onTestReload();
    void onResetTime();

private:
    OnyxPlayerView              view_;
    scoped_ptr<PlayListModel>   model_;
    QString                     path_;          // path of the document
    qint64                      current_time_;
    bool                        hide_view_on_waking_up;

    NO_COPY_AND_ASSIGN(PlayerApplication);
};


class PlayerApplicationAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT;
    Q_CLASSINFO("D-Bus Interface", "com.onyx.interface.music");

public:
    PlayerApplicationAdaptor(PlayerApplication *application)
        : QDBusAbstractAdaptor(application)
        , app_(application)
    {
        QDBusConnection::systemBus().registerService("com.onyx.service.music");
        QDBusConnection::systemBus().registerObject("/com/onyx/object/music", app_);

        connect(app_, SIGNAL(stateChanged(int)), this, SLOT(onStateChanged(int)));
    }

public Q_SLOTS:
    /// Must be in dbus data type, otherwise these methods will not
    /// be exported as dbus methods. So you can not use std::string
    /// here.
    bool open(const QString & path) { return app_->open(path); }
    bool close(const QString & path) { return app_->close(path); }
    int  state();
    void show(bool is_show);

    void onStateChanged(int);

private:
    PlayerApplication *app_;
    NO_COPY_AND_ASSIGN(PlayerApplicationAdaptor);

};  // NabooApplicationAdaptor

};

#endif
