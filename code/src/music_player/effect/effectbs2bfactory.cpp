#include <libbs2b/bs2bversion.h>
#include "effectbs2bfactory.h"
#include "bs2bconverter.h"

namespace player
{

EffectBs2bFactory::EffectBs2bFactory()
{
}

EffectBs2bFactory::~EffectBs2bFactory()
{
}

const EffectProperties EffectBs2bFactory::properties() const
{
    EffectProperties properties;
    properties.name = tr("BS2B Converter");
    properties.short_name = "bs2b";
    properties.has_settings = TRUE;
    properties.has_about = TRUE;
    return properties;
};

Effect *EffectBs2bFactory::create(QObject *parent)
{
    return new Bs2bConverter(parent);
};

void EffectBs2bFactory::showSettings(QWidget *parent)
{
};

void EffectBs2bFactory::showAbout(QWidget *parent)
{
};

QTranslator *EffectBs2bFactory::createTranslator(QObject *parent)
{
    QTranslator *translator = new QTranslator(parent);
    QString locale = PlayerUtils::systemLanguageID();
    translator->load(QString(":/bs2b_converter_") + locale);
    return translator;
}

}

