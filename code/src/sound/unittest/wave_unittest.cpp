
#include "onyx/base/base.h"
#include "testing/testing.h"
#include "sound/wave.h"


namespace
{

TEST(PlayWave)
{
    QString path = SAMPLE_ROOT;
    path += "/wav/short.wav";

    Sound sound;
    EXPECT_TRUE(playWaveFile(path, sound));

}

}

