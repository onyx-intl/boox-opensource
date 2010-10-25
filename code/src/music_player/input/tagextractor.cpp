#include "tagextractor.h"

namespace player
{

TagExtractor::TagExtractor(QIODevice *d)
{
    d_ = d;
}

TagExtractor::~TagExtractor()
{
}

const QMap<PlayerUtils::MetaData, QString> TagExtractor::id3v2tag()
{
    QByteArray array = d_->peek(2048);
    int offset = array.indexOf("ID3");
    if (offset < 0)
    {
        return tag_;
    }

    ID3v2Tag taglib_tag(&array, offset);
    if (taglib_tag.isEmpty())
    {
        return tag_;
    }

    TagLib::String album = taglib_tag.album();
    TagLib::String artist = taglib_tag.artist();
    TagLib::String comment = taglib_tag.comment();
    TagLib::String genre = taglib_tag.genre();
    TagLib::String title = taglib_tag.title();

    QSettings settings(PlayerUtils::configFile(), QSettings::IniFormat);
    settings.beginGroup("MAD");
    QByteArray name = settings.value("ID3v2_encoding","UTF-8").toByteArray ();
    bool utf = FALSE;
    QTextCodec *codec = 0;
    if (name.contains("UTF"))
    {
        codec = QTextCodec::codecForName ("UTF-8");
        utf = TRUE;
    }
    else
        codec = QTextCodec::codecForName(name);
    settings.endGroup();

    if (!codec)
        codec = QTextCodec::codecForName ("UTF-8");

    tag_.insert(PlayerUtils::ALBUM,
                 codec->toUnicode(album.toCString(utf)).trimmed());
    tag_.insert(PlayerUtils::ARTIST,
                 codec->toUnicode(artist.toCString(utf)).trimmed());
    tag_.insert(PlayerUtils::COMMENT,
                 codec->toUnicode(comment.toCString(utf)).trimmed());
    tag_.insert(PlayerUtils::GENRE,
                 codec->toUnicode(genre.toCString(utf)).trimmed());
    tag_.insert(PlayerUtils::TITLE,
                 codec->toUnicode(title.toCString(utf)).trimmed());
    tag_.insert(PlayerUtils::YEAR,
                 QString::number(taglib_tag.year()));
    tag_.insert(PlayerUtils::TRACK,
                 QString::number(taglib_tag.track()));

    return tag_;

}

ID3v2Tag::ID3v2Tag(QByteArray *array, long offset) : TagLib::ID3v2::Tag()
{
    buf_ = new QBuffer(array);
    buf_->open(QIODevice::ReadOnly);
    offset_ = offset;
    read();
}

void ID3v2Tag::read ()
{
    buf_->seek(offset_);
    uint to_read = TagLib::ID3v2::Header::size();
    if (to_read > 2048 - uint(offset_))
    {
        return;
    }

    header()->setData(TagLib::ByteVector(buf_->read(to_read).data(), to_read));
    to_read = header()->tagSize();
    if (!to_read ||  2048 < offset_ + TagLib::ID3v2::Header::size())
    {
        return;
    }

    QByteArray array = buf_->read(to_read);
    TagLib::ByteVector v(array.data(), array.size());
    parse(v);
}

}
