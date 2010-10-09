
#include "tts.h"
//#include "espeak/tts_espeak.h"
//#include "aisound/tts_aisound.h"
#include "ej/tts_ej.h"

namespace tts
{
static const int BPS = 16;
static const int SAMPLE_RATE = 44100;
static const int CHANNELS = 1;


TTS::TTS(const QLocale & locale)
: span_(3000)
{
    tts_impl_.reset(new EJSound);
    tts_impl_->initialize(locale, sound());

    connect(tts_impl_.get(), SIGNAL(synthDone(bool, QByteArray &)),
            this, SLOT(onSynthDone(bool, QByteArray &)));
    connect(&AsyncPlayer::instance(), SIGNAL(playFinished(int)), this, SLOT(onPlayFinished(int)));
    connect(&timer_, SIGNAL(timeout()), this, SLOT(onTimeout()));

    setState(TTS_STOPPED);
}

TTS::~TTS()
{
}

/// Synthesis the text to speech. The data will be ready later.
bool TTS::speak(const QString & text)
{
    setState(TTS_PLAYING);
    if (tts_impl_)
    {
        return tts_impl_->synthText(text);
    }
    return false;
}

bool TTS::pause()
{
    setState(TTS_PAUSED);
    return true;
}

bool TTS::resume()
{
    setState(TTS_PLAYING);
    return true;
}

bool TTS::toggle()
{
    if (state() == TTS_PLAYING)
    {
        setState(TTS_PAUSED);
    }
    else if (state() == TTS_PAUSED)
    {
        setState(TTS_PLAYING);
    }
    return true;
}

bool TTS::stop()
{
    setState(TTS_STOPPED);
    data_.clear();
    return true;
}

void TTS::changeVolume(int v, bool m)
{
    sound().setVolume(v);
}

/// Segment of sentence has been finished. But it does not mean that
/// the whole input message has been finished.
void TTS::onSynthDone(bool ok, QByteArray &data)
{
    qDebug("TTS::onSynthDone data length %d", data.length());
    QApplication::processEvents();

    // If stopped, just ignore the chunk.
    if (state() == TTS_STOPPED)
    {
        data_.clear();
        return;
    }

    data_.append(data);

    // Send the data to async player. Before that, it's necessary to
    // check the state.
    if (state() == TTS_PLAYING)
    {
        onTimeout();
        // timer_.stop();
        // timer_.start(span_);
    }
}

void TTS::onTimeout()
{
    /* Dump to file.
    QFile file("dump.tts");
    if (file.open(QIODevice::Append))
    {
        file.write(data_);
        file.flush();
        file.close();
    }
    */
    qDebug("Send to play.");
    AsyncPlayer::instance().play(sound(), data_);
    data_.clear();
}

void TTS::setState(TTS_State state)
{
    state_ = state;
    sound().enable(state == TTS_PLAYING);

    if (state == TTS_STOPPED)
    {
        tts_impl_->stop();
    }

    if (state == TTS_PAUSED)
    {
        tts_impl_->pause();
    }

    if (state == TTS_STOPPED || state == TTS_PAUSED)
    {
        AsyncPlayer::instance().waitForDone();
        sound_.reset(0);
    }
}

void TTS::onPlayFinished(int)
{
    if (isPlaying())
    {
        emit speakDone();
    }
}

/// Check if we have any tts engine installed.
/// Return false if there is no tts backend enabled. Otherwise returns true.
bool TTS::support()
{
    return true;
}

bool TTS::speakers(QStringList & list)
{
    if (tts_impl_)
    {
        return tts_impl_->speakers(list);
    }
    return false;
}

bool TTS::currentSpeaker(QString & speaker)
{
    if (tts_impl_)
    {
        return tts_impl_->currentSpeaker(speaker);
    }
    return false;
}

bool TTS::setSpeaker(const QString & speaker)
{
    if (tts_impl_)
    {
        return tts_impl_->setSpeaker(speaker);
    }
    return false;
}

bool TTS::speeds(QVector<int> & list)
{
    if (tts_impl_)
    {
        return tts_impl_->speeds(list);
    }
    return false;
}

bool TTS::currentSpeed(int & speed)
{
    if (tts_impl_)
    {
        return tts_impl_->currentSpeed(speed);
    }
    return false;
}


bool TTS::setSpeed(int speed)
{
    if (tts_impl_)
    {
        return tts_impl_->setSpeed(speed);
    }
    return false;
}

bool TTS::styles(QVector<int> & styles)
{
    if (tts_impl_)
    {
        return tts_impl_->styles(styles);
    }
    return false;
}

bool TTS::currentStyle(int & style)
{
    if (tts_impl_)
    {
        return tts_impl_->currentStyle(style);
    }
    return false;
}

bool TTS::setStyle(int style)
{
    if (tts_impl_)
    {
        return tts_impl_->setStyle(style);
    }
    return false;
}

Sound & TTS::sound()
{
    if (!sound_)
    {
        sound_.reset(new Sound);
        sound_->setBitsPerSample(BPS);
        sound_->setChannels(CHANNELS);
        sound_->setSamplingRate(SAMPLE_RATE);
    }
    return *sound_;
}

}
