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

#include "locking.h"

#include "itemStore.h"
#include "feedStore.h"
#include "syncHttp.h"

///<summary>c'tor used by CFeedStore.</summary>
///<param name="basePath">Base path of the data store.</param>
///<param name="parentFeedStore">Pointer to the CFeedStore instance that owns this item store.</param>
///<param name="storeUID">The UID of the data store to aquire a lock for the store within the item. This is for reference counting.</param>
CItemStore::CItemStore(QDir basePath, CFeedStore* parentFeedStore, QString storeUID) :
	BasePath(basePath),
	StoreUID(storeUID),
	ParentFeedStore(parentFeedStore)
{
	CLock::Lock("StoreLock", StoreUID);
}

///<summary>Copy c'tor</summary>
CItemStore::CItemStore(const CItemStore& source) :
	BasePath(source.BasePath),
	StoreUID(source.StoreUID),
	ParentFeedStore(source.ParentFeedStore)
{
	CLock::Lock("StoreLock", StoreUID);
}

///<summary>d'tor</summary>
CItemStore::~CItemStore()
{
	CLock::Unlock("StoreLock", StoreUID);
}

///<summary>Stores an index.html file into the item store. This is the main file for this item.</summary>
///<param name="docToSave">QString instance containing the html document to store into the index.html file.</param>
///<returns>Returns a QUrl instance that contains a local URL for the index.html file.</returns>
QUrl CItemStore::StoreIndex(QString docToSave)
{
	QFile outFile(BasePath.absoluteFilePath("index.html"));
	if (outFile.open(QIODevice::ReadWrite|QIODevice::Truncate))
	{
		outFile.write(docToSave.toUtf8());
		outFile.close();
	}//!
	
	return ParentFeedStore->GetWriterRelUrlForFeedItem(this, "index.html");
}

///<summary>Returns a local file name for a URL</summary>
///<param name="url">A QUrl instance containing the internet URL for the file.</param>
///<returns>Returns a string containing the local file name that will be used for storing the content of the supplied URL.</returns>
QString CItemStore::GetLocalName(QUrl url)
{
	QString localFileName;

	//-- Treat the remote url path part as 
	QFileInfo remoteFileInfo(url.toString(QUrl::RemoveScheme|QUrl::RemoveAuthority|QUrl::RemoveUserInfo|QUrl::RemoveQuery|QUrl::StripTrailingSlash));
	
	localFileName=QCryptographicHash::hash(remoteFileInfo.filePath().toUtf8(), QCryptographicHash::Md5).toHex();
	localFileName.append(".");
	localFileName.append(remoteFileInfo.suffix());
	
	return localFileName;
}

///<summary>Gets the relative path of the localName in relation to the supplied basePath.</summary>
///<param name="basePath">QDir instance containing the base path.</param>
///<param name="localName">Local name of the file as supplied by GetLocalName.</param>
///<returns>Returns the relative path necessary to access localName from within a html file stored in basePath</returns>
QString CItemStore::GetRelPathForLocalName(QDir basePath, QString localName)
{
	return basePath.relativeFilePath(BasePath.absoluteFilePath(localName));
}

///<summary>Returns the parent CFeedStore instance of this CItemStore instance.</summary>
///<returns>Returns the CFeedStore instance supplied in the parentFeedStore-parameter of the constructor.</returns>
CFeedStore* CItemStore::GetParentFeedStore()
{
	return ParentFeedStore;
}

///<summary>Stores the supplied QByteArray instance into this item store.</summary>
///<param name="localName">Local name to use for this item inside the store. Use GetLocalName to construct a unique name from a URL.</param>
///<param name="data">Reference to a QByteArray instance containing the data to store inside the item store.</param>
///<returns>Returns true if the data was stored successfully or false if storing faild.</returns>
bool CItemStore::StoreElement(QString localName, const QByteArray& data)
{
	bool result=true;
	QFile outFile(BasePath.absoluteFilePath(localName));
	if ((result=outFile.open(QIODevice::ReadWrite|QIODevice::Truncate))==true)
	{
		result=(outFile.write(data)==-1);	
		outFile.close();
	}
	
	return result;
}
