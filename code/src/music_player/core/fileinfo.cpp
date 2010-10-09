#include "fileinfo.h"

namespace player
{

FileInfo::FileInfo(const QString &path)
{
    path_ = path;
    length_ = 0;
}

FileInfo::FileInfo(const FileInfo &other)
{
    *this = other;
}

FileInfo::~FileInfo()
{}

void FileInfo::operator=(const FileInfo &info)
{
    setLength(info.length());
    setMetaData(info.metaData());
    setPath(info.path());
}

bool FileInfo::operator==(const FileInfo &info)
{
    return metaData () == info.metaData () &&
           length () == info.length ();
    path() == info.path();
}

bool FileInfo::operator!=(const FileInfo &info)
{
    return !operator==(info);
}

qint64 FileInfo::length () const
{
    return length_;
}

const QString FileInfo::metaData (PlayerUtils::MetaData key) const
{
    return metadata_[key];
}

const QMap<PlayerUtils::MetaData, QString>  FileInfo::metaData () const
{
    return metadata_;
}

void FileInfo::setMetaData(const QMap<PlayerUtils::MetaData,  QString> &metaData)
{
    metadata_ = metaData;
}

bool FileInfo::isEmpty() const
{
    return metadata_.isEmpty(); //TODO add correct test
}

const QString FileInfo::path() const
{
    return path_;
}

void FileInfo::setLength(qint64 length)
{
    length_ = length;
}

void FileInfo::setMetaData(PlayerUtils::MetaData key, const QString &value)
{
    metadata_.insert(key, value);
}

void FileInfo::setMetaData(PlayerUtils::MetaData key, int value)
{
    metadata_.insert(key, QString::number(value));
}

void FileInfo::setPath(const QString &path)
{
    path_ = path;
}

}
