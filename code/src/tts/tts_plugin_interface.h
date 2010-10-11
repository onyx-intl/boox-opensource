#ifndef TTS_PLUGIN_H
#define TTS_PLUGIN_H

#include "sound/sound.h"
#include "sound/async_player.h"

class TTSPlugin
{
public:
    virtual ~TTSPlugin() {}
    virtual bool support();
    virtual TTS_State state();
    virtual void setState(TTS_State state);
    virtual bool isPlaying();

    virtual int span();
    virtual void setSpan(const int s = 100) ;

    virtual bool speakers(QStringList & list);
    virtual bool currentSpeaker(QString & speaker);
    virtual bool setSpeaker(const QString & speaker);

    virtual bool speeds(QVector<int> & list);
    virtual bool currentSpeed(int & speed);
    virtual bool setSpeed(int speed);

    virtual bool styles(QVector<int> & styles);
    virtual bool currentStyle(int & style);
    virtual bool setStyle(int style);
    Sound & sound();
};

Q_DECLARE_INTERFACE(TTSPlugin, "com.onyx-international.Plugin.TTSPluginInterface/1.0");

#endif 
