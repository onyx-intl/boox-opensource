
#include "onyx/base/base.h"
#include "sound/sound.h"
#include <QtGui/QtGui>


int main(int argc, char * argv[])
{
    QApplication app(argc, argv);
    Sound sound;
    if (argc != 5)
    {
        printf("Usage: sound_unittest bps channels rate data_filename\n");
        return -1;
    }

    int bps = app.arguments().at(1).toInt();
    int channels = app.arguments().at(2).toInt();
    int rate = app.arguments().at(3).toInt();
    QString name = app.arguments().at(4);

    sound.setVolume(100);
    sound.setBitsPerSample(bps);
    sound.setChannels(channels);
    sound.setSamplingRate(rate);

    QByteArray data;
    QFile file(name);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug("Could not open document %s", qPrintable(name));
        return -1;
    }
    data = file.readAll();
    sound.play(data.constData(), data.size());
    return 0;
}


