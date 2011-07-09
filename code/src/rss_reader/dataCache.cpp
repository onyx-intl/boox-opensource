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


#include "dataCache.h"
#include "conf.h"

///<summary>C'tor</summary>
CDataCache::CDataCache() :
	PreparedCacheEntry(NULL)
{
}

///<summary>D'tor</summary>
CDataCache::~CDataCache()
{
	QHashIterator<QString, SCacheEntry*> cacheI(Cache);
	
	while (cacheI.hasNext())
		delete cacheI.next().value();
		
	Cache.clear();

	PreparedCacheEntry=NULL;		
}

///<summary>Creates the key for the CHash list from a QUrl instance.</summary>
///<param name="url">Reference to the QUrl instance to create the key from.</param>
///<returns>Returns a QString instance that can be used as a key inside the QHash list.</returns>
QString CDataCache::KeyFromUrl(QUrl &url)
{
	return url.toString(QUrl::RemoveUserInfo);
}

///<summary>Deletes the least used item from the cache to make room for one new item.</summary>
void CDataCache::SweepCache()
{
	QHashIterator<QString, SCacheEntry*> cacheI(Cache);	
	unsigned int LeastHitCount=(unsigned int)-1;
	QString leastCacheEntryKey;
	
	while (cacheI.hasNext())
	{
		cacheI.next();
		
		if (cacheI.value()->HitCount<LeastHitCount)
		{
			LeastHitCount=cacheI.value()->HitCount;
			leastCacheEntryKey=cacheI.key();
		}
	}
	
	//-- Delete the lest used item from the cache
	delete Cache.take(leastCacheEntryKey);
		
	PreparedCacheEntry=NULL;
}

///<summary>Adds an item to the cache.</summary>
///<param name="url">URL of the item to add.</param>
///<param name="data">Data to add to the cache for this item.</param>
void CDataCache::AddToCache(QUrl url, QByteArray data)
{
	if (data.size() < CACHELIMIT*1024)
	{
		if (Cache.size() >= CACHEBUCKETS)
			SweepCache();
			
		SCacheEntry* cacheEntry=new SCacheEntry;
		if (cacheEntry==NULL) return;
		
		cacheEntry->HitCount=0;
		cacheEntry->Data=data;
		Cache.insert(KeyFromUrl(url), cacheEntry);
	}
}

///<summary>Prepares the data for a cache entry.</summary>
///<param name="url">URL for which the data should be prepared.</param>
///<returns>Returns true on cache hit or false otherwise.</returns>
///<remarks>If this method returns true the data can be collected with a call to Fetch.</remarks>
bool CDataCache::PrepareData(QUrl url)
{
	QString key=KeyFromUrl(url);
	QHash<QString, SCacheEntry*>::const_iterator preparedEntry(Cache.find(key));
	
	if ((preparedEntry!=Cache.end()) && (preparedEntry.key()==key))
	{		
		PreparedCacheEntry=preparedEntry.value();		
		return true;
	}
	else
	{		
		PreparedCacheEntry=NULL;
		return false;
	}
}

///<summary>Returns the prepared cache entry.</summary>
///<returns>Returns the prepared cache entry.</returns>
///<remarks>Only call this method after PrepareData returned true!</remarks>
QByteArray CDataCache::Fetch()
{
	if (PreparedCacheEntry==NULL)
		return QByteArray();
	
	PreparedCacheEntry->HitCount++;
	return PreparedCacheEntry->Data;
}
