#ifndef PLAYER_OUTPUT_ASYNCHRONOUS_H_
#define PLAYER_OUTPUT_ASYNCHRONOUS_H_

#include "sound/sound.h"
#include "sound/async_player.h"

#include <core/output.h>
#include <core/volumecontrol.h>

namespace player
{

class SoundObject
{
public:
    static SoundObject & instance()
    {
        static SoundObject object;
        return object;
    }
    ~SoundObject() {}

    inline Sound* sound() { return sound_.get(); }
    inline void reset() { sound_->enable(false); }

private:
    SoundObject() : sound_(new Sound()) {}
    SoundObject(const SoundObject & right);

private:
    scoped_ptr<Sound> sound_;
};

class OutputAsynPlayer : public Output
{
    Q_OBJECT
public:
    OutputAsynPlayer(QObject * parent = 0);
    ~OutputAsynPlayer();

    bool initialize();
    void configure(quint32, int, int);
    qint64 latency();
    void enable(bool e);

private:
    //output api
    qint64 writeAudio(unsigned char *data, qint64 maxSize);
    void flush();

    // helper functions
    void status();
    void uninitialize();
};

class AsynPlayerVolumeControl : public VolumeControl
{
    Q_OBJECT
public:
    AsynPlayerVolumeControl(QObject *parent = 0);
    ~AsynPlayerVolumeControl();

    void setVolume(int left, int right);

protected:
    void volume(int *left, int *right);
};


};

#endif // PLAYER_OUTPUTWAVEOUT_H
