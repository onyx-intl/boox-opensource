#include "outputalsa.h"
#include "outputalsafactory.h"

namespace player
{

OutputAlsaFactory::OutputAlsaFactory()
{
}

OutputAlsaFactory::~OutputAlsaFactory()
{
}

const OutputProperties OutputAlsaFactory::properties() const
{
    OutputProperties properties;
    properties.name_ = tr("Alsa Plugin");
    properties.has_about_ = TRUE;
    properties.has_settings_ = FALSE;
    properties.short_name_ = "alsa";
    return properties;
}

Output* OutputAlsaFactory::create(QObject* parent)
{
    return new OutputAlsa(parent);
}

VolumeControl *OutputAlsaFactory::createVolumeControl(QObject* parent)
{
    return new AlsaVolumeControl(parent);
}

void OutputAlsaFactory::showSettings(QWidget* parent)
{
   Q_UNUSED(parent);
}

void OutputAlsaFactory::showAbout(QWidget *parent)
{
}

QTranslator *OutputAlsaFactory::createTranslator(QObject *parent)
{
    QTranslator *translator = new QTranslator(parent);
    QString locale = PlayerUtils::systemLanguageID();
    translator->load(QString(":/alsa_plugin_") + locale);
    return translator;
}

}
