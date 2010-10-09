#ifndef PLAYER_DOWNLOADER_H_
#define PLAYER_DOWNLOADER_H_

#include "onyx/base/base.h"
#include "onyx/base/down_cast.h"
#include "onyx/base/dbus.h"
#include "onyx/ui/ui.h"

#include <curl/curl.h>

namespace player
{

struct Stream
{
    char*   buf;
    int     buf_fill;
    QString content_type;
    bool    aborted;
    QMap<QString, QString> header;
    bool    icy_meta_data;
    int     icy_metaint;
};

class Downloader : public QThread
{
    Q_OBJECT
public:
    Downloader(QObject *parent, const QString &url);
    ~Downloader();

    qint64  read(char* data, qint64 maxlen);
    Stream* stream();
    QMutex* mutex();
    QString contentType();
    void    abort();
    int     bytesAvailable();

    const QString& title() const;
    void checkBuffer();
    bool isReady();

Q_SIGNALS:
    void readyRead();
    void bufferingProgress(int);

protected:
    void run();

private:
    qint64 readBuffer(char* data, qint64 maxlen);
    void readICYMetaData();
    void parseICYMetaData(char *data);

private:
    CURL*   handle_;
    QMutex  mutex_;
    Stream  stream_;
    QString url_;
    int     meta_count_;
    QString title_;
    bool    ready_;
};

};

#endif
