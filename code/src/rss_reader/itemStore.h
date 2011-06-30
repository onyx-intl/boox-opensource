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

#include <QString>
#include <QUrl>
#include <QDir>
#include <QDomDocument>

class CSyncHttp;
class CItemStore;
class CFeedStore;

///<summary>Stores an item inside a datastore. An instance of this class is returned by AttachItem</summary>
class CItemStore
{
	//-- Private members
	private:
		QDir BasePath;
		QString StoreUID;
		CFeedStore* ParentFeedStore;
		
  //-- Public methods
  public:
    QUrl StoreIndex(QString docToSave);
		QString GetLocalName(QUrl url);
		QString GetRelPathForLocalName(QDir basePath, QString localName);
		bool StoreElement(QString localName, const QByteArray& data);
		CFeedStore* GetParentFeedStore();
    
  //-- c'tor and d'tor
  public:
    CItemStore(QDir basePath, CFeedStore* parentFeedStore, QString storeUID);
    CItemStore(const CItemStore& source);
    ~CItemStore();    
};
