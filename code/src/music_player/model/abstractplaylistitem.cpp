#include "abstractplaylistitem.h"

namespace player
{

AbstractPlaylistItem::AbstractPlaylistItem()
{
    length_ = 0;
}

AbstractPlaylistItem::~AbstractPlaylistItem()
{
}

const QString AbstractPlaylistItem::title () const
{
    return metadata_.value(PlayerUtils::TITLE);
}

const QString AbstractPlaylistItem::artist () const
{
    return metadata_.value(PlayerUtils::ARTIST);
}

const QString AbstractPlaylistItem::album () const
{
    return metadata_.value(PlayerUtils::ALBUM);
}

const QString AbstractPlaylistItem::comment () const
{
    return metadata_.value(PlayerUtils::COMMENT);
}

const QString AbstractPlaylistItem::genre () const
{
    return metadata_.value(PlayerUtils::GENRE);
}

const QString AbstractPlaylistItem::track () const
{
    return metadata_.value(PlayerUtils::TRACK);
}

const QString AbstractPlaylistItem::year () const
{
    return metadata_.value(PlayerUtils::YEAR);
}

const QString AbstractPlaylistItem::url () const
{
    return metadata_.value(PlayerUtils::URL);
}

qint64 AbstractPlaylistItem::length ()
{
    return length_;
}

bool AbstractPlaylistItem::isEmpty()
{
    return metadata_.isEmpty();
}

void AbstractPlaylistItem::clear()
{
    metadata_.clear();
    length_ = 0;
}

void AbstractPlaylistItem::setMetaData(const QMap<PlayerUtils::MetaData, QString> &metaData)
{
    metadata_ = metaData;
}

void AbstractPlaylistItem::setMetaData(PlayerUtils::MetaData key, const QString &value)
{
    metadata_.insert(key, value);
}

void AbstractPlaylistItem::setLength(qint64 length)
{
    length_ = length;
}

}
