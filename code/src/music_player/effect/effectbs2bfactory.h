#ifndef PLAYER_EFFECT_BS2BFACTORY_H_
#define PLAYER_EFFECT_BS2BFACTORY_H_

#include <core/effectfactory.h>
#include <core/effect.h>

namespace player
{

class EffectBs2bFactory : public QObject, public EffectFactory
{
    Q_OBJECT
public:
    EffectBs2bFactory();
    virtual ~EffectBs2bFactory();

    const EffectProperties properties() const;
    Effect *create(QObject *parent);
    void showSettings(QWidget *parent);
    void showAbout(QWidget *parent);
    QTranslator *createTranslator(QObject *parent);
};

};

#endif
