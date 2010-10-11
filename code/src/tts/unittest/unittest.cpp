
#include "onyx/base/base.h"
#include "onyx/ui/ui.h"
#include "testing/testing.h"
#include "tts/tts.h"

using namespace tts;

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    if (argc != 5)
    {
        printf("Usage: tts_unittest bps channels rate filename\n");
        return -1;
    }

    int bps = app.arguments().at(1).toInt();
    int channels = app.arguments().at(2).toInt();
    int rate = app.arguments().at(3).toInt();
    QString name = app.arguments().at(4);

    TTS tts(QLocale::system());
    tts.support();

    // Read file
    QString text;
    QByteArray data;
    QFile file(name);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug("Could not open document %s", qPrintable(name));
        return -1;
    }
    data = file.readAll();


    tts.sound().setBitsPerSample(bps);
    tts.sound().setChannels(channels);
    tts.sound().setSamplingRate(rate);
    tts.sound().setRec();
    for(int i = 0; i < 1; ++i)
    {
        tts.sound().setVolume(90);
        qDebug("playing now.");
        // tts.sound().play(data.constData(), data.size());
        tts.speak(QString::fromUtf8(data));
    }

    qDebug("play done");
    return app.exec();
}
