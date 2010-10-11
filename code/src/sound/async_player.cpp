#include <stdlib.h>
#include <stdio.h>
#include <QThreadPool>
#include "async_player.h"


AsyncPlayer::~AsyncPlayer()
{
}


bool AsyncPlayer::play(Sound & sound, const QByteArray & data, int key)
{
    SoundRunnable * object = new SoundRunnable(sound, data, key);
    connect(object, SIGNAL(finished(int)), this, SLOT(onPlayed(int)));
    QThreadPool::globalInstance()->start(object);
    return true;
}

bool AsyncPlayer::play(Sound & sound, const unsigned char * data, qint64 len, int key)
{
    QByteArray d(reinterpret_cast<const char *>(data), static_cast<int>(len));
    return play(sound, d, key);
}

void AsyncPlayer::waitForDone()
{
    QThreadPool::globalInstance()->waitForDone();
}

void AsyncPlayer::onPlayed(int key)
{
    emit playFinished(key);
}
