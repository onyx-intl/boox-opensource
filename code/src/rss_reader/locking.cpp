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

#include <QtDebug>

#include "locking.h"

///<summary>Signleton pattern for the list's mutex.</summary>
///<returns>Returns a reference to one (and only one) QMutex instance.</returns>
/*static*/ QMutex& CLock::ListMutex()
{
	static QMutex listMutex;
	return listMutex;
}

///<summary>Signleton pattern for the lock list.</summary>
///<returns>Returns a reference to one (and only one) QList<QString> instance.</returns>
/*static*/ QList<QString>& CLock::LockList()
{
	static QList<QString> lockList;
	return lockList;
}

///<summary>Locks the supplied GUID</summary>
///<param name="lockNamespace">The name of the namespace where the uid is unique. This allows to use this singleton lock for multiple different lockings.</param>
///<param name="uid">The UID to lock</param>
///<remarks>IsLocked will return true for this UID. Every call to Lock must be balanced by a call to Unlock</remarks>
/*static*/ void CLock::Lock(QString lockNamespace, QString uid)
{
	ListMutex().lock();
	LockList().append(lockNamespace+uid);
	ListMutex().unlock();
}

///<summary>Unlocks the supplied GUID</summary>
///<param name="lockNamespace">The name of the namespace where the uid is unique. This allows to use this singleton lock for multiple different lockings.</param>
///<param name="uid">The UID to unlock</param>
///<remarks>The reference count will be decreased by one. If it is zero IsLocked will return false for this UID.</remarks>
/*static*/ void CLock::Unlock(QString lockNamespace, QString uid)
{
	ListMutex().lock();
	if (!LockList().removeOne(lockNamespace+uid))
		qWarning() << "Warning: Unlocking object that has no lock " << lockNamespace << "::" << uid;
	ListMutex().unlock();
}

///<summary>Checks if a supplied GUID is locked</summary>
///<param name="lockNamespace">The name of the namespace where the uid is unique. This allows to use this singleton lock for multiple different lockings.</param>
///<param name="uid">The UID to check</param>
///<returns>Returns true if the UID was previously lock by a call to Lock.</returns>
/*static*/ bool CLock::IsLocked(QString lockNamespace, QString uid)
{
	bool isLocked;
	
	ListMutex().lock();
	isLocked=LockList().contains(lockNamespace+uid);
	ListMutex().unlock();
	return isLocked;
}
