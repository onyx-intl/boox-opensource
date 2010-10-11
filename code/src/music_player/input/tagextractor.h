#ifndef PLAYER_TAGEXTRACTOR_H_
#define PLAYER_TAGEXTRACTOR_H_

#include <tag.h>
#include <fileref.h>
#include <id3v1tag.h>
#include <id3v2tag.h>
#include <id3v2header.h>

#include <utils/player_utils.h>

namespace player
{

class TagExtractor
{
public:
    TagExtractor(QIODevice *d);
    ~TagExtractor();

    const QMap<PlayerUtils::MetaData, QString> id3v2tag();

private:
    QMap<PlayerUtils::MetaData, QString> tag_;
    QIODevice *                          d_;

};

class ID3v2Tag : public TagLib::ID3v2::Tag
{
public:
    ID3v2Tag(QByteArray *array, long offset);

protected:
    void read ();

private:
    QBuffer*    buf_;
    long        offset_;
};

};

#endif
