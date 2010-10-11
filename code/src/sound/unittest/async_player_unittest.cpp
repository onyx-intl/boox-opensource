
#include "onyx/base/base.h"
#include "testing/testing.h"
#include "sound/wave.h"
#include "sound/async_player.h"
#include <QFile>

namespace
{

TEST(ASyncPlayer)
{
    QString path = SAMPLE_ROOT;
    path += "/wav/short.wav";

    Sound sound;
    sound.setVolume(90);
    sound.setVolume(90);

    QFile file(path);
    EXPECT_TRUE(file.open(QIODevice::ReadOnly));
    QByteArray data = file.readAll();
    for(int i = 0; i < 100; ++i)
    {
        EXPECT_TRUE(AsyncPlayer::instance().play(sound, data));
    }
}

}

