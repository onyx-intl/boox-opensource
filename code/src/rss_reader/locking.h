/*	NewsFlash
		Copyright 2010 Daniel Goﬂ (Flash Systems)

		This file is part of NewsFlash

    NewsFlash is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    NewsFlash is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with NewsFlash.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <QList>
#include <QString>
#include <QMutex>

///<summary>Allows thread safe locking using strings as a key.</summary>
class CLock
{
	//-- Private methods (for singleton)
	private:
		static QMutex& ListMutex();
		static QList<QString>& LockList();
		
  //-- Public static methods
  public:
    static void Lock(QString lockNamespace, QString uid);
		static void Unlock(QString lockNamespace, QString uid);
		static bool IsLocked(QString lockNamespace, QString uid);
};
