#ifndef NABOO_LIB_TTS_AISOUND_H_
#define NABOO_LIB_TTS_AISOUND_H_

#include "../tts_interface.h"
#include "src/speak_lib.h"
#include "bs2b/bs2b.h"

namespace tts
{

/// eSpeak based tts backend.
class ESpeakImpl : public TTSInterface
{
public:
    ESpeakImpl();
    ~ESpeakImpl();

public:
    virtual bool initialize(const QLocale & locale, Sound & sound);
    virtual bool synthText(const QString & text);
    virtual void stop() { stop_ = true; }

private:
    bool create(const QLocale & locale);
    bool destroy();
    bool isStopped() { return stop_; }
    ulong process(char *in_data, const ulong size, char **out_data);
    static int synthCallback(short *wav, int numsamples, espeak_EVENT *events);

private:
    QFile file_;
    bool initialized_;
    bool stop_;
    QByteArray data_;
    t_bs2bdp bs2b_handler_;
};

}

#endif
