#ifndef PLAYER_PLAYLISTITEM_H_
#define PLAYER_PLAYLISTITEM_H_

#include <utils/player_utils.h>
#include "abstractplaylistitem.h"

namespace player
{

class FileInfo;

/// The PlayListItem class provides an item for use with the PlayListModel class.
class PlayListItem : public AbstractPlaylistItem
{
public:
    enum FLAGS
    {
        FREE = 0,              /*!< instance is free and may be deleted */
        EDITING,               /*!< instance is currently busy */
        SCHEDULED_FOR_DELETION /*!< instance is sheduled for deletion */
    };

public:
    PlayListItem();
    PlayListItem(FileInfo *info);
    ~PlayListItem();

    inline FileInfo* fileInfo() { return info_; }

    void setSelected(bool select);
    bool isSelected() const;
    void setCurrent(bool yes);
    bool isCurrent() const;
    FLAGS flag() const;
    void setFlag(FLAGS);
    const QString text() const;
    void setText(const QString &title);
    void updateMetaData(const QMap<PlayerUtils::MetaData, QString> &metaData);
    void updateTags();

private:
    void readMetadata();
    QString printTag(QString str, QString regExp, QString tagStr);

private:
    QString   title_;
    FileInfo* info_;
    bool      selected_;
    bool      current_;
    FLAGS     flag_;
};

};

#endif
