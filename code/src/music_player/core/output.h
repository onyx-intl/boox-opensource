#ifndef PLAYER_OUTPUT_H_
#define PLAYER_OUTPUT_H_

#include <utils/player_utils.h>

#include "outputfactory.h"
#include "statehandler.h"
#include "recycler.h"

namespace player
{

/// The Output class provides the base interface class of audio outputs.
class Output : public QThread
{
    Q_OBJECT
public:
    Output(QObject * parent = 0);
    ~Output();

    virtual bool   initialize() = 0;
    virtual qint64 latency() = 0;
    virtual void   enable(bool e) = 0;

    virtual void   configure(quint32 freq, int chan, int bits);
    virtual void   pause();

    void      stop();
    void      finish();
    qint64    written();
    void      seek(qint64 pos);
    Recycler* recycler();
    QMutex*   mutex();
    void      setStateHandler(StateHandler *handler);
    quint32   sampleRate();
    int       numChannels();
    int       sampleSize();

    static Output *create(QObject *parent);
    static QList<OutputFactory*> *outputFactories();
    static QStringList outputFiles();
    static void setCurrentFactory(OutputFactory* factory);
    static OutputFactory *currentFactory();

protected:
    virtual qint64 writeAudio(unsigned char *data, qint64 maxSize) = 0;
    virtual void flush() = 0;

private:
    void run(); //thread run function
    void status();
    void changeVolume(uchar *data, qint64 size, int chan);
    void dispatch(qint64 elapsed,
                  qint64 totalTime,
                  int bitrate,
                  int frequency,
                  int precision,
                  int channels);
    void dispatch(const PlayerUtils::State &state);

protected:
    quint32       frequency_;
    int           channels_;
    int           precision_;

private:
    QMutex        mutex_;
    Recycler      recycler_;
    StateHandler* handler_;
    int           kbps_;
    qint64        bytes_per_millisecond_;
    bool          user_stop_;
    bool          pause_;
    bool          finish_;
    bool          is_seeking_;
    qint64        total_written_;
    qint64        current_milliseconds_;

private:
    static void checkFactories();
    static void registerFactory(OutputFactory *);

private:
    //TODO use QMap instead
    static QList<OutputFactory*>* factories_;
    static QStringList            files_;
    static QTimer*                timer_;
};

};

#endif // PLAYER_OUTPUT_H_
