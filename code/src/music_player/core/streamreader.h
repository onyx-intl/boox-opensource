#ifndef PLAYER_STREAMREADER_H_
#define PLAYER_STREAMREADER_H_

#include <utils/player_utils.h>

namespace player
{

class Downloader;

class StreamReader : public QIODevice
{
    Q_OBJECT
public:
    StreamReader(const QString &name, QObject *parent = 0);
    ~StreamReader();

    bool   atEnd () const;
    qint64 bytesAvailable () const;
    qint64 bytesToWrite () const;
    bool   canReadLine () const;
    void   close ();
    bool   isSequential () const;
    bool   open ( OpenMode mode );
    bool   reset ();
    bool   seek ( qint64 pos );
    qint64 size () const;
    bool   waitForBytesWritten ( int msecs );
    bool   waitForReadyRead ( int msecs );

    const QString &contentType();
    void downloadFile();

Q_SIGNALS:
    void readyRead();
    void bufferingProgress(int);

protected:
    qint64 readData(char*, qint64);
    qint64 writeData(const char*, qint64);

private:
    void fillBuffer();

private:
    QUrl        url_;
    QString     content_type_;
    Downloader* downloader_;
};

};

#endif
