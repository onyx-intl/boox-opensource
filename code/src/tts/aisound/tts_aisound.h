#ifndef NABOO_LIB_TTS_AISOUND_H_
#define NABOO_LIB_TTS_AISOUND_H_

#include "../tts_interface.h"
#include "ivTTS.h"

namespace tts
{

/// AI Sound based tts backend.
class AISound : public TTSInterface
{
public:
    AISound();
    ~AISound();

public:
    virtual bool initialize(const QLocale & locale, Sound & sound);
    virtual bool synthText(const QString & text);
    virtual void stop() { stop_ = true; }

private:
    bool create(const QLocale & locale);
    bool destroy();
    bool isStopped() { return stop_; }

#ifdef BUILD_FOR_ARM
private:
    static void ivCall readResouceCallback(ivPointer, ivPointer, ivResAddress, ivResSize);
    static ivTTSErrID ivCall outputCallback(ivPointer, ivUInt16, ivCPointer, ivSize);
#endif

private:
#ifdef BUILD_FOR_ARM
    ivHTTS          handle_;        ///< TTS handle.
    ivPByte         heap_;          ///< Allocated chunk.
    ivTResPackDesc  resource_desc_; ///< Resource package description.
#endif
    QFile file_;
    bool initialized_;
    bool stop_;
    QByteArray data_;
};

}

#endif
