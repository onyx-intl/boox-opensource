
#include "wave.h"
#include <QFile>

/// Open a wave file and play it.
bool playWaveFile(const QString & path, Sound & sound)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning("Could not open file %s", qPrintable(path));
        return false;
    }

    WaveHeader header;
    file.read(reinterpret_cast<char *>(&header), sizeof(header));

    // Sample size.
    qDebug("sample size %d", header.bitsPerSample);
    sound.setBitsPerSample(header.bitsPerSample);

    // channels.
    qDebug("channels %d", header.numChannels);
    sound.setChannels(header.numChannels);

    // sample rate.
    qDebug("sample rate %d", header.sampleRate);
    sound.setSamplingRate(header.sampleRate);
    sound.setRec();

    // Read all data.
    QByteArray data = file.read(file.size() - sizeof(header));
    qDebug("set volume to 90");
    sound.setVolume(90);
    sound.setVolume(90);
    sound.play(data.constData(), data.size());
    return true;
}
