#include <core/decoder.h>
#include <core/fileinfo.h>

#include "playlistsettings.h"
#include "playlistitem.h"

namespace player
{

PlayListItem::PlayListItem()
    : AbstractPlaylistItem()
    , info_(0)
    , flag_(FREE)
{
}

PlayListItem::PlayListItem(FileInfo *info)
    : AbstractPlaylistItem()
    , info_(info)
    , selected_(false)
    , current_(false)
    , flag_(FREE)
{
    setMetaData(info->metaData());
    setMetaData(PlayerUtils::URL, info_->path());
    setLength(info_->length());
    readMetadata();
}

PlayListItem::~PlayListItem()
{
    if (info_)
    {
        delete info_;
    }
}

void PlayListItem::setSelected(bool yes)
{
    selected_ = yes;
}

bool PlayListItem::isSelected() const
{
    return selected_;
}

void PlayListItem::setCurrent(bool yes)
{
    current_ = yes;
}

bool PlayListItem::isCurrent() const
{
    return current_;
}

void PlayListItem::setFlag(FLAGS f)
{
    flag_ = f;
}

PlayListItem::FLAGS PlayListItem::flag() const
{
    return flag_;
}

void PlayListItem::updateMetaData(const QMap<PlayerUtils::MetaData, QString> &metaData)
{
    setMetaData(metaData);
    readMetadata();
}

void PlayListItem::updateTags()
{
    if (url().startsWith("http://"))
    {
        return;
    }

    QList<FileInfo*> play_list;
    Decoder::createPlayList(url(), play_list);
    if (!play_list.isEmpty())
    {
        if (info_)
        {
            delete info_;
            info_ = 0;
        }
        info_ = play_list.at(0);
        setMetaData(info_->metaData());
        setMetaData(PlayerUtils::URL, info_->path());
        readMetadata();
    }
}

const QString PlayListItem::text() const
{
    return title_;
}

void PlayListItem::setText(const QString &title)
{
    title_ = title;
}

void PlayListItem::readMetadata()
{
    title_ = PlaylistSettings::instance()->format();
    title_ = printTag(title_, "%p", artist());
    title_ = printTag(title_, "%a", album());
    title_ = printTag(title_, "%t", title());
    title_ = printTag(title_, "%n", QString("%1").arg(track()));
    title_ = printTag(title_, "%g", genre());
    title_ = printTag(title_, "%f", url().section('/',-1));
    title_ = printTag(title_, "%F", url());
    title_ = printTag(title_, "%y", QString("%1").arg(year ()));

    //TODO rewrite this
    if (title_.isEmpty())
    {
        return;
    }

    if (PlaylistSettings::instance()->convertUnderscore())
    {
        title_.replace("_", " ");
    }
    if (PlaylistSettings::instance()->convertTwenty())
    {
        title_.replace("%20", " ");
    }
}

QString PlayListItem::printTag(QString str, QString regExp, QString tagStr)
{
    QString format = PlaylistSettings::instance()->format();
    if (!tagStr.isEmpty())
    {
        str.replace(regExp, tagStr);
    }
    else
    {
        //remove unused separators
        int regExpPos = str.indexOf(regExp);
        if (regExpPos < 0)
            return str;
        int nextPos = str.indexOf("%", regExpPos + 1);
        if (nextPos < 0)
        {
            //last separator
            regExpPos = format.lastIndexOf(regExp);
            nextPos = format.lastIndexOf("%", regExpPos - 1);
            QString lastSep = format.right (format.size() - nextPos - 2);
            str.remove(lastSep);
            str.remove(regExp);
        }
        else
        {
            str.remove(regExpPos, nextPos - regExpPos);
        }
    }
    return str;
}

}
