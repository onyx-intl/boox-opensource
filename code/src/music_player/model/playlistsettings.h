#ifndef PLAYER_PLAYLISTSETTINGS_H_
#define PLAYER_PLAYLISTSETTINGS_H_

#include <utils/player_utils.h>

namespace player
{

class PlaylistSettings
{
public:
    PlaylistSettings();
    ~PlaylistSettings();

    bool convertUnderscore();
    bool convertTwenty();
    bool useMetadata();
    const QString format() const;
    void setConvertUnderscore(bool);
    void setConvertTwenty(bool);
    void setFormat(const QString &format);
    void setUseMetadata(bool);

    static PlaylistSettings* instance();

private:
    bool    convert_underscore_;
    bool    convert_twenty_;
    bool    use_metadata_;
    QString format_;

private:
    static PlaylistSettings* instance_;
};

};

#endif
