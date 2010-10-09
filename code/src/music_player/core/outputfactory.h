#ifndef PLAYER_OUTPUTFACTORY_H_
#define PLAYER_OUTPUTFACTORY_H_

#include <utils/player_utils.h>

namespace player
{

class VolumeControl;
class Decoder;
class Output;

/// Helper class to store output plugin properies.
class OutputProperties
{
public:
    OutputProperties()
    {
        has_about_    = FALSE;
        has_settings_ = FALSE;
    }

    QString name_;           /*!< Effect plugin full name */
    QString short_name_;     /*!< Effect plugin short name for internal usage */
    bool    has_about_;      /*!< Should be \b true if plugin has about dialog, otherwise returns \b false */
    bool    has_settings_;   /*!< Should be \b true if plugin has settings dialog, otherwise returns \b false */
};

/// Output plugin interface (output factory).
class OutputFactory
{
public:
    virtual ~OutputFactory() {}
    virtual const OutputProperties properties() const = 0;
    virtual Output *create(QObject *parent) = 0;
    virtual VolumeControl *createVolumeControl(QObject *parent) = 0;
    virtual void showSettings(QWidget *parent) = 0;
    virtual void showAbout(QWidget *parent) = 0;
    virtual QTranslator *createTranslator(QObject *parent) = 0;
};

};

#endif
