#ifndef NABOO_ASYNC_PLAYER_H_
#define NABOO_ASYNC_PLAYER_H_

#include "sound.h"
#include <QObject>
#include <QByteArray>
#include <QRunnable>
#include <QTime>


class SoundRunnable : public QObject
                    , public QRunnable
{
    Q_OBJECT
public:
    SoundRunnable(Sound & sound, const QByteArray & data, int key)
        : sound_(sound)
        , key_(key)
    {
        data_.append(data);
        setAutoDelete(true);
    }

    ~SoundRunnable()
    {
    }

    void run()
    {
        if (!sound_.isEnabled())
        {
            return;
        }
#ifdef BUILD_FOR_ARM
        // If caller close the device or disable the device,
        // the function will return immediately,
        // so we don't need to check it any more here.
        // The signal is emitted in working thread. The thread safety
        // is implemented by Qt.
        sound_.play(data_.constData(), data_.size());
#else
        QTime t; t.start();
        while (t.elapsed() < 1500)
        {
        }
#endif
        emit finished(key_);
    }

Q_SIGNALS:
    void finished(int key);

private:
    QByteArray data_;
    Sound & sound_;
    int key_;
};

/// Async player which is used to play sound in a async way.
class AsyncPlayer : public QObject
{
    Q_OBJECT
public:
    static AsyncPlayer & instance()
    {
        static AsyncPlayer instance_;
        return instance_;
    }
    ~AsyncPlayer();

public:
    bool play(Sound & sound, const QByteArray & data, int key = 0);
    bool play(Sound & sound, const unsigned char * data, qint64 len, int key = 0);
    void waitForDone();

Q_SIGNALS:
    void playFinished(int key);

private Q_SLOTS:
    void onPlayed(int key);

private:
    AsyncPlayer(){}
    AsyncPlayer(const AsyncPlayer&) {}
};


#endif // NABOO_ASYNC_PLAYER_H_
