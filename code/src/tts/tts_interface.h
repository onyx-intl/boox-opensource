#ifndef NABOO_LIB_TTS_INTERFACE_H_
#define NABOO_LIB_TTS_INTERFACE_H_

#include <QtGui/QtGui>
#include "sound/sound.h"

namespace tts
{

/// TTS engine interface. The tts engine only needs to provide
/// the sound data from given text. The tts manager then can use
/// the data either to play or just store them in a file.
class TTSInterface : public QObject
{
    Q_OBJECT
public:
    TTSInterface(){}
    virtual ~TTSInterface(){}

public:
    virtual bool initialize(const QLocale & locale, Sound & sound) = 0;
    virtual bool synthText(const QString & text) = 0;
    virtual bool pause() { return true; }
    virtual void stop() = 0;

    virtual bool speakers(QStringList & list) = 0;
    virtual bool currentSpeaker(QString & speaker) = 0;
    virtual bool setSpeaker(const QString & speaker) = 0;

    virtual bool speeds(QVector<int> & list) = 0;
    virtual bool currentSpeed(int & speed) = 0;
    virtual bool setSpeed(int speed) = 0;

    virtual bool styles(QVector<int> & styles) = 0;
    virtual bool currentStyle(int & style) = 0;
    virtual bool setStyle(int style) = 0;

Q_SIGNALS:
    // The byte array should contain PCM data of the QString passed to
    // synthText.
    void synthDone(bool ok, QByteArray &data);

};

}   // namespace tts

#endif  // NABOO_LIB_TTS_INTERFACE_H_
