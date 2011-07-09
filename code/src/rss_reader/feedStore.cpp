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

#include "itemStore.h"
#include "rssItem.h"
#include "locking.h"

#include "feedStore.h"

///<summary>C'tor</summary>
CFeedStore::CFeedStore() :
	Locked(false),
	DeleteOnDelete(false),
	FeedStoreDirI(NULL)
{
}

///<summary>Copy c'tor</summary>
///<param name="source">Reference to the source instance to copy</param>
CFeedStore::CFeedStore(const CFeedStore& source) :
	FeedStoreDir(source.FeedStoreDir),
	Locked(true),
	DeleteOnDelete(false),
	StoreUID(source.StoreUID),
	FeedStoreDirI(NULL)
{
	CLock::Lock("StoreLock", StoreUID);
}

///<summary>D'tor</summary>
CFeedStore::~CFeedStore()
{
	//-- If the Delete() method was called, delete the feedStoreDir now
	if (DeleteOnDelete)
	{
		//-- Delete all items
		ClearItems();
		
		//-- Now remove the feed store dir itself
		FeedStoreDir.cdUp();
		FeedStoreDir.rmdir(StoreUID);
	}
	
	//-- Locked makes sure that only after successfully calling init or copying the instance an Unlock call is happening
	if (Locked)
		CLock::Unlock("StoreLock", StoreUID);
		
	if (FeedStoreDirI)
		delete FeedStoreDirI;
}

///<summary>Changes directory and creates the directory if neccessary.</summary>
///<param name="dir">Reference to the QDir-Instance that should be cd'd.</param>
///<param name="dirName">The name of the Subdirectory.</param>
///<returns>Returns true if the cd was successfull or false if the creation of the subdirectory failed.</returns>
bool CFeedStore::CreateAndCD(QDir& dir, QString dirName)
{
	if (!dir.cd(dirName))
	{
		if (!dir.mkdir(dirName))
			return false;
		dir.cd(dirName);
	}
	
	return true;
}

///<summary>Initializes the CFeedStore instance and creates the storage directories if neccessary.</summary>
///<param name="url">An QUrl instance that specifies the URL of the feed to store within this feed store.</param>
///<returns>Returns ssSucceeded on success or one of the error codes within enum EStoreState.</returns>
CFeedStore::EStoreState CFeedStore::Initialize(QUrl url)
{
	//-- Hash the url, it will be used as the subdir name and UID of the store
	StoreUID=QCryptographicHash::hash(url.toString(QUrl::RemoveScheme|QUrl::RemoveUserInfo|QUrl::StripTrailingSlash).toUtf8(), QCryptographicHash::Md5).toHex();
	
	//-- Check if this store is already locked. If this is the case, it can't be initialized twice
	if (CLock::IsLocked("StoreLock", StoreUID))
		return ssAlreadyLocked;
		
	//-- Create the base dir for the store
	FeedStoreDir.setPath(MyConf::getStoreDir());
	if (!FeedStoreDir.exists())
    {
        FeedStoreDir.mkpath(MyConf::getStoreDir());
    }

	if (!FeedStoreDir.exists())
		return ssBaseDirMissing;
	if (!CreateAndCD(FeedStoreDir, DATASTOREDIR))
		return ssCantCreateStore;
	
	if (!CreateAndCD(FeedStoreDir, StoreUID))
		return ssCantCreateStore;
	
	//-- Now mark the feed Store directory as locked so calls to delete, etc. will fail until it is
	//   unlocked.
	CLock::Lock("StoreLock", StoreUID);
	Locked=true;
	
	return ssSucceeded;
}

///<summary>Attaches a CRSSItem-instance to the data store.</summary>
///<param name="rssItem">The CRSSItem-instance that should be attached to the data store.</param>
///<returns>Returns a CItemStore-instance that must be used to store data for this feed. The caller is responsible for deleting the returned instance.</returns>
///<remarks>CFeedStore does not take ownership of the passed rssItem-instance. It must be deleted by the caller.
///         This function saves the item information in a .item file within the data store directory.</remarks>
CItemStore* CFeedStore::AttachItem(CRSSItem* rssItem)
{
	QDir itemStoreDir(FeedStoreDir);
	QString ItemUID;
	
	//-- Hash the url GUID of the rssItem and use it for creating and locking the ItemUID
	ItemUID=QCryptographicHash::hash(rssItem->Guid.toUtf8(), QCryptographicHash::Md5).toHex();
	if (!CreateAndCD(itemStoreDir, ItemUID))
		return NULL;
		
	//-- Save the item data for now
	UpdateItem(rssItem);
	
	return new CItemStore(itemStoreDir, this, StoreUID);
}

///<summary>Updates the meta information stored fore this item.</summary>
///<param name="rssItem">A CRSSItem instance contining the item to update the information for.</param>
///<remarks>Will call Serialize on the supplied item to store all neccessary information into the corresponding .item file.</remarks>
void CFeedStore::UpdateItem(CRSSItem* rssItem)
{
	QString ItemUID=QCryptographicHash::hash(rssItem->Guid.toUtf8(), QCryptographicHash::Md5).toHex();

	//-- Serialize this item into a store file
	QFile outFile(FeedStoreDir.absoluteFilePath(ItemUID+".item"));
	outFile.open(QIODevice::ReadWrite|QIODevice::Truncate);
	rssItem->Serialize(&outFile);
	outFile.close();
}

