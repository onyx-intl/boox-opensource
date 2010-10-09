#include "outputasynplayer.h"
#include "outputasynplayerfactory.h"

namespace player
{

OutputAsynPlayerFactory::OutputAsynPlayerFactory()
{
}

OutputAsynPlayerFactory::~OutputAsynPlayerFactory()
{
}

const OutputProperties OutputAsynPlayerFactory::properties() const
{
    OutputProperties properties;
    properties.name_ = tr("AsynPlayer Plugin");
    properties.has_about_ = TRUE;
    properties.has_settings_ = FALSE;
    properties.short_name_ = "asynplayer";
    return properties;
}

Output* OutputAsynPlayerFactory::create(QObject* parent)
{
    return new OutputAsynPlayer(parent);
}

VolumeControl *OutputAsynPlayerFactory::createVolumeControl(QObject* parent)
{
    return new AsynPlayerVolumeControl(parent);
}

void OutputAsynPlayerFactory::showSettings(QWidget* parent)
{
   Q_UNUSED(parent);
}

void OutputAsynPlayerFactory::showAbout(QWidget *parent)
{
}

QTranslator *OutputAsynPlayerFactory::createTranslator(QObject *parent)
{
    QTranslator *translator = new QTranslator(parent);
    QString locale = PlayerUtils::systemLanguageID();
    translator->load(QString(":/async_player_plugin_") + locale);
    return translator;
}

}
