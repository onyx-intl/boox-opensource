#ifndef PLAYER_DECODER_H_
#define PLAYER_DECODER_H_

#include <utils/player_utils.h>

#include "effect.h"
#include "statehandler.h"
#include "decoderfactory.h"

namespace player
{

/// The Decoder class provides the base interface class of audio decoders.
class Decoder : public QThread
{
    Q_OBJECT
public:
    Decoder(QObject *parent, DecoderFactory *d, QIODevice *input = 0, Output *output = 0);
    Decoder(QObject *parent, DecoderFactory *d, Output *output);
    virtual ~Decoder();

    virtual bool initialize() = 0;
    virtual qint64 totalTime() = 0;
    virtual void seek(qint64 time);
    virtual void stop() = 0;
    virtual void pause(){};

    DecoderFactory *factory() const;
    QIODevice *input();
    Output *output();
    QMutex *mutex();
    QWaitCondition *cond();
    StateHandler *stateHandler();
    void setStateHandler(StateHandler *handler);

    virtual void setEQ(double bands[10], double preamp);
    virtual void setEQEnabled(bool on);

    static bool supports(const QString &file);
    static DecoderFactory *findByPath(const QString &path);
    static DecoderFactory *findByMime(const QString &mime);
    static DecoderFactory *findByContent(QIODevice *input);
    static DecoderFactory *findByURL(const QUrl &url);
    static bool createPlayList(const QString &path,
                               QList<FileInfo *> &results,
                               bool useMetaData = TRUE);
    static QStringList filters();
    static QStringList nameFilters();
    static QList<DecoderFactory*> *factories();
    static QStringList files();
    static void setEnabled(DecoderFactory* factory, bool enable = TRUE);
    static bool isEnabled(DecoderFactory* factory);

Q_SIGNALS:
    void playbackFinished();

protected:
    void configure(quint32 srate, int chan, int bps);
    qint64 produceSound(char *data, qint64 size, quint32 brate, int chan);

protected Q_SLOTS:
    void finish();

private:
    void init();
    static void checkFactories();

protected:
    qint64          seek_time_;
    bool            seeking_finished_;

private:
    DecoderFactory* factory_;
    QList<Effect*>  effects_;
    QIODevice*      input_;
    Output*         output_;

    bool            eq_inited_;
    bool            use_eq_;
    QMutex          mutex_;
    QWaitCondition  wait_condition_;

    uint            block_size_;
    StateHandler*   handler_;

private:
    static QList<DecoderFactory*>* factories_;
    static DecoderFactory*         last_factory_;
    static QStringList             files_;
};

};

#endif // DECODER_H
