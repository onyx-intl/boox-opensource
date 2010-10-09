#include <core/constants.h>
#include <core/buffer.h>
#include "outputasynplayer.h"

namespace player
{

AsynPlayerVolumeControl::AsynPlayerVolumeControl(QObject *parent)
{
}

AsynPlayerVolumeControl::~AsynPlayerVolumeControl()
{
}

void AsynPlayerVolumeControl::setVolume(int left, int right)
{
    // TODO. Update the volume by two channels
    SoundObject::instance().sound()->setVolume(qMax(left, right));
}

void AsynPlayerVolumeControl::volume(int *left, int *right)
{
    // TODO. Update the volume by two channels
    *left  = SoundObject::instance().sound()->volume();
    *right = SoundObject::instance().sound()->volume();
}

OutputAsynPlayer::OutputAsynPlayer(QObject * parent)
    : Output(parent)
{
}

OutputAsynPlayer::~OutputAsynPlayer()
{
    uninitialize();
}

void OutputAsynPlayer::configure(quint32 freq, int chan, int prec)
{
    qDebug("Configure Frequency:%d, Channel:%d, Precision:%d", freq, chan, prec);
    SoundObject::instance().sound()->setSamplingRate(freq);
    SoundObject::instance().sound()->setChannels(chan);
    SoundObject::instance().sound()->setBitsPerSample(prec);

    Output::configure(freq, chan, prec);
}

bool OutputAsynPlayer::initialize()
{
    return true;
}

qint64 OutputAsynPlayer::latency()
{
    // TODO. Implement Me
    return 0;
}

void OutputAsynPlayer::enable(bool e)
{
    if (e)
    {
        if (!SoundObject::instance().sound()->isEnabled())
        {
            SoundObject::instance().sound()->enable(true);
        }
    }
    else
    {
        if (SoundObject::instance().sound()->isEnabled())
        {
            SoundObject::instance().sound()->enable(false);
        }
    }
}

qint64 OutputAsynPlayer::writeAudio(unsigned char *data, qint64 len)
{
    // TODO. Check this function
    /*static QByteArray dump_array;
    static int count = 0;
    if (count++ < 100)
    {
        dump_array.append(reinterpret_cast<char*>(data), len);
    }

    if (count == 300)
    {
        QString path = QDir::homePath() + "/dump";
        qDebug("Path:%s", qPrintable(path));
        QFile file(path);
        file.open(QIODevice::Append);
        file.write(dump_array.constData(), dump_array.length());
        file.flush();
    }*/

    if (!SoundObject::instance().sound()->isEnabled())
    {
        return len;
    }

    if (SoundObject::instance().sound()->play(reinterpret_cast<char*>(data), len) || !SoundObject::instance().sound()->isEnabled())
    {
        return len;
    }
    return -1;
}

void OutputAsynPlayer::flush()
{
    // TODO. Implement Me
}

void OutputAsynPlayer::uninitialize()
{
    // TODO. Implement Me
}

}
