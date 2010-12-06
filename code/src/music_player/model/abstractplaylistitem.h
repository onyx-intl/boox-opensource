#ifndef PLAYER_ABSTRACTPLAYLISTITEM_H_
#define PLAYER_ABSTRACTPLAYLISTITEM_H_

#include <utils/player_utils.h>

namespace player
{

/// The AbstractPlaylistItem class provides the basic functionality for the playlist items.
class AbstractPlaylistItem
{
public:
    AbstractPlaylistItem();
    ~AbstractPlaylistItem();

    const QString title () const;
    const QString artist () const;
    const QString album () const;
    const QString comment () const;
    const QString genre () const;
    const QString track () const;
    const QString year () const;
    const QString url () const;
    qint64 length ();
    bool isEmpty();
    void clear();
    virtual void setMetaData(const QMap<PlayerUtils::MetaData, QString> &metaData);
    virtual void setMetaData(PlayerUtils::MetaData key, const QString &value);
    virtual void setLength(qint64 length);

private:
    QMap<PlayerUtils::MetaData, QString> metadata_;
    qint64                               length_;
};

};

#endif
