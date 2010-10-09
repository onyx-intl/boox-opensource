#ifndef PLAYER_UTILS_H_
#define PLAYER_UTILS_H_

#include "onyx/base/base.h"
#include "onyx/base/down_cast.h"
#include "onyx/base/dbus.h"

#include "onyx/ui/ui.h"
#include "onyx/ui/languages.h"
#include "onyx/sys/sys.h"
#include "onyx/screen/screen_proxy.h"
#include "onyx/data/configuration.h"

#include <QMainWindow>

namespace player
{

/// The PlayerUtils class stores global settings and enums.
class PlayerUtils
{
public:
    enum State
    {
        Playing = 0, /*!< The player is playing source */
        Paused,      /*!< The player has currently paused its playback */
        Stopped,     /*!< The player is ready to play source */
        Buffering,   /*!< The Player is waiting for data to be able to start playing.   */
        NormalError, /*!< Input source is invalid or unsupported. Player should skip this file */
        FatalError   /*!< This means unrecorvable error die audio output problems. Player should abort playback. */
    };

    enum MetaData
    {
        TITLE = 0, /*!< Title */
        ARTIST,    /*!< Artist  */
        ALBUM,     /*!< Album */
        COMMENT,   /*!< Comment */
        GENRE,     /*!< Genre */
        YEAR,      /*!< Year */
        TRACK,     /*!< Track number */
        URL        /*!< Stream url or local file path */
    };

    static const QString configFile();
    static void setConfigFile(const QString &path);
    static const QString strVersion();
    static bool useProxy();
    static bool useProxyAuth();
    static const QUrl proxy();
    static const QString lastSong();
    static const int leftVolume();
    static const int rightVolume();
    static bool isShuffled();
    static bool isRepeatableList();

    static void setProxyEnabled(bool yes);
    static void setProxyAuthEnabled(bool yes);
    static void setProxy (const QUrl &proxy);
    static void setLastSong(const QString &song_path);
    static void setLeftVolume(const int l);
    static void setRightVolume(const int r);
    static void setShuffled(bool yes);
    static void setRepeatableList(bool yes);

    static QString systemLanguageID();

private:
    static QString config_file_;

};

};

#endif
