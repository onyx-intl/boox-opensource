#ifndef DS_VSFTPD_H_
#define DS_VSFTPD_H_

#include <QMap>
#include <QString>

#include "onyx/base/base.h"
#include "onyx/ui/ui.h"
#include "onyx/base/dbus.h"

class VsftpdServer : public QObject
{
    Q_OBJECT;

public:
    VsftpdServer();
    ~VsftpdServer();

public Q_SLOTS:
    bool start();
    bool stop();
    bool isRunning();
    void setOption(const QString &key, const QString & value);
    QString option(const QString &key);

private:
    bool updateConf(const QString & path = "/etc/vsftpd.conf");

private:
    QMap<QString, QString> options_;
};

#endif