///<summary>Deletes an item from the store.</summary>
///<param name="rssItem">Pointer to the CRSSItem instance that should be deleted from the cache.</param>
void CFeedStore::DeleteItem(CRSSItem* rssItem)
{
	QDir itemStoreDir(FeedStoreDir);
	QString ItemUID=QCryptographicHash::hash(rssItem->Guid.toUtf8(), QCryptographicHash::Md5).toHex();

	if (CreateAndCD(itemStoreDir, ItemUID))
	{
		QStringListIterator itemStoreI=itemStoreDir.entryList(QDir::Files|QDir::NoDotAndDotDot);
		while (itemStoreI.hasNext())
			itemStoreDir.remove(itemStoreI.next());
	}
	
	//-- Delete the item-file only if the store dir could be deleted
	if (FeedStoreDir.rmdir(ItemUID))
		FeedStoreDir.remove(ItemUID+".item");
}

///<summary>Deletes all items from the data store.</summary>
void CFeedStore::ClearItems()
{
	CRSSItem* rssItem;

	Reset();
	while ((rssItem=NextItem())!=NULL)
		DeleteItem(rssItem);

	//-- Finally delete everything in the root of the feedstore
	QStringListIterator feedStoreI=FeedStoreDir.entryList(QDir::Files|QDir::NoDotAndDotDot);
	while (feedStoreI.hasNext())
		FeedStoreDir.remove(feedStoreI.next());
}

///<summary>Deletes the whole feed store from disk.</summary>
///<remarks>The store is deleted within the d'tor of this class. This way the store can be used normally until it's deleted.</remarks>
void CFeedStore::Delete()
{
	//-- Only allow deletion if the feed store was successfully locked
	if (Locked)
		DeleteOnDelete=true;
}

///<summary>Creates a URL to access an item stored inside an item store from a file created with GetWriter.</summary>
///<param name="itemStore">Pointer to the CItemStore instance containing the item.</param>
///<param name="localName">Local name of the file to access.</param>
///<returns>Returns the URL to insert into the feed store.</returns>
QUrl CFeedStore::GetWriterRelUrlForFeedItem(CItemStore* itemStore, QString localName)
{
	return QUrl::fromLocalFile(itemStore->GetRelPathForLocalName(FeedStoreDir, localName));
}

///<summary>Gets the URL for a local file stored within this data store.</summary>
///<param name="itemName">String with the name of the item to get the local URL for.</param>
///<returns>Returns the QUrl for the given item within the store.</returns>
QUrl CFeedStore::GetLocalFileUrl(QString itemName)
{
	return QUrl::fromLocalFile(FeedStoreDir.absoluteFilePath(itemName));
}

///<summary>Gets a pointer to a QFile instance for the specified item within the store.</summary>
///<param name="itemName">String with the name of the item to get the QFile instance for.</param>
///<returns>Returns a QFile instance for the given file. This instance is readonly! It is the responsibility of the caller to free the QFile instance.</returns>
QFile* CFeedStore::GetReader(QString itemName)
{
	QFile* outFile=new QFile(FeedStoreDir.absoluteFilePath(itemName));
	outFile->open(QIODevice::ReadOnly);
	return outFile;
}

///<summary>Gets a pointer to a QFile instance for the specified item within the store.</summary>
///<param name="itemName">String with the name of the item to get the QFile instance for.</param>
///<returns>Returns a QFile instance for the given file. This instance is read-/write enabled! It is the responsibility of the caller to free the QFile instance.</returns>
QFile* CFeedStore::GetWriter(QString itemName)
{
	QFile* outFile=new QFile(FeedStoreDir.absoluteFilePath(itemName));
	outFile->open(QIODevice::ReadWrite|QIODevice::Truncate);
	return outFile;
}

///<summary>Resets the feedstore iterator.</summary>
///<remarks>After reset NextItem begins again to return the first item.</remarks>
void CFeedStore::Reset()
{
	if (FeedStoreDirI)
		delete FeedStoreDirI;
	FeedStoreDirI=new QStringListIterator(FeedStoreDir.entryList(QStringList("*.item"), QDir::Files|QDir::NoSymLinks|QDir::NoDotAndDotDot|QDir::Readable));
	FeedStoreDirI->toFront();
}

///<summary>Retrieves the next item within the store.</summary>
///<returns>Returns the CRSSItem instance for the next feed item. It is the responsibility of the caller to free the returned instance.</returns>
CRSSItem* CFeedStore::NextItem()
{
	CRSSItem* rssItem=NULL;
	
	if (FeedStoreDirI==NULL)
		Reset();
	
	if (FeedStoreDirI->hasNext())
	{
		QFile inFile(FeedStoreDir.absoluteFilePath(FeedStoreDirI->next()));
		inFile.open(QIODevice::ReadOnly);
		rssItem=new CRSSItem(&inFile);
		inFile.close();
	}
	
	return rssItem;
}
