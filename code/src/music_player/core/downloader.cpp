#include <stdlib.h>

#include "statehandler.h"
#include "constants.h"
#include "downloader.h"

namespace player
{

//curl callbacks
static size_t curl_write_data(void *data, size_t size, size_t nmemb,
                              void *pointer)
{
    Downloader *dl = (Downloader *)pointer;
    dl->mutex()->lock ();
    size_t buf_start = dl->stream()->buf_fill;
    size_t data_size = size * nmemb;
    dl->stream()->buf_fill += data_size;

    dl->stream()->buf = (char *)realloc (dl->stream()->buf, dl->stream()->buf_fill);
    memcpy(dl->stream()->buf + buf_start, data, data_size);
    dl->mutex()->unlock();
    dl->checkBuffer();
    return data_size;
}

static size_t curl_header(void *data, size_t size, size_t nmemb,
                          void *pointer)
{
    Downloader *dl = (Downloader *)pointer;
    dl->mutex()->lock ();
    size_t data_size = size * nmemb;
    if (data_size < 3)
    {
        dl->mutex()->unlock();
        return data_size;
    }

    //qDebug("header received: %s", (char*) data);
    QString str = QString::fromAscii((char *) data, data_size);
    str = str.trimmed ();
    if (str.left(4).contains("HTTP"))
    {
        qDebug("Downloader: header received");
        //TODO open metadata socket
    }
    else if (str.left(4).contains("ICY"))
    {
        qDebug("Downloader: shoutcast header received");
        //dl->stream()->icy_meta_data = true;
    }
    else
    {
        QString key = str.left(str.indexOf(":")).trimmed().toLower();
        QString value = str.right(str.size() - str.indexOf(":") - 1).trimmed().toLower();
        dl->stream()->header.insert(key, value);
        qDebug("Downloader: key=%s, value=%s",qPrintable(key),qPrintable(value));

        if (key == "icy-metaint")
        {
            dl->stream()->icy_metaint = value.toInt();
            dl->stream()->icy_meta_data = true;
        }
    }
    dl->mutex()->unlock();
    return data_size;
}

int curl_progress(void *pointer, double dltotal, double dlnow, double ultotal, double ulnow)
{
    Q_UNUSED(dltotal);
    Q_UNUSED(dlnow);
    Q_UNUSED(ultotal);
    Q_UNUSED(ulnow);
    Downloader *dl = (Downloader *)pointer;
    dl->mutex()->lock ();
    bool aborted = dl->stream()->aborted;
    dl->mutex()->unlock();
    if (aborted)
    {
        return -1;
    }
    return 0;
}

Downloader::Downloader(QObject *parent, const QString &url)
    : QThread(parent)
{
    url_ = url;
    curl_global_init(CURL_GLOBAL_ALL);
    handle_ = 0;
    stream_.buf_fill = 0;
    stream_.buf = 0;
    stream_.icy_meta_data = false;
    stream_.aborted = true;
    stream_.icy_metaint = 0;
    meta_count_ = 0;
}


Downloader::~Downloader()
{
    abort();
    curl_global_cleanup();
    stream_.aborted = true;
    stream_.buf_fill = 0;
    if (stream_.buf)
    {
        free(stream_.buf);
    }
    stream_.buf = 0;
}

qint64 Downloader::read(char* data, qint64 maxlen)
{
    qint64 len = 0;
    mutex_.lock();
    if (!stream_.icy_meta_data || stream_.icy_metaint == 0)
    {
        len = readBuffer(data, maxlen);
    }
    else
    {
        qint64 nread = 0;
        qint64 to_read;
        while (maxlen > nread && stream_.buf_fill > nread)
        {
            to_read = qMin<qint64>(stream_.icy_metaint - meta_count_, maxlen - nread);
            //to_read = (maxlen - nread);
            qint64 res = readBuffer(data + nread, to_read);
            nread += res;
            meta_count_ += res;
            if (meta_count_ == stream_.icy_metaint)
            {
                meta_count_ = 0;
                mutex_.unlock();
                readICYMetaData();
                mutex_.lock();
            }

        }
        len = nread;

    }
    mutex_.unlock();
    return len;
}

Stream *Downloader::stream()
{
    return &stream_;
}

QMutex *Downloader::mutex()
{
    return &mutex_;
}

QString Downloader::contentType()
{
    QString content;
    if (stream_.header.contains("content-type"))
        content = stream_.header.value("content-type");
    return content;
}

void Downloader::abort()
{
    mutex_.lock();
    if (stream_.aborted)
    {
        mutex_.unlock();
        return;
    }
    stream_.aborted = true;
    mutex_.unlock();
    wait();

    if (handle_)
    {
        curl_easy_cleanup(handle_);
        handle_ = 0;
    }
}

int Downloader::bytesAvailable()
{
    mutex_.lock();
    int b = stream_.buf_fill;
    mutex_.unlock();
    return b;
}

void Downloader::run()
{
    qDebug("Downloader: starting download thread");
    handle_ = curl_easy_init();
    //proxy
    QSettings settings ( PlayerUtils::configFile(), QSettings::IniFormat );
    if (PlayerUtils::useProxy())
    {
        curl_easy_setopt(handle_, CURLOPT_PROXY,
                         strdup((PlayerUtils::proxy().host() + ":" +
                                 QString("%1").arg(PlayerUtils::proxy().port())).
                                toLatin1 ().constData ()));
    }

    if (PlayerUtils::useProxyAuth())
    {
        curl_easy_setopt(handle_, CURLOPT_PROXYUSERPWD,
                         strdup((PlayerUtils::proxy().userName() + ":" +
                                 PlayerUtils::proxy().password()).
                                toLatin1 ().constData ()));
    }

    // Set url to download
    curl_easy_setopt(handle_, CURLOPT_URL, strdup(url_.toAscii().constData()));
    // callback for wrting
    curl_easy_setopt(handle_, CURLOPT_WRITEFUNCTION, curl_write_data);
    // Set destination file
    curl_easy_setopt(handle_, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(handle_, CURLOPT_HEADERDATA, this);
    curl_easy_setopt(handle_, CURLOPT_HEADERFUNCTION, curl_header);
    // Disable SSL
    curl_easy_setopt(handle_, CURLOPT_SSL_VERIFYPEER, false);
    curl_easy_setopt(handle_, CURLOPT_SSL_VERIFYHOST, 0);
    // Enable progress meter
    curl_easy_setopt(handle_, CURLOPT_NOPROGRESS, 0);
    curl_easy_setopt(handle_, CURLOPT_PROGRESSDATA, this);
    curl_easy_setopt(handle_, CURLOPT_PROGRESSFUNCTION, curl_progress);
    // Any kind of authentication
    curl_easy_setopt(handle_, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
    curl_easy_setopt(handle_, CURLOPT_VERBOSE, 1);
    // Auto referrer
    curl_easy_setopt(handle_, CURLOPT_AUTOREFERER, 1);
    // Follow redirections
    curl_easy_setopt(handle_, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(handle_, CURLOPT_FAILONERROR, 1);
    curl_easy_setopt(handle_, CURLOPT_MAXREDIRS, 15);
    // user agent
    curl_easy_setopt(handle_, CURLOPT_USERAGENT, "player/0.1.0");
    curl_easy_setopt(handle_, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);

    struct curl_slist *http200_aliases = curl_slist_append(NULL, "ICY");
    struct curl_slist *http_headers = curl_slist_append(NULL, "Icy-PlayerUtils::MetaData: 1");
    curl_easy_setopt(handle_, CURLOPT_HTTP200ALIASES, http200_aliases);
    curl_easy_setopt(handle_, CURLOPT_HTTPHEADER, http_headers);
    mutex_.lock();
    stream_.buf_fill = 0;
    stream_.buf = 0;
    stream_.aborted = false;
    stream_.header.clear ();
    ready_  = false;
    int return_code;
    qDebug("Downloader: starting libcurl");
    mutex_.unlock();
    return_code = curl_easy_perform(handle_);
    qDebug("curl_easy_perform %d", return_code);
    qDebug("Downloader: thread finished");
}

qint64 Downloader::readBuffer(char* data, qint64 maxlen)
{
    if (stream_.buf_fill > 0 && !stream_.aborted)
    {
        int len = qMin<qint64>(stream_.buf_fill, maxlen);
        memcpy(data, stream_.buf, len);
        stream_.buf_fill -= len;
        memmove(stream_.buf, stream_.buf + len, stream_.buf_fill);
        return len;
    }
    return 0;
}

const QString &Downloader::title() const
{
    return title_;
}

void Downloader::checkBuffer()
{
    if (stream_.buf_fill > BUFFER_SIZE && !ready_)
    {
        ready_  = true;
        qDebug("Downloader: ready");
        emit readyRead();
    }
    else if (!ready_)
    {
        emit bufferingProgress(100 * stream_.buf_fill / BUFFER_SIZE);
        qApp->processEvents();
    }
}

bool Downloader::isReady()
{
    return ready_;
}

void Downloader::readICYMetaData()
{
    quint8 packet_size;
    meta_count_ = 0;
    mutex_.lock();
    readBuffer((char *)&packet_size, sizeof(packet_size));
    if (packet_size != 0)
    {
        int size = packet_size * 16;
        char * packet = new char[size];
        while (stream_.buf_fill < size && isRunning())
        {
            mutex_.unlock();
            qApp->processEvents();
            mutex_.lock();
        }
        readBuffer(packet, size);
        qDebug("Downloader: ICY metadata: %s", packet);
        parseICYMetaData(packet);
        delete [] packet;
    }
    mutex_.unlock();
}

void Downloader::parseICYMetaData(char *data)
{
    QString str(data);
    QStringList list(str.split(";", QString::SkipEmptyParts));
    foreach(QString line, list)
    {
        if (line.contains("StreamTitle="))
        {
            line = line.right(line.size() - line.indexOf("=") - 1).trimmed();
            title_ = line.remove("'");
            if (!title_.isEmpty())
            {
                QMap<PlayerUtils::MetaData, QString> metaData;
                metaData.insert(PlayerUtils::TITLE, title_);
                StateHandler::instance()->dispatch(metaData);
            }
            break;
        }
    }
}

}
