#ifndef PLAYER_STATEHANDLER_H_
#define PLAYER_STATEHANDLER_H_

#include <utils/player_utils.h>

namespace player
{

/// The StateHandler class allows to track information about playback progress.
class StateHandler : public QObject
{
    Q_OBJECT
public:
    StateHandler(QObject *parent = 0);
    ~StateHandler();

    virtual void dispatch(qint64 elapsed,
                          qint64 totalTime,
                          int bitrate,
                          quint32 frequency,
                          int precision,
                          int channels);
    virtual void dispatch(const QMap<PlayerUtils::MetaData, QString> &metaData);
    virtual void dispatch(const PlayerUtils::State &state);

    qint64 elapsed();
    int bitrate();
    int frequency();
    int precision();
    int channels();
    PlayerUtils::State state() const;
    QMap<PlayerUtils::MetaData, QString> metaData();
    QString metaData(PlayerUtils::MetaData key);
    static StateHandler* instance();

Q_SIGNALS:
    void elapsedChanged(qint64 time);
    void bitrateChanged(int bitrate);
    void frequencyChanged(quint32 frequency);
    void precisionChanged(int precision);
    void channelsChanged(int channels);
    void metaDataChanged();
    void stateChanged(PlayerUtils::State newState);
    void finished();

private:
    qint64    elapsed_;
    quint32   frequency_;
    int       bitrate_;
    int       precision_;
    int       channels_;

    QMap<PlayerUtils::MetaData, QString> metadata_;
    PlayerUtils::State                   state_;
    QMutex                  mutex_;
    bool                    send_meta_;

private:
    static StateHandler* instance_;
};

};

#endif
