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

#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QDir>
#include <QString>
#include <QTextStream>

#include "conf.h"

class CItemStore;
class CRSSItem;

#define NO_LIMIT	366

///<summary>Data store for the items within a feed.</summary>
class CFeedStore
{
	//-- Structs and Enums
	public:
		enum EStoreState {ssSucceeded, ssBaseDirMissing, ssCantCreateStore, ssAlreadyLocked};
		
	//-- Private properties
	private:
		QDir FeedStoreDir;
		QStringListIterator* FeedStoreDirI;
		QString StoreUID;		
		bool Locked;
		bool DeleteOnDelete;
		
	//-- Private methods
	private:
		bool CreateAndCD(QDir& dir, QString dirName);

  //-- Public methods
  public:
		EStoreState Initialize(QUrl url);
    CItemStore* AttachItem(CRSSItem* rssItem);
		void UpdateItem(CRSSItem* rssItem);
		void DeleteItem(CRSSItem* rssItem);
		void ClearItems();
		void Delete();
		QFile* GetReader(QString itemName);
		QFile* GetWriter(QString itemName);
		QUrl GetLocalFileUrl(QString itemName);
		QUrl GetWriterRelUrlForFeedItem(CItemStore* itemStore, QString localName);
		
	//-- Public methods of the item iterator
	public:
		void Reset();
		CRSSItem* NextItem();
    
  //-- c'tor and d'tor
  public:
    CFeedStore();
    CFeedStore(const CFeedStore& source);
    ~CFeedStore();    
};
