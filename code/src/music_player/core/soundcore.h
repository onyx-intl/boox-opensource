#ifndef PLAYER_SOUNDCORE_H_
#define PLAYER_SOUNDCORE_H_

#include <utils/player_utils.h>

#include "decoder.h"
#include "output.h"

namespace player
{

///The SoundCore class provides a simple interface for audio playback.
class SoundCore : public QObject
{
    Q_OBJECT
public:
    SoundCore(QObject *parent = 0);
    ~SoundCore();

    /// Returns length in milliseconds
    qint64  totalTime() const;

    void    setEQ(double bands[10], double preamp);
    void    setEQEnabled(bool on);

    int     leftVolume();
    int     rightVolume();
    bool    softwareVolume();
    qint64  elapsed();
    int     bitrate();
    quint32 frequency();
    int     precision();
    int     channels();
    PlayerUtils::State state() const;
    QMap<PlayerUtils::MetaData, QString> metaData();
    QString metaData(PlayerUtils::MetaData key);

    static SoundCore* instance();

public Q_SLOTS:
    void setSoftwareVolume(bool yes);
    void setVolume(int left, int right);
    bool play(const QString &source);
    void stop();
    void pause();
    void seek(qint64 pos);
    const QString url();

Q_SIGNALS:
    void bufferingProgress(int progress);
    void elapsedChanged(qint64 time);
    void bitrateChanged(int bitrate);
    void frequencyChanged(quint32 frequency);
    void precisionChanged(int precision);
    void channelsChanged(int channels);
    void metaDataChanged ();
    void stateChanged(PlayerUtils::State newState);
    void finished();
    void volumeChanged(int left, int right);

private Q_SLOTS:
    bool decode();

private:
    scoped_ptr<Decoder>        decoder_;
    DecoderFactory*            factory_;
    QString                    source_;
    scoped_ptr<Output>         output_;
    scoped_ptr<QIODevice>      input_;
    uint                       error_;
    bool                       paused_;
    bool                       update_;
    bool                       block_;
    bool                       use_eq_;
    double                     preamp_;
    double                     bands_[10];
    QWidget*                   parent_widget_;
    scoped_ptr<StateHandler>   handler_;
    scoped_ptr<VolumeControl>  volume_control_;

private:
    static SoundCore* instance_;
};

};

#endif
