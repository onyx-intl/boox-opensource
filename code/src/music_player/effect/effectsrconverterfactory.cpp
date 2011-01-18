#include "effectsrconverterfactory.h"
#include "srconverter.h"

namespace player
{

EffectSRConverterFactory::EffectSRConverterFactory()
{
}

EffectSRConverterFactory::~EffectSRConverterFactory()
{
}

const EffectProperties EffectSRConverterFactory::properties() const
{
    EffectProperties properties;
    properties.name = tr("SR Converter");
    properties.short_name = "SR";
    properties.has_settings = TRUE;
    properties.has_about = TRUE;
    return properties;
};

Effect *EffectSRConverterFactory::create(QObject *parent) 
{
    return new SRConverter(parent);
};

void EffectSRConverterFactory::showSettings(QWidget *parent) 
{
};

void EffectSRConverterFactory::showAbout(QWidget *parent) 
{
};

QTranslator *EffectSRConverterFactory::createTranslator(QObject *parent) 
{
    QTranslator *translator = new QTranslator(parent);
    QString locale = PlayerUtils::systemLanguageID();
    translator->load(QString(":/srconverter_") + locale);
    return translator;
};

}
