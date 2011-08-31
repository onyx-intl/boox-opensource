#include "constants.h"
#include "buffer.h"
#include "output.h"
#include "volumecontrol.h"

#include <stdio.h>

// TODO. Remove the reference
#include <output/outputwaveoutfactory.h>
#include <output/outputasynplayerfactory.h>
#include <output/outputalsafactory.h>

namespace player
{

Output::Output(QObject* parent)
    : QThread(parent)
    , recycler_(stackSize())
{
    handler_               = 0;
    frequency_             = 0;
    channels_              = 0;
    kbps_                  = 0;
    total_written_         = 0;
    current_milliseconds_  = -1;
    bytes_per_millisecond_ = 0;
    user_stop_             = false;
    pause_                 = false;
    finish_                = false;
    is_seeking_            = false;
}

Output::~Output()
{
}

void Output::configure(quint32 freq, int chan, int prec)
{
    frequency_ = freq;
    channels_  = chan;
    precision_ = prec;
    bytes_per_millisecond_ = freq * chan * (prec / 8) / 1000;
}

void Output::pause()
{
    pause_ = !pause_;
    PlayerUtils::State state = pause_ ? PlayerUtils::Paused : PlayerUtils::Playing;
    dispatch(state);
}

void Output::stop()
{
    user_stop_ = true;
}

void Output::finish()
{
    finish_ = true;
}

qint64 Output::written()
{
    return total_written_;
}

void Output::seek(qint64 pos)
{
    total_written_ = pos * bytes_per_millisecond_;
    current_milliseconds_ = -1;
    is_seeking_ = true;
    enable(false);
}

Recycler *Output::recycler()
{
    return &recycler_;
}

QMutex *Output::mutex()
{
    return &mutex_;
}

void Output::setStateHandler(StateHandler *handler)
{
    handler_ = handler;
}

quint32 Output::sampleRate()
{
    return frequency_;
}

int Output::numChannels()
{
    return channels_;
}

int Output::sampleSize()
{
    return precision_;
}

void Output::dispatch(qint64 elapsed,
                      qint64 totalTime,
                      int bitrate,
                      int frequency,
                      int precision,
                      int channels)
{
    if (handler_)
    {
        handler_->dispatch(elapsed,
                           totalTime,
                           bitrate,
                           frequency,
                           precision,
                           channels);
    }
}

void Output::dispatch(const PlayerUtils::State &state)
{
    if (handler_)
    {
        handler_->dispatch(state);
    }
}

void Output::run()
{
    mutex()->lock ();
    if (!bytes_per_millisecond_)
    {
        qWarning("Output: invalid audio parameters");
        mutex()->unlock ();
        return;
    }
    mutex()->unlock ();

    bool done = false;
    Buffer *b = 0;
    qint64 l, m = 0;

    dispatch(PlayerUtils::Playing);

    while (!done)
    {
        mutex()->lock();
        recycler()->mutex()->lock();
        done = user_stop_;

        while (!done && (recycler()->empty() || pause_))
        {
            mutex()->unlock();
            recycler()->cond()->wakeOne();
            recycler()->cond()->wait(recycler()->mutex());
            mutex()->lock ();
            done = user_stop_;
        }
        status();
        if (!b)
        {
            b = recycler()->next();
            if (b && b->rate)
            {
                kbps_ = b->rate;
            }
        }
        recycler()->cond()->wakeOne();
        recycler()->mutex()->unlock();
        mutex()->unlock();
        if (b)
        {
            changeVolume(b->data, b->nbytes, channels_);
            l = 0;
            m = 0;

            if (is_seeking_)
            {
                enable(b->seeking_finished);
                is_seeking_ = !b->seeking_finished;
            }

            while (l < b->nbytes)
            {
                m = writeAudio(b->data + l, b->nbytes - l);
                if(m >= 0)
                {
                    total_written_ += m;
                    l+= m;
                }
                else
                {
                    break;
                }
            }
            if(m < 0)
            {
                break;
            }
        }
        mutex()->lock();
        //force buffer change
        recycler()->mutex()->lock ();
        recycler()->done();
        recycler()->mutex()->unlock();
        b = 0;
        mutex()->unlock();
    }
    mutex()->lock ();
    //write remaining data
    if(finish_)
    {
        flush();
        qDebug("Output: total written %lld", total_written_);
    }
    dispatch(PlayerUtils::Stopped);
    mutex()->unlock();
}

void Output::status()
{
    qint64 ct = total_written_ / bytes_per_millisecond_ - latency();
    if (ct < 0)
    {
        ct = 0;
    }

    if (ct > current_milliseconds_)
    {
        current_milliseconds_ = ct;
        dispatch(current_milliseconds_,
                 total_written_,
                 kbps_,
                 frequency_,
                 precision_,
                 channels_);
    }
}

void Output::changeVolume(uchar *data, qint64 size, int chan)
{
    if (!SoftwareVolume::instance())
    {
        return;
    }
    if (chan > 1)
    {
        for (qint64 i = 0; i < size/2; i+=2)
        {
            ((short*)data)[i]*= SoftwareVolume::instance()->left()/100.0;
            ((short*)data)[i+1]*= SoftwareVolume::instance()->right()/100.0;
        }
    }
    else
    {
        int l = qMax(SoftwareVolume::instance()->left(), SoftwareVolume::instance()->right());
        for (qint64 i = 0; i < size/2; i++)
        {
            ((short*)data)[i]*= l/100.0;
        }
    }
}

// static methods

QList<OutputFactory*> *Output::factories_ = 0;
QStringList Output::files_;
QTimer *Output::timer_ = 0;

void Output::checkFactories()
{
    // TODO. Do NOT use the plugin now.
    // Create the wave output factory directly
    if (!factories_)
    {
        files_.clear();
        factories_ = new QList<OutputFactory *>;

#ifndef WIN32
#ifdef BUILD_WITH_TFT
        OutputFactory *factory = new OutputAlsaFactory();
#else
        OutputFactory *factory = new OutputAsynPlayerFactory();
#endif
#else
        OutputFactory *factory = new OutputWaveOutFactory();
#endif

        if (factory)
        {
            factories_->append(factory);
            qApp->installTranslator(factory->createTranslator(qApp));
        }
    }
}

void Output::registerFactory ( OutputFactory *fact )
{
    factories_->append( fact );
}

Output *Output::create (QObject *parent)
{
    Output *output = 0;
    checkFactories();
    if (factories_->isEmpty ())
    {
        qDebug("Output: unable to find output plugins");
        return output;
    }
    OutputFactory *fact = Output::currentFactory();
    if (!fact && !factories_->isEmpty())
    {
        fact = factories_->at(0);
    }
    if (fact)
    {
        output = fact->create(parent);
    }
    return output;
}

QList<OutputFactory*> *Output::outputFactories()
{
    checkFactories();
    return factories_;
}

QStringList Output::outputFiles()
{
    checkFactories();
    return files_;
}

void Output::setCurrentFactory(OutputFactory* factory)
{
    checkFactories();
    if (!factories_->contains(factory))
    {
        return;
    }

    // TODO. Set Current Factory
}

OutputFactory *Output::currentFactory()
{
    checkFactories();

    // TODO. Implement Me
    return factories_->at(0);
}

}
