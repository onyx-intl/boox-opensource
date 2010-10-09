#ifndef PLAYER_STR_VERSION
#define PLAYER_STR_VERSION "0.3.0"
#endif

#ifndef LIB_DIR
#define LIB_DIR "/lib"
#endif

#include "player_utils.h"

namespace player
{

QString PlayerUtils::config_file_;

const QString PlayerUtils::configFile()
{
    return config_file_.isEmpty() ? QDir::homePath() +"/.MusicPlayer/config" : config_file_;
}

void PlayerUtils::setConfigFile(const QString &path)
{
    config_file_ = path;
}

const QString PlayerUtils::strVersion()
{
#ifdef SVN_REVISION
    return QString("%1-%2").arg(PLAYER_STR_VERSION).arg(SVN_REVISION);
#else
    return PLAYER_STR_VERSION;
#endif
}

bool PlayerUtils::useProxy()
{
    QSettings settings(configFile(), QSettings::IniFormat);
    return settings.value("Proxy/use_proxy", false).toBool();
}

bool PlayerUtils::useProxyAuth()
{
    QSettings settings(configFile(), QSettings::IniFormat);
    return settings.value("Proxy/authentication", false).toBool();
}

const QUrl PlayerUtils::proxy()
{
    QSettings settings(configFile(), QSettings::IniFormat);
    return settings.value("Proxy/url").toUrl();
}

bool PlayerUtils::isShuffled()
{
    QSettings settings(configFile(), QSettings::IniFormat);
    return settings.value("PlayList/is_shuffled", false).toBool();
}

bool PlayerUtils::isRepeatableList()
{
    QSettings settings(configFile(), QSettings::IniFormat);
    return settings.value("PlayList/is_repeatable_list", false).toBool();
}

const int PlayerUtils::leftVolume()
{
    QSettings settings(configFile(), QSettings::IniFormat);
    return settings.value("left_volume").toInt();
}

const int PlayerUtils::rightVolume()
{
    QSettings settings(configFile(), QSettings::IniFormat);
    return settings.value("right_volume").toInt();
}

const QString PlayerUtils::lastSong()
{
    QSettings settings(configFile(), QSettings::IniFormat);
    return settings.value("last_song").toString();
}

void PlayerUtils::setProxyEnabled(bool yes)
{
    QSettings settings(configFile(), QSettings::IniFormat);
    settings.setValue("Proxy/use_proxy", yes);
}

void PlayerUtils::setProxyAuthEnabled(bool yes)
{
    QSettings settings(configFile(), QSettings::IniFormat);
    settings.setValue("Proxy/authentication", yes);
}

void PlayerUtils::setProxy (const QUrl &proxy)
{
    QSettings settings(configFile(), QSettings::IniFormat);
    settings.setValue("Proxy/url", proxy);
}

void PlayerUtils::setShuffled(bool yes)
{
    QSettings settings(configFile(), QSettings::IniFormat);
    settings.setValue("PlayList/is_shuffled", yes);
}

void PlayerUtils::setRepeatableList(bool yes)
{
    QSettings settings(configFile(), QSettings::IniFormat);
    settings.setValue("PlayList/is_repeatable_list", yes);
}

QString PlayerUtils::systemLanguageID()
{
#ifdef Q_OS_UNIX
    QByteArray v = qgetenv ("LC_MESSAGES");
    if (v.isEmpty())
        v = qgetenv ("LC_ALL");
    if (v.isEmpty())
        v = qgetenv ("LANG");
    if (!v.isEmpty())
        return QLocale (v).name();
#endif
    return  QLocale::system().name();
}

void PlayerUtils::setLastSong(const QString &song_path)
{
    QSettings settings(configFile(), QSettings::IniFormat);
    settings.setValue("last_song", song_path);
}

void PlayerUtils::setLeftVolume(const int l)
{
    QSettings settings(configFile(), QSettings::IniFormat);
    settings.setValue("left_volume", l);
}

void PlayerUtils::setRightVolume(const int r)
{
    QSettings settings(configFile(), QSettings::IniFormat);
    settings.setValue("right_volume", r);
}

}
