/*	NewsFlash
		Copyright 2010 Daniel Go� (Flash Systems)

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

#include <QMap>
#include <memory>
#include "syncHttp.h"
#include "exitCodes.h"
#include "rssFeed.h"
#include "htmlWriter.h"
#include "rssFeedInfo.h"
#include "feedStore.h"
#include "itemStore.h"
#include "rssItem.h"

#include "rssLoaderThread.h"
#include "onyx/sys/sys_status.h"

//-- Defines a helper macro for loging
#define LOG(__line) if (log.get()) (*log) << __line << "\r\n";

using namespace std;

///<summary>Konstruktor</summary>
CRSSLoaderThread::CRSSLoaderThread()
{
    Cancel=false;
    Busy=false;
}

///<summary>Destructor</summary>
CRSSLoaderThread::~CRSSLoaderThread()
{
}

///<summary>Pumps events until exit is left with a given result code.</summary>
///<param name="exitCode">The exit code that must be returned by exec() to leave this method.</param>
///<remarks>This is used for waiting for a given event from within CSyncHttp and CRSSLoaderThread</remarks>
void CRSSLoaderThread::WaitForExit(int exitCode)
{
	while (exec()!=exitCode);
}

///<summary>Update the supplied feed.</summary>
///<param name="feed">Reference to a CRSSFeedInfo instance specifying the feed to update.</param>
void CRSSLoaderThread::UpdateFeed(CRSSFeedInfo& feed)
{
    CSyncHttp httpLoader(this);
    QByteArray httpData;
    CRSSItem* rssItem;
    QMap<QDateTime, CRSSItem*> itemList;
    QString rssParseError;
    QList<QString> guidList;
    auto_ptr<QTextStream> log;
    auto_ptr<QFile> logfile;
    int iItemIndex;
    int itemCount=0;
    int newItemCount=0;
    int currentDayId=-1;

    //-- Prevent idling
    Working();

    //-- New context to delete CStatus after using
    {	CStatus newStatus("Loading stored items...", 0); emit sendStatus(feed, newStatus);	}

    //-- Create the feed store and load the already stored items
    CFeedStore feedStore;
    switch (feedStore.Initialize(feed.Url))
    {
        case CFeedStore::ssBaseDirMissing:
            {
                CStatus newStatus("Cache base dir missing!", 0, CStatus::sfFailed);
                emit sendStatus(feed, newStatus);
            }
            return;
        case CFeedStore::ssCantCreateStore:
            {
                CStatus newStatus("Creation of feed dir failed!", 0, CStatus::sfFailed);
                emit sendStatus(feed, newStatus);
            }
            return;
        case CFeedStore::ssAlreadyLocked:
            {
                CStatus newStatus("Internal error. Please contact developer.", 0, CStatus::sfFailed);
                emit sendStatus(feed, newStatus);
            }
            return;
    }

    //-- If debuging is enabled, create a log-file as a textstream	
    if (feed.Debug)
    {
        logfile=auto_ptr<QFile>(feedStore.GetWriter("fetch.log"));
        log=auto_ptr<QTextStream>(new QTextStream(logfile.get()));
    }

    //-- Load the already stored feeds from disk
    feedStore.Reset();
    while ((rssItem=feedStore.NextItem())!=NULL)
    {
        guidList+=rssItem->Guid;
        rssItem->SetLog(log.get());
        itemList.insert(rssItem->Timestamp, rssItem);	

        LOG("Loaded " << rssItem->Title << " from store.");
    }

    //-- New context to delete CStatus after using
    {	CStatus newStatus("Downloading feed...", 5); emit sendStatus(feed, newStatus);	}	
    Working();

    //-- Load an Parse the RSS-feed
    QUrl url(feed.Url);
    LOG("Downloading feed from " << url.toString());
    httpData=httpLoader.Get(url);

    if (httpLoader.GetLastError()!=QNetworkReply::NoError)
    {
        CStatus newStatus(httpLoader.GetLastErrorMessage(), 0, CStatus::sfFailed);
        emit sendStatus(feed, newStatus);
        LOG("HTTP Error: " << httpLoader.GetLastErrorMessage());
        return;
    }

    { CStatus newStatus("Parsing feed...", 10); emit sendStatus(feed, newStatus); }

    //-- Now parse the feed and add new instances to the list
    CRSSFeed rssFeed(httpData);
    rssFeed.SetLog(log.get());
    LOG("RSS elements:");

    while ((rssItem=rssFeed.NextItem())!=NULL)
    {
        LOG("Element\r\n  Title: " << rssItem->Title << "\r\n  Link: " << rssItem->Link  << "\r\n  Guid: " << rssItem->Guid << "\r\n  Timestamp: " << rssItem->Timestamp.toString(Qt::ISODate));
        Working();

        //-- Add only elements to the list if the GUID is not already in the list
        if (!guidList.contains(rssItem->Guid))
        {
            guidList+=rssItem->Guid;
            itemList.insert(rssItem->Timestamp, rssItem);
        }
        else
            delete rssItem;

        //-- Allow cancel
        if (Cancel)
            break;
    }

    //-- Save the parse error if one occured
    rssParseError=rssFeed.GetErrorMessage();

    //-- Create the output-file for the index page
    CHtmlWriter indexWriter(feedStore.GetWriter("index.html"), ":/summaryHeader.txt", "onkeyup=\"KeyEvent(event)\" onkeydown=\"KeyDownEvent(event)\"", rssFeed.GetTitle());

    //-- Now iterate over all items, build an index page and download the links
    QMapIterator<QDateTime, CRSSItem*> itemListIterator(itemList);
    iItemIndex=0;
    itemListIterator.toBack();
    while (itemListIterator.hasPrevious())
    {
        iItemIndex++;
        Working();

        //-- Get the next item
        itemListIterator.previous();
        rssItem=itemListIterator.value();
        if(feed.KeepDays == 0)
        {
            feed.KeepDays = 3;
        }

        if (rssItem->IsInTime(feed.KeepDays))
        {			
            //-- If cancel is set, do nothing, just delete everything
            if (!Cancel)
            {
                //-- Download and store only if the item is not already marked as fetched (loaded from store)
                if (!rssItem->Fetched)
                {
                    { CStatus newStatus("Downloading items...", (iItemIndex*70)/itemList.count()+20); emit sendStatus(feed, newStatus); }

                    //-- Open an item store and download the linked page into it
                    //   add the item only to the index if the itemStore could be created successfully
                    auto_ptr<CItemStore> itemStore(feedStore.AttachItem(rssItem));
                    if (itemStore.get())
                    {
                        //-- Fetch the data and images for the summary attribute
                        rssItem->FetchSummary(itemStore.get(), &httpLoader, feed.SkipNonLocalImages, &Cancel);

                        //-- Fetch the details link, if FetchLink is true (normaly it should be)
                        if (feed.FetchLink)
                            rssItem->FetchDetailsLink(itemStore.get(), &httpLoader, feed.Hint, feed.Threshold, feed.SkipNonLocalImages, &Cancel);
                        else
                            rssItem->Fetched=true;

                        //-- Save the updated item information
                        feedStore.UpdateItem(rssItem);
                    }
                }
            }

            //-- Call add to index after fetching the details so LocalLink is valid
            //   If we're canceling add the already fetched items to the index, so they are still available
            if ((!Cancel)||(rssItem->Fetched))
            {
                if(rssItem->Timestamp.toString() == "")
                {
                    rssItem->Timestamp = QDateTime::currentDateTime();
                }
                //-- Insert a divider between days
                if (rssItem->GetDayId()!=currentDayId)
                {
                    currentDayId=rssItem->GetDayId();
                    indexWriter.Write("<DIV CLASS=\"nfday\">" + rssItem->Timestamp.toLocalTime().date().toString(Qt::SystemLocaleLongDate) + "</DIV><BR/>");
                }
                rssItem->AddToIndex(&indexWriter, feed.LastReadTimestamp);

                //-- Increase the item count and increase the new item count if the item's timestamp is greater then the "last read" timestamp
                itemCount++;

                if (rssItem->Timestamp>=feed.LastReadTimestamp)
                    newItemCount++;
            }
        }
        else
        {
            //-- The item is not in the valid time range but was already fetched,
            //   remmove it from the data store
            if (rssItem->Fetched)
                feedStore.DeleteItem(rssItem);
        }

        delete rssItem;
    }

    //-- Show Ready or the error Message
    if (rssParseError.length()>0)
    { CStatus newStatus(rssParseError, 100, newItemCount, itemCount, (CStatus::EStateFlags)(CStatus::sfFinished|CStatus::sfFailed)); emit sendStatus(feed, newStatus); }
    else
    { CStatus newStatus("Ready", 100, newItemCount, itemCount, CStatus::sfFinished); emit sendStatus(feed, newStatus); }
}

///<summary>Updates all Feed in the FeedList</summary>
void CRSSLoaderThread::UpdateFeeds()
{
    CRSSFeedInfo feed;

    Busy=true;
    SysStatus::instance().enableIdle(false);
    //-- Now execute the UpdateFeed-Procedure for every feed in the list
    while (!FeedList.isEmpty())
    {
        feed=FeedList.dequeue();
        if (Cancel)
        {
            CStatus newStatus("Cancel", 100, 0, 0, CStatus::sfFinished); emit sendStatus(feed, newStatus);
            continue;
        }

        UpdateFeed(feed);
    }	

    emit sendDone();
    SysStatus::instance().enableIdle(true);
    Busy=false;
    Cancel=false;
}

///<summary>Worker method.</summary>
void CRSSLoaderThread::run()
{
    //-- If new feed info is added, OnUpdateFeed exits with 1 so the
    //   message prcessing can start.
    while (exec() == EXIT_NEWLISTITEM)
    {
        if (!FeedList.isEmpty())
            UpdateFeeds();
    }
}

///<summary>Returns true if the loader thread is currently executing downloads.</summary>
///<returns>Returns true if the loader thread is busy at the moment or falls if it is idle.</returns>
bool CRSSLoaderThread::IsBusy()
{
    return Busy;
}

///<summary>Slot that is asynchronously called by the main window to add a feed to the download queue.</summary>
///<param name="feed">CRSSFeedInstance with the feed information for the download to add.</param>
///<remarks>If the feed is not busy at the moment the download ist started emediately and the busy flag is asserted. If the feed is busy the download will be added to the end of the queue.</remarks>
void CRSSLoaderThread::OnUpdateFeed(CRSSFeedInfo feed)
{
    //-- Enqeue the new feed
    FeedList.enqueue(feed);

    //-- If we're not busy, exit the exec-Function in run() to update the feed list
    //   We can't hang here, because there is no other exec between busy=false and
    //   the main exec() call. So nobody can catch away this exec, even if we're
    //   not already within the exit-function.
    if (!Busy)
        exit(EXIT_NEWLISTITEM);
}

void CRSSLoaderThread::Working()
{
    SysStatus::instance().resetIdle();
}
