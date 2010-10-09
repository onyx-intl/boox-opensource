#include <core/decoder.h>

#include "fileloader.h"
#include "playlistsettings.h"
#include "playlistitem.h"

namespace player
{

FileLoader::FileLoader(QObject *parent)
    : QThread(parent)
    , files_to_load_()
    , directory_()
{
    filters_ = Decoder::nameFilters();
    finished_ = false;
}


FileLoader::~FileLoader()
{
    qWarning("FileLoader::~FileLoader()");
}


void FileLoader::addFiles(const QStringList &files)
{
    if (files.isEmpty ())
    {
        return;
    }

    foreach(QString s, files)
    {
        QList <FileInfo *> playList;
        Decoder::createPlayList(s, playList, PlaylistSettings::instance()->useMetadata());
        foreach(FileInfo *info, playList)
        {
            emit newPlayListItem(new PlayListItem(info));
        }
        if (finished_)
        {
            return;
        }
    }
}


void FileLoader::addDirectory(const QString& s)
{
    QList <FileInfo *> playList;
    QDir dir(s);
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setSorting(QDir::Name);
    QFileInfoList l = dir.entryInfoList(filters_);
    for (int i = 0; i < l.size(); ++i)
    {
        QFileInfo fileInfo = l.at(i);
        Decoder::createPlayList(fileInfo.absoluteFilePath (),
                                playList,
                                PlaylistSettings::instance()->useMetadata());
        foreach(FileInfo *info, playList)
        {
            emit newPlayListItem(new PlayListItem(info));
        }
        if (finished_) return;
    }
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::Name);
    l.clear();
    l = dir.entryInfoList();
    if (l.size() > 0)
    {
        for (int i = 0; i < l.size(); ++i)
        {
            QFileInfo fileInfo = l.at(i);
            addDirectory(fileInfo.absoluteFilePath ());
            if (finished_) return;
        }
    }
}

void FileLoader::run()
{
    if (!files_to_load_.isEmpty())
    {
        addFiles(files_to_load_);
    }
    else if (!directory_.isEmpty())
    {
        addDirectory(directory_);
    }
}

void FileLoader::setFilesToLoad(const QStringList & l)
{
    files_to_load_ = l;
    directory_ = QString();
}

void FileLoader::setDirectoryToLoad(const QString & d)
{
    directory_ = d;
    files_to_load_.clear();
}

void FileLoader::finish()
{
    finished_ = true;
}

}
