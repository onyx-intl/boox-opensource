#ifndef PLAYER_FILELOADER_H_
#define PLAYER_FILELOADER_H_

#include <utils/player_utils.h>

namespace player
{

class PlayListItem;

class FileLoader : public QThread
{
    Q_OBJECT
public:
    FileLoader(QObject *parent = 0);
    ~FileLoader();

    void finish();
    void setFilesToLoad(const QStringList&);
    void setDirectoryToLoad(const QString&);

Q_SIGNALS:
    void newPlayListItem(PlayListItem *item);

protected:
    virtual void run();
    void addFiles(const QStringList &files);
    void addDirectory(const QString& s);

private:
    QStringList filters_;
    QStringList files_to_load_;
    QString     directory_;
    bool        finished_;
};

};

#endif
