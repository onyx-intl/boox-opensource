#include "downloader.h"
#include "streamreader.h"

namespace player
{

StreamReader::StreamReader(const QString &name, QObject *parent)
    : QIODevice(parent)
{
    downloader_ = new Downloader(this, name);
    connect(downloader_, SIGNAL(readyRead()), SIGNAL(readyRead()));
    connect(downloader_, SIGNAL(bufferingProgress(int)), SIGNAL(bufferingProgress(int)));
}

StreamReader::~StreamReader()
{
    downloader_->abort();
    qDebug("StreamReader::~StreamReader()");
}

bool StreamReader::atEnd () const
{
    return false;
}

qint64 StreamReader::bytesAvailable () const
{
    return downloader_->bytesAvailable ();
}

qint64 StreamReader::bytesToWrite () const
{
    return -1;
}

bool StreamReader::canReadLine () const
{
    return false;
}

void StreamReader::close ()
{
    downloader_->abort();
}

bool StreamReader::isSequential () const
{
    return true;
}

bool StreamReader::open ( OpenMode mode )
{
    if (mode != QIODevice::ReadOnly)
    {
        return false;
    }

    //downloadFile();
    setOpenMode(QIODevice::ReadOnly);
    if (downloader_->isReady())
    {
        return true;
    }
    return false;
}

bool StreamReader::reset ()
{
    QIODevice::reset();
    return true;
}

bool StreamReader::seek ( qint64 pos )
{
    QIODevice::seek(pos);
    return false;
}

qint64 StreamReader::size () const
{
    return bytesAvailable ();
}

bool StreamReader::waitForBytesWritten ( int msecs )
{
    //usleep(msecs*1000);
    return true;
}

bool StreamReader::waitForReadyRead ( int msecs )
{
    //usleep(msecs*1000);
    return true;
}

qint64 StreamReader::readData(char* data, qint64 maxlen)
{
    return downloader_->read (data, maxlen);
}

qint64 StreamReader::writeData(const char*, qint64)
{
    return 0;
}

void StreamReader::downloadFile()
{
    downloader_->start();
}

const QString &StreamReader::contentType()
{
    downloader_->mutex()->lock ();
    content_type_ = downloader_->contentType();
    downloader_->mutex()->unlock();
    qApp->processEvents();
    qDebug("StreamReader: content type: %s", qPrintable(content_type_));
    return content_type_;
}

}
