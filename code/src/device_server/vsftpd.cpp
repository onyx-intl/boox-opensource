#include <QProcess>

#include "onyx/data/configuration.h"
#include "onyx/ui/languages.h"
#include "vsftpd.h"

VsftpdServer::VsftpdServer()
{
    options_["listen"]="YES";
    options_["write_enable"]="YES";
    options_["anonymous_enable"]="YES";
    options_["anon_upload_enable"]="YES";
    options_["anon_mkdir_write_enable"]="YES";
    options_["anon_other_write_enable"]="YES";
    options_["no_anon_password"]="YES";
    options_["anon_root"]="/media/flash";
    options_["anon_umask"]="0777";
}

VsftpdServer::~VsftpdServer()
{
}

bool VsftpdServer::start()
{
    updateConf();
    QProcess::startDetached("start_vsftpd.sh");
    return true;
}

bool VsftpdServer::stop()
{
    QProcess::startDetached("stop_vsftpd.sh");
    return true;
}

bool VsftpdServer::isRunning()
{
    return true;
}

bool VsftpdServer::updateConf(const QString & path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadWrite))
    {
        return false;
    }

    QString string;
    QMap<QString, QString>::const_iterator i = options_.constBegin();
    while ( i != options_.constEnd() )
    {
        string = i.key();
        string += "=";
        string += i.value();
        string += "\n";
        file.write(string.toLocal8Bit());
        i++;
    }
    file.close();
    return true;
}

void VsftpdServer::setOption(const QString &key, const QString & value)
{
    options_[key] = value;
}

QString VsftpdServer::option(const QString &key)
{
    return options_[key];
}



