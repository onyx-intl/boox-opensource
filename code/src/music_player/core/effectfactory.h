#ifndef PLAYER_EFFECTFACTORY_H_
#define PLAYER_EFFECTFACTORY_H_

#include <utils/player_utils.h>

namespace player
{

class Effect;

/// Helper class to store effect plugin properies.
class EffectProperties
{
public:
    EffectProperties()
    {
        has_about = false;
        has_settings = false;
    }
    QString name;          /*!< Effect plugin full name */
    QString short_name;    /*!< Effect plugin short name for internal usage */
    bool    has_about;     /*!< Should be \b true if plugin has about dialog, otherwise returns \b false */
    bool    has_settings;  /*!< Should be \b true if plugin has settings dialog, otherwise returns \b false */
};

/// %Effect plugin interface (effect factory).
class EffectFactory
{
public:
    virtual const EffectProperties properties() const = 0;
    virtual Effect *create(QObject *parent) = 0;
    virtual void showSettings(QWidget *parent) = 0;
    virtual void showAbout(QWidget *parent) = 0;
    virtual QTranslator *createTranslator(QObject *parent) = 0;
};

};

#endif
