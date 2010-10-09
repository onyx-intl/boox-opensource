#include "effectfactory.h"
#include "constants.h"
#include "effect.h"

// TODO Remove this reference
#include <effect/effectbs2bfactory.h>
#include <effect/effectsrconverterfactory.h>

namespace player
{

Effect::Effect(QObject *parent)
    : QObject(parent)
{
}

Effect::~Effect()
{
}

void Effect::configure(quint32 freq, int chan, int res)
{
    freq_ = freq;
    chan_ = chan;
    res_  = res;
}

quint32 Effect::sampleRate()
{
    return freq_;
}

int Effect::channels()
{
    return chan_;
}

int Effect::bitsPerSample()
{
    return res_;
}

static QList<EffectFactory*> *factories = 0;
static QStringList files;

static void checkFactories()
{
    // TODO. Do NOT use the plugin now.
    // Create the MAD decoder factory directly
    if (!factories)
    {
        files.clear();
        factories = new QList<EffectFactory *>;

        EffectFactory *factory = new EffectSRConverterFactory();
        //EffectFactory *factory = new EffectBs2bFactory();

        if (factory)
        {
            factories->append(factory);
            qApp->installTranslator(factory->createTranslator(qApp));
        }
    }
}

QList<Effect*> Effect::create(QObject *parent)
{
    checkFactories();
    QList<Effect*> effects;
    EffectFactory *factory = 0;
    foreach (factory, *factories)
    {
        if(isEnabled(factory))
        {
            effects.append(factory->create(parent));
        }
    }
    return effects;
}

QList<EffectFactory*> *Effect::effectFactories()
{
    checkFactories();
    return factories;
}

QStringList Effect::effectFiles()
{
    checkFactories();
    return files;
}

void Effect::setEnabled(EffectFactory* factory, bool enable)
{
    checkFactories();
    if(!factories->contains(factory))
    {
        return;
    }

    //QString name = factory->properties().shortName;
    //QSettings settings (PlayerUtils::configFile(), QSettings::IniFormat);
    //QStringList effList = settings.value("Effect/enabled_plugins").toStringList();

    //if(enable)
    //{
    //    if (!effList.contains(name))
    //        effList << name;
    //}
    //else
    //    effList.removeAll(name);
    //settings.setValue("Effect/enabled_plugins", effList);

    // Implement Me
}

bool Effect::isEnabled(EffectFactory* factory)
{
    checkFactories();
    if(!factories->contains(factory))
    {
        return false;
    }

    /*QString name = factory->properties().shortName;
    QSettings settings ( PlayerUtils::configFile(), QSettings::IniFormat );
    QStringList effList = settings.value("Effect/enabled_plugins").toStringList();
    return effList.contains(name);*/

    // TODO Implement Me
    return true;
}

}
