/*
* Copyright (C) 2004-2009 Geometer Plus <contact@geometerplus.com>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
* 02110-1301, USA.
*/

#include <QtCore/QtCore>

#include "ZLQtFSManager.h"
#include "../../../../core/src/filesystem/ZLFSDir.h"

std::string ZLQtFSManager::convertFilenameToUtf8(const std::string &name) const {
    if (name.empty()) {
        return name;
    }

    QString qString = QString::fromLocal8Bit(name.c_str());
    return (qString == QString::null) ? "" : (const char*)qString.toUtf8();
}


void ZLQtFSManager::normalize(std::string &path) const
{
    QFileInfo info(QString::fromLocal8Bit(path.c_str()));
    path = info.absoluteFilePath().toStdString();
}

std::string ZLQtFSManager::resolveSymlink(const std::string &path) const
{
    QFileInfo info(QString::fromLocal8Bit(path.c_str()));
    return info.absoluteFilePath().toStdString();
}

ZLFSDir *ZLQtFSManager::createNewDirectory(const std::string &path) const
{
    QDir dir;
    if (dir.mkdir(QString::fromLocal8Bit(path.c_str())))
    {
        return createPlainDirectory(dir.absolutePath().toStdString());
    }
    return 0;
}

bool ZLQtFSManager::removeFile(const std::string &path) const
{
    return QFile::remove(QString::fromLocal8Bit(path.c_str()));
}

ZLFileInfo ZLQtFSManager::fileInfo(const std::string &path) const
{
    QFileInfo info(QString::fromLocal8Bit(path.c_str()));
    ZLFileInfo result;
    result.Exists = info.exists();
    result.IsDirectory = info.isDir();
    result.Size = info.size();
    return result;
}

int ZLQtFSManager::findArchiveFileNameDelimiter(const std::string &path) const
{
#ifndef _WINDOWS
    return path.rfind(':');
#else
    int index = path.rfind(':');
    return (index == 1) ? -1 : index;
#endif
}

shared_ptr<ZLDir> ZLQtFSManager::rootDirectory() const
{
    return static_cast<ZLDir*>(createPlainDirectory(QDir::root().absolutePath().toStdString()));
}

const std::string &ZLQtFSManager::rootDirectoryPath() const
{
    static const std::string root = QDir::root().absolutePath().toStdString();
    return root;
}

std::string ZLQtFSManager::parentPath(const std::string &path) const
{
    if (path == rootDirectoryPath())
    {
        return path;
    }
    int index = findLastFileNameDelimiter(path);
    return (index <= 0) ? rootDirectoryPath() : path.substr(0, index);
}
