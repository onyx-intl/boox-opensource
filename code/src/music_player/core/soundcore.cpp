#include "decoderfactory.h"
#include "constants.h"
#include "streamreader.h"
#include "effect.h"
#include "statehandler.h"
#include "volumecontrol.h"
#include "fileinfo.h"

#include "soundcore.h"

namespace player
{

SoundCore *SoundCore::instance_ = 0;

SoundCore::SoundCore(QObject *parent)
    : QObject(parent)
    , decoder_(0)
    , output_(0)
    , input_(0)
    , paused_(false)
    , update_(false)
    , block_(false)
    , use_eq_(false)
    , preamp_(0)
    , parent_widget_(0)
    , factory_(0)
    , handler_(new StateHandler(this))
    , volume_control_(0)
{
    instance_ = this;
    for (int i = 1; i < 10; ++i)
    {
        bands_[i] = 0;
    }

    connect(handler_.get(), SIGNAL(elapsedChanged(qint64)), SIGNAL(elapsedChanged(qint64)));
    connect(handler_.get(), SIGNAL(bitrateChanged(int)), SIGNAL(bitrateChanged(int)));
    connect(handler_.get(), SIGNAL(frequencyChanged(quint32)), SIGNAL(frequencyChanged(quint32)));
    connect(handler_.get(), SIGNAL(precisionChanged(int)), SIGNAL(precisionChanged(int)));
    connect(handler_.get(), SIGNAL(channelsChanged(int)), SIGNAL(channelsChanged(int)));
    connect(handler_.get(), SIGNAL(metaDataChanged ()), SIGNAL(metaDataChanged ()));
    connect(handler_.get(), SIGNAL(stateChanged (PlayerUtils::State)), SIGNAL(stateChanged(PlayerUtils::State)));

    volume_control_.reset(VolumeControl::create(this));
    connect(volume_control_.get(), SIGNAL(volumeChanged(int, int)), SIGNAL(volumeChanged(int, int)));
}


SoundCore::~SoundCore()
{
    stop();
}

bool SoundCore::play(const QString &source)
{
    stop();
    source_ = source;
    if (handler_->state() != PlayerUtils::Stopped)
    {
        //clear error state
        handler_->dispatch(PlayerUtils::Stopped);
    }

    //buffering state
    handler_->dispatch(PlayerUtils::Buffering);

    QUrl url;
    if (source_.contains("://"))
    {
        //url
        url = source_;
    }
    else if (QFile::exists(source_))
    {
        url = QUrl::fromLocalFile(source_);
    }
    else
    {
        qDebug("SoundCore: file doesn't exist");
        handler_->dispatch(PlayerUtils::NormalError);
        return false;
    }

    factory_ = Decoder::findByURL(url);
    if (factory_ != 0)
    {
        return decode();
    }

    if (url.scheme() == "file")
    {
        if ((factory_ = Decoder::findByPath(source_)))
        {
            input_.reset(new QFile(source_));
            if (!input_->open(QIODevice::ReadOnly))
            {
                qDebug("SoundCore: cannot open input");
                stop();
                handler_->dispatch(PlayerUtils::NormalError);
                return false;
            }
            return decode();
        }
        else
        {
            qWarning("SoundCore: unsupported fileformat");
            stop();
            handler_->dispatch(PlayerUtils::NormalError);
            return false;
        }
    }
    if (url.scheme() == "http")
    {
        input_.reset(new StreamReader(source_, this));
        connect(input_.get(), SIGNAL(bufferingProgress(int)), SIGNAL(bufferingProgress(int)));
        connect(input_.get(), SIGNAL(readyRead()),SLOT(decode()));
        qobject_cast<StreamReader *>(input_.get())->downloadFile();
        return true;
    }
    qWarning("SoundCore: unsupported fileformat");
    stop();
    handler_->dispatch(PlayerUtils::NormalError);
    return false;
}

void SoundCore::stop()
{
    factory_ = 0;
    source_.clear();
    if (decoder_ /*&& decoder_->isRunning()*/)
    {
        decoder_->mutex()->lock ();
        decoder_->stop();
        decoder_->mutex()->unlock();
        //decoder_->stateHandler()->dispatch(Stopped);
    }
    if (output_)
    {
        output_->mutex()->lock ();
        output_->stop();
        output_->mutex()->unlock();
    }

    // wake up threads
    if (decoder_)
    {
        decoder_->mutex()->lock ();
        decoder_->cond()->wakeAll();
        decoder_->mutex()->unlock();
    }
    if (output_)
    {
        output_->recycler()->mutex()->lock ();
        output_->recycler()->cond()->wakeAll();
        output_->recycler()->mutex()->unlock();
    }
    if (decoder_)
    {
        decoder_->wait();
    }
    if (output_)
    {
        output_->wait();
    }

    //if (input_)
    //{
    //    input_->deleteLater();
    //}

    //update VolumeControl
    volume_control_.reset(VolumeControl::create(this));
    connect(volume_control_.get(), SIGNAL(volumeChanged(int, int)), SIGNAL(volumeChanged(int, int)));
}

void SoundCore::pause()
{
    if (output_)
    {
        output_->mutex()->lock ();
        output_->pause();
        output_->mutex()->unlock();
    }
    else if (decoder_)
    {
        decoder_->mutex()->lock ();
        decoder_->pause();
        decoder_->mutex()->unlock();
    }

    // wake up threads
    if (decoder_)
    {
        decoder_->mutex()->lock ();
        decoder_->cond()->wakeAll();
        decoder_->mutex()->unlock();
    }

    if (output_)
    {
        output_->recycler()->mutex()->lock ();
        output_->recycler()->cond()->wakeAll();
        output_->recycler()->mutex()->unlock();
    }
}

void SoundCore::seek(qint64 pos)
{
    if (output_ && output_->isRunning())
    {
        qDebug("Seek");
        output_->mutex()->lock ();
        output_->seek(pos);
        output_->mutex()->unlock();
        if (decoder_ && decoder_->isRunning())
        {
            decoder_->mutex()->lock ();
            decoder_->seek(pos);
            decoder_->mutex()->unlock();
        }
    }
    else if (decoder_)
    {
        decoder_->mutex()->lock ();
        decoder_->seek(pos);
        decoder_->mutex()->unlock();
    }
}

const QString SoundCore::url()
{
    return source_;
}

qint64 SoundCore::totalTime() const
{
    return  (decoder_) ? decoder_->totalTime() : 0;
}

void SoundCore::setEQ(double bands[10], double preamp)
{
    for (int i = 0; i < 10; ++i)
    {
        bands_[i] = bands[i];
    }
    preamp_ = preamp;
    if (decoder_)
    {
        decoder_->mutex()->lock ();
        decoder_->setEQ(bands_, preamp_);
        decoder_->setEQEnabled(use_eq_);
        decoder_->mutex()->unlock();
    }
}

void SoundCore::setEQEnabled(bool on)
{
    use_eq_ = on;
    if (decoder_)
    {
        decoder_->mutex()->lock ();
        decoder_->setEQ(bands_, preamp_);
        decoder_->setEQEnabled(on);
        decoder_->mutex()->unlock();
    }
}

void SoundCore::setVolume(int L, int R)
{
    volume_control_->setVolume(L, R);
}

int SoundCore::leftVolume()
{
    return volume_control_->left();
}

int SoundCore::rightVolume()
{
    return volume_control_->right();
}

void SoundCore::setSoftwareVolume(bool b)
{
    SoftwareVolume::setEnabled(b);
    if (decoder_)
    {
        decoder_->mutex()->lock();
    }

    volume_control_.reset(VolumeControl::create(this));
    connect(volume_control_.get(), SIGNAL(volumeChanged(int, int)), SIGNAL(volumeChanged(int, int)));
    if (decoder_)
    {
        decoder_->mutex()->unlock();
    }
}

bool SoundCore::softwareVolume()
{
    return SoftwareVolume::instance() != 0;
}

qint64 SoundCore::elapsed()
{
    return  handler_->elapsed();
}

int SoundCore::bitrate()
{
    return  handler_->bitrate();
}

quint32 SoundCore::frequency()
{
    return  handler_->frequency();
}

int SoundCore::precision() //TODO rename
{
    return  handler_->precision();
}

int SoundCore::channels()
{
    return  handler_->channels();
}

PlayerUtils::State SoundCore::state() const
{
    return  handler_->state();
}

QMap<PlayerUtils::MetaData, QString> SoundCore::metaData()
{
    return handler_->metaData();
}

QString SoundCore::metaData(PlayerUtils::MetaData key)
{
    return handler_->metaData(key);
}

bool SoundCore::decode()
{
    if (!factory_)
    {
        if (!input_->open(QIODevice::ReadOnly))
        {
            qDebug("SoundCore:: cannot open input");
            handler_->dispatch(PlayerUtils::NormalError);
            return false;
        }
        if (!(factory_ = Decoder::findByMime(qobject_cast<StreamReader *>(input_.get())->contentType())))
        {
            if (!(factory_ = Decoder::findByContent(input_.get())))
            {
                handler_->dispatch(PlayerUtils::NormalError);
                return false;
            }
        }
    }
    if (!factory_->properties().no_output_)
    {
        output_.reset(0);
        output_.reset(Output::create(this));
        if (!output_)
        {
            qWarning("SoundCore: unable to create output");
            handler_->dispatch(PlayerUtils::FatalError);
            return false;
        }
        if (!output_->initialize())
        {
            qWarning("SoundCore: unable to initialize output");
            output_.reset(0);
            handler_->dispatch(PlayerUtils::FatalError);
            return false;
        }
    }
    decoder_.reset(factory_->create(this, input_.get(), output_.get(), source_));
    if (!decoder_)
    {
        qWarning("SoundCore: unsupported fileformat");
        block_ = false;
        stop();
        handler_->dispatch(PlayerUtils::NormalError);
        return false;
    }
    decoder_->setStateHandler(handler_.get());
    setEQ(bands_, preamp_);
    setEQEnabled(use_eq_);
    qDebug ("ok");
    connect(decoder_.get(), SIGNAL(playbackFinished()), SIGNAL(finished()));
    if (output_)
    {
        output_->setStateHandler(decoder_->stateHandler());
    }

    if (decoder_->initialize())
    {
        if (QFile::exists(source_)) //send metadata for local files
        {
            QList <FileInfo *> list;
            factory_->createPlayList(source_, true, list);
            if (!list.isEmpty())
            {
                handler_->dispatch(list[0]->metaData());
                while (!list.isEmpty())
                {
                    delete list.takeFirst();
                }
            }
        }

        if (output_)
        {
            output_->start();
        }
        decoder_->start();
        return true;
    }
    stop();
    return false;
}

SoundCore* SoundCore::instance()
{
    return instance_;
}

}
