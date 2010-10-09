#include "playlistsettings.h"

namespace player
{

PlaylistSettings *PlaylistSettings::instance_ = 0;

PlaylistSettings::PlaylistSettings()
{
    QSettings s (PlayerUtils::configFile(), QSettings::IniFormat);
    format_ = s.value("PlayList/title_format", "%p - %t").toString();
    convert_underscore_ = s.value ("PlayList/convert_underscore", TRUE).toBool();
    convert_twenty_ = s.value ("PlayList/convert_twenty", TRUE).toBool();
    use_metadata_ = s.value ("PlayList/load_metadata", TRUE).toBool();
}

PlaylistSettings::~PlaylistSettings()
{
    instance_ = 0;
    QSettings s(PlayerUtils::configFile(), QSettings::IniFormat);
    s.setValue("PlayList/title_format", format_);
    s.setValue("PlayList/convert_underscore", convert_underscore_);
    s.setValue("PlayList/convert_twenty", convert_twenty_);
    s.setValue("PlayList/load_metadata", use_metadata_);
}

PlaylistSettings *PlaylistSettings::instance()
{
    if (!instance_)
    {
        instance_ = new PlaylistSettings();
    }
    return instance_;
}

const QString PlaylistSettings::format() const
{
    return format_;
}

bool PlaylistSettings::convertUnderscore()
{
    return convert_underscore_;
}

bool PlaylistSettings::convertTwenty()
{
    return convert_twenty_;
}

bool PlaylistSettings::useMetadata()
{
    return use_metadata_;
}

void PlaylistSettings::setConvertUnderscore(bool yes)
{
    convert_underscore_ = yes;
}

void  PlaylistSettings::setConvertTwenty(bool yes)
{
    convert_twenty_ = yes;
}

void PlaylistSettings::setFormat(const QString &format)
{
    format_ = format;
}

void PlaylistSettings::setUseMetadata(bool yes)
{
    use_metadata_ = yes;
}

}
