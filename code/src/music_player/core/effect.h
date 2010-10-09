#ifndef PLAYER_EFFECT_H_
#define PLAYER_EFFECT_H_

#include <utils/player_utils.h>

namespace player
{

class EffectFactory;

/// The Effect class provides the base interface class of audio effects.
class Effect : public QObject
{
    Q_OBJECT
public:
    Effect(QObject *parent = 0);
    virtual ~Effect();

    virtual ulong process(char *in_data, const ulong size, char **out_data) = 0;
    virtual void configure(quint32 freq, int chan, int res);
    quint32 sampleRate();
    int channels();
    int bitsPerSample();

    static QList<Effect*> create(QObject *parent);
    static QList<EffectFactory*> *effectFactories();
    static QStringList effectFiles();
    static void setEnabled(EffectFactory* factory, bool enable = true);
    static bool isEnabled(EffectFactory* factory);

private:
    quint32 freq_;
    int     chan_;
    int     res_;
};

};

#endif
