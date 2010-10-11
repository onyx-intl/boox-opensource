//vim: set ft=cpp:fenc=utf-8:sw=4:et:ts=4:tw=80:ai:si:fdm=indent:nu
/*
* Onyx international, Inc
* Note: this is an interface to TTS backend
*/

#ifndef ONYX_LIB_TTS_PLUGIN_H_
#define ONYX_LIB_TTS_PLUGIN_H_
#include <QString>
#include "tts_interface.h"
using namespace tts;

class TTSPlugin : public TTSInterface
{
public:
    ~TTSPlugin() {};

public:
    virtual bool initialize(const QLocale & locale, Sound & sound);

    virtual bool synthText(const QString & text);
    virtual void stop();

    virtual bool speakers(QStringList & list);
    virtual bool currentSpeaker(QString & speaker);
    virtual bool setSpeaker(const QString & speaker);

    virtual bool speeds(QVector<int> & list);
    virtual bool currentSpeed(int & speed);
    virtual bool setSpeed(int speed);

    virtual bool styles(QVector<int> & styles);
    virtual bool currentStyle(int & style);
    virtual bool setStyle(int style);
};

Q_DECLARE_INTERFACE(TTSPlugin,
                     "com.onyx-international.Plugin.TTSPlugin/1.0");
#endif
