#include "outputwaveout.h"
#include "outputwaveoutfactory.h"

namespace player
{

OutputWaveOutFactory::OutputWaveOutFactory()
{
}

OutputWaveOutFactory::~OutputWaveOutFactory()
{
}

const OutputProperties OutputWaveOutFactory::properties() const
{
    OutputProperties properties;
    properties.name_ = tr("WaveOut Plugin");
    properties.has_about_ = TRUE;
    properties.has_settings_ = FALSE;
    properties.short_name_ = "waveout";
    return properties;
}

Output* OutputWaveOutFactory::create(QObject* parent)
{
    return new OutputWaveOut(parent);
}

VolumeControl *OutputWaveOutFactory::createVolumeControl(QObject *)
{
    return 0;
}

void OutputWaveOutFactory::showSettings(QWidget* parent)
{
   Q_UNUSED(parent);
}

void OutputWaveOutFactory::showAbout(QWidget *parent)
{
}

QTranslator *OutputWaveOutFactory::createTranslator(QObject *parent)
{
    QTranslator *translator = new QTranslator(parent);
    QString locale = PlayerUtils::systemLanguageID();
    translator->load(QString(":/waveout_plugin_") + locale);
    return translator;
}

}
