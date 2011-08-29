#include <core/constants.h>
#include <core/buffer.h>
#include "outputalsa.h"

namespace player
{

AlsaVolumeControl::AlsaVolumeControl(QObject *parent)
{
}

AlsaVolumeControl::~AlsaVolumeControl()
{
}

void AlsaVolumeControl::setVolume(int left, int right)
{
    // TODO. Update the volume by two channels
}

void AlsaVolumeControl::volume(int *left, int *right)
{
    // TODO.
}

OutputAlsa::OutputAlsa(QObject * parent)
: Output(parent)
{
}

OutputAlsa::~OutputAlsa()
{
    uninitialize();
}

void OutputAlsa::configure(quint32 samplerate, int channels, int bitspersample)
{
#ifdef BUILD_WITH_TFT
    snd_pcm_format_t format;
    int rc;
    if (bitspersample == 8)
    {
        format = SND_PCM_FORMAT_S8;
    }
    else if (bitspersample == 16)
    {
        format = SND_PCM_FORMAT_S16;
    }
    else
    {
        qDebug("Invalid bps");
        return;
    }

    if (snd_pcm_set_params (pcm_handle_, format, SND_PCM_ACCESS_RW_INTERLEAVED,
                            channels, samplerate, 0, 500*1000) < 0)
    {

        qDebug("Error setting PCM params!");
        return;
    }
    byte_per_frames_ = channels * bitspersample / 8;
#endif
}

bool OutputAlsa::initialize()
{
#ifdef BUILD_WITH_TFT
    if (snd_pcm_open(&pcm_handle_, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0)
    {
        fprintf(stderr, "Could not initial ALSA sound system.\n");
        return false;
    }
#endif
    return true;
}

qint64 OutputAlsa::latency()
{
    // TODO. Implement Me
    return 0;
}

void OutputAlsa::enable(bool e)
{
    // TODO. Implement Me
}

qint64 OutputAlsa::writeAudio(unsigned char *data, qint64 len)
{
#ifdef BUILD_WITH_TFT
    static const unsigned int MAX_VOLUME_FOR_AK98 = 0xa186;

    bool ret;
    unsigned long frames = len / byte_per_frames_;
    while (frames > 0)
    {
        int rc = snd_pcm_writei (pcm_handle_, data, frames);
        if (rc == -EAGAIN)
        {
            continue;
        }

        if (rc < 0)
        {
            if (snd_pcm_recover(pcm_handle_, rc, 0) < 0)
            {
                qDebug("Cannot recover from an error in playing audio!");
                return -1;
            }
            break;
        }

        data += rc * byte_per_frames_;
        frames -= rc;
    }
#endif
    return len;
}

void OutputAlsa::flush()
{
    // TODO. Implement Me
}

void OutputAsynPlayer::uninitialize()
{
#ifdef BUILD_WITH_TFT
    snd_pcm_drain(pcm_handle_);
    snd_pcm_close(pcm_handle_);
    pcm_handle_  = 0;
#endif
}

}
