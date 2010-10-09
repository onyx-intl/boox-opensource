#ifndef PLAYER_FILEINFO_H_
#define PLAYER_FILEINFO_H_

#include <utils/player_utils.h>

namespace player
{

/// The FileInfo class stores metadata and audio information about media file or stream.
class FileInfo
{
public:
    FileInfo(const QString &path = QString());
    FileInfo(const FileInfo &info);
    ~FileInfo();

    void operator=(const FileInfo &info);
    bool operator==(const FileInfo &info);
    bool operator!=(const FileInfo &info);
    qint64 length () const;
    const QString metaData (PlayerUtils::MetaData key) const;
    const QMap<PlayerUtils::MetaData, QString>  metaData () const;

    bool isEmpty() const;
    const QString path() const;
    void setLength(qint64 length);
    void setMetaData(PlayerUtils::MetaData key, const QString &value);
    void setMetaData(PlayerUtils::MetaData key, int value);
    void setMetaData(const QMap <PlayerUtils::MetaData,  QString> &metaData);
    void setPath(const QString &path);

private:
    QMap<PlayerUtils::MetaData, QString> metadata_;
    qint64  length_;
    QString path_;
};

};

#endif
