#ifndef PLAYER_EFFECT_SRFACTORY_H_
#define PLAYER_EFFECT_SRFACTORY_H_

#include <core/effectfactory.h>
#include <core/effect.h>

namespace player
{

class EffectSRConverterFactory : public QObject, public EffectFactory
{
    Q_OBJECT
public:
    EffectSRConverterFactory();
    virtual ~EffectSRConverterFactory();

    const EffectProperties properties() const;
    Effect *create(QObject *parent);
    void showSettings(QWidget *parent);
    void showAbout(QWidget *parent);
    QTranslator *createTranslator(QObject *parent);
};

};
#endif
