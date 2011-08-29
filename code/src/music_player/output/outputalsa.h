#ifndef PLAYER_OUTPUT_ALSA_H_
#define PLAYER_OUTPUT_ALSA_H_

#include <core/output.h>
#include <core/volumecontrol.h>

#ifdef BUILD_WITH_TFT
#include "akuio/akuio.h"
#include "alsa/asoundlib.h"
#endif

namespace player
{

class OutputAlsa : public Output
{
    Q_OBJECT
public:
    OutputAlsa(QObject * parent = 0);
    ~OutputAlsa();

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

private:
#ifdef BUILD_WITH_TFT
    snd_pcm_t *pcm_handle_;
#endif
    int byte_per_frames_;
};

class AlsaVolumeControl : public VolumeControl
{
    Q_OBJECT
public:
    AlsaVolumeControl(QObject *parent = 0);
    ~AlsaVolumeControl();

    void setVolume(int left, int right);

protected:
    void volume(int *left, int *right);
};


};

#endif // PLAYER_OUTPUT_ALSA_H
