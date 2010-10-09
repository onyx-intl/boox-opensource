#ifndef ONYX_LIB_TTS_EJ_H_
#define ONYX_LIB_TTS_EJ_H_

#include "../tts_interface.h"
#include "inc/eJTTS.h"
#include "onyx/ui/ui_global.h"

namespace tts
{

/// EJ Sound based tts backend.
class EJSound : public TTSInterface
{
public:
    EJSound();
    ~EJSound();

public:
    virtual bool initialize(const QLocale & locale, Sound & sound);
    virtual bool synthText(const QString & text);
    virtual void stop() { stop_ = true; }

    virtual bool speakers(QStringList & list);
    virtual bool currentSpeaker(QString & speaker);
    virtual bool setSpeaker(const QString & speaker);

    virtual bool speeds(QVector<int> & list);
    virtual bool currentSpeed(int & speed);
    virtual bool setSpeed(int speed);

    virtual bool styles(QVector<int> & styles);
    virtual bool currentStyle(int & style);
    virtual bool setStyle(int style);

private:
    bool create(const QLocale & locale);
    bool destroy();
    bool isInitialized() { return initialized_; }

    QString prefix();
    QString packageName(const QString &language, const QString & speaker);
    bool package(const QString &language, const QString & speaker, std::string & name);

    bool isStopped() { return stop_; }
    QMap<int, int> & speed_map();

    ui::SpeakStyle toStandard(int);
    int toLibrary(ui::SpeakStyle);

    void reload();
    bool isStop() { return stop_; }

private:
    static jtErrCode outputCallback(void* parameter, long outputFormat, void* data, long size);

private:
    unsigned long handle_;        ///< TTS handle.
    unsigned char * heap_;          ///< Allocated chunk.
    QFile file_;
    bool initialized_;
    bool stop_;
    QByteArray data_;
    ui::SpeakStyle current_style_;
    int current_speed_;
    QMap<int, int> speed_map_;
    QString package_;
    QString current_speaker_;
};

}

#endif
