#include <stdlib.h>
#include <stdio.h>

#ifndef _WINDOWS
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/soundcard.h>
#endif

#include "sound.h"
#include <QProcess>

/// Before using sound driver, make sure the sound module has been loaded.
/// modprobe snd-soc-imx-3stack-wm8711
static void loadKernelModule()
{
#ifndef _WINDOWS
    QProcess loader;
    QStringList module;
    QString mod = qgetenv("SOUND_MODULE");
    if (mod.isEmpty())
    {
      mod = "snd-soc-imx-3stack-wm8711";
    }
    module << mod;
    loader.start("modprobe", module);
    const int TIME = 2000;
    if (loader.waitForStarted(TIME))
    {
        loader.waitForFinished(TIME);
    }

    // Have to ensure the module has been loaded.
    loader.start("lsmod");
    usleep(1000 * 200);
#endif
}


Sound::Sound(const char *dev)
: device_(-1)
, enable_(true)
, bps_(8)
, channels_(0)
{
    open(dev);
}

Sound::~Sound()
{
    close();
}

bool Sound::open(const char *device_name)
{
#ifndef _WINDOWS
    device_ = ::open(device_name, O_WRONLY);
    if (device_ == -1)
    {
        printf("Can't open audio playback device, try to load kernel module.\n");
        // Use our own sound driver.
        loadKernelModule();
    }

    // Open again.
    if (device_ < 0)
    {
        device_ = ::open(device_name, O_WRONLY);
        if (device_ == -1)
        {
            printf("Still can't open audio playback device %s!\n", device_name);
            return false;
        }
    }
#endif
    return true;
}

void Sound::close()
{
#ifndef _WINDOWS
    ::close(device_);
#endif
    device_ = -1;
}

/// Convert mono to stereo according to the bps.
QByteArray Sound::toStereo(const char *src, int size, const int bps)
{
    QByteArray data;
    const char * p = src;
    int len = bps / 8;
    data.reserve(size * len);
    size = size / len;
    for(int i = 0; i < size; ++i)
    {
        data.append(p, len);
        data.append(p, len);
        p += len;
    }
    return data;
}

int Sound::volume()
{
    int volume = 0;

#ifndef _WINDOWS
    int ret = -1;
    if (ret = ioctl(device_, SOUND_MIXER_READ_VOLUME, &volume) ==  - 1)
    {
        printf("SOUND_MIXER_READ_VOLUME ioctl failed.\n");
        return 0;
    }
    volume = volume & 0xff;
#endif
    return volume;
}

/// Change the volume. Make sure the volume is in [0, 100]
bool Sound::setVolume(int volume)
{
    if (volume < 0)
    {
        volume = 0;
    }

    if (volume > 100)
    {
        volume = 100;
    }

    // TODO, check we need to set the left/right volume or not.
    // It seems the sound chip only supports mono.
#ifndef _WINDOWS
    volume = volume | (volume << 8);
    int ret = -1;
    if (ret = ioctl(device_, SOUND_MIXER_WRITE_VOLUME, &volume) ==  - 1)
    {
        printf("SOUND_MIXER_WRITE_VOLUME ioctl failed.\n");
        return false;
    }

    // Call it again, there is bug with sound driver.
    if (ret = ioctl(device_, SOUND_MIXER_WRITE_VOLUME, &volume) ==  - 1)
    {
        printf("SOUND_MIXER_WRITE_VOLUME ioctl failed.\n");
        return false;
    }
#endif
    return true;
}

bool Sound::setBitsPerSample(int bps)
{
#ifndef _WINDOWS
    bps_ = bps;
    int ret = -1;
    int arg = bps;
    if (ret = ioctl(device_, SOUND_PCM_WRITE_BITS, &arg) ==  - 1)
    {
        printf("SOUND_PCM_WRITE_BITS ioctl failed.\n");
        return false;
    }
    if (arg != bps)
    {
        printf("unable to set sample size.\n");
        return false;
    }
#endif

    return true;
}

bool Sound::setChannels(int channels)
{
    channels_ = channels;
    if (channels != 2)
    {
        channels = 2;
    }

#ifndef _WINDOWS
    int ret = -1;
    int arg = channels;
    if (ret = ioctl(device_, SOUND_PCM_WRITE_CHANNELS, &arg) ==  - 1)
    {
        printf("SOUND_PCM_WRITE_CHANNELS ioctl failed.\n");
        return false;
    }
    if (arg != channels)
    {
        printf("unable to set number of channels.\n");
        return false;
    }
#endif

    return true;
}

bool Sound::setSamplingRate(const int rate)
{
#ifndef _WINDOWS
    int ret = -1;
    int arg = rate;
    if (ret = ioctl(device_, SOUND_PCM_WRITE_RATE, &arg) ==  - 1)
    {
        printf("SOUND_PCM_WRITE_RATE ioctl failed.\n");
        return false;
    }
#endif

    return true;
}

/// Not sure the use of SOUND_MIXER_WRITE_RECSRC, seems it does not support
/// sound record. It's necessary to call this to initialize the sound driver
/// otherwise we can not get any voice.
bool Sound::setRec()
{
#ifndef _WINDOWS
    int arg = SOUND_MASK_LINE;
    int ret = -1;
    if (ret = ioctl(device_, SOUND_MIXER_WRITE_RECSRC, &arg) == -1)
    {
        printf("SOUND_MIXER_WRITE_RECSRC failed! \n");
        return false;
    }
#endif
    return true;
}


bool Sound::play(const char *data, int size)
{
    if (!isEnabled())
    {
        qDebug("Device is disabled by caller.");
        return false;
    }

    const char *buffer = data;
    // have to convert
    QByteArray d;
    if (channels_ != 0 && channels_ != 2)
    {
        d = toStereo(data, size, bps_);
        buffer = d.constData();
        size = d.size();
    }
#ifndef _WINDOWS
    int written = 0;
    const int buffer_size = 16 * 1024;      /// Not sure yet.
    while(written < size)
    {
        int left = size - written;
        int count = write(device_, buffer, buffer_size > left ?  left : buffer_size );
        if(count <= 0)
        {
            printf( "Could not write data to device.\n" );
            return false;
        }

        if (!isEnabled())
        {
            printf("Device is disabled by caller during playing.");
            return false;
        }
        written += count;
        buffer += count;
    }
#endif

    return true;
}



