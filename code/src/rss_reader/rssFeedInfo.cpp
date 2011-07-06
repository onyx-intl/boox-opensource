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

#include "rssFeedInfo.h"
#include "onyx/ui/content_view.h"

///<summary>Default constructor</summary>
///<remarks>This is neccessary to allow registration via qRegisterMetaType</remarks>
CRSSFeedInfo::CRSSFeedInfo() :
	ListItem(NULL),
	Url(),
	Title("Empty"),
	NewItems(0),
	ItemCount(0),
	KeepDays(0),
	Hint(""),
	Threshold(0),
	Debug(false),
	FetchLink(true),
	Marked(false),
	Selected(false),
	SkipNonLocalImages(false),
	LastReadTimestamp(QDate(2000, 1, 1), QTime(0, 0, 0), Qt::UTC),
	LastUpdateTimestamp(QDate(2000, 1, 1), QTime(0, 0, 0), Qt::UTC)
{
}

///<summary>Constructor</summary>
///<param name="listItem">The list item for this thread (may not be used outside the GUI thread)</param>
///<param name="url">The URL for the feed to download</param>
///<remarks>The "new CFeedStore" construct is necessary because FeedStore must be NULLable. This is due to the necessary default constructor</remarks>
CRSSFeedInfo::CRSSFeedInfo(QString title, QString url, int newItems, int itemCount, int keepDays, QString hint, int threshold) :
	ListItem(NULL),
	Url(url),
	Title(title),
	NewItems(newItems),
	ItemCount(itemCount),
	KeepDays(keepDays),
	Hint(hint),
	Threshold(threshold),
	Debug(false),
	FetchLink(true),
	Marked(false),
	Selected(false),
	SkipNonLocalImages(false),
	LastReadTimestamp(QDate(2000, 1, 1), QTime(0, 0, 0), Qt::UTC),
	LastUpdateTimestamp(QDate(2000, 1, 1), QTime(0, 0, 0), Qt::UTC)
{
}

///<summary>Copy constructor</summary>
///<param name="source">Source instance to copy</param>
///<remarks>The "new CFeedStore" construct is necessary because FeedStore must be NULLable. This is due to the necessary default constructor</remarks>
CRSSFeedInfo::CRSSFeedInfo(const CRSSFeedInfo &source) :
	ListItem(source.ListItem),
	Url(source.Url),
	Title(source.Title),
	Title_for_save(source.Title_for_save),
	NewItems(source.NewItems),
	ItemCount(source.ItemCount),
	KeepDays(source.KeepDays),
	Hint(source.Hint),
	Threshold(source.Threshold),
	Debug(source.Debug),
	FetchLink(source.FetchLink),
	Marked(source.Marked),
	Selected(source.Selected),
	SkipNonLocalImages(source.SkipNonLocalImages),
	LastReadTimestamp(source.LastReadTimestamp),
	LastUpdateTimestamp(source.LastUpdateTimestamp)
{
}

///<summary>Destructor</summary>
CRSSFeedInfo::~CRSSFeedInfo()
{
}

///<summary>Loads the feed info from the supplied QSettings instance.</summary>
///<param name="settings">Pointer to a QSettings instance containing the information for this feed.</param>
///<param name="section">The name of the section within the supplied QSettings instance containing the information for this feed.</param>
void CRSSFeedInfo::Load(QSettings* settings, const QString& section)
{
    //very weird
	// Title=section;
    Title_for_save = section;
    Title = QUrl::fromPercentEncoding (section.toAscii());
	
    settings->beginGroup(section);
    Url=QUrl(settings->value("url").toString());
    KeepDays=settings->value("keep", 3).toInt();
    Hint=settings->value("hint", "content, indivbody, article").toString();
    Threshold=settings->value("threshold", 35).toInt();
    FetchLink=settings->value("fetchlink", 1).toInt();
    Debug=(settings->value("debug", 0).toInt()>0);
    NewItems=settings->value("new", 0).toInt();
    ItemCount=settings->value("count", 0).toInt();
    Marked=(settings->value("marked", 0).toInt()>0);
    Selected=(settings->value("selected", 0).toInt()>0);
    SkipNonLocalImages=(settings->value("allimages", 0).toInt()==0);
    LastReadTimestamp=QDateTime::fromTime_t(settings->value("readtimestamp", 0).toInt());
    LastUpdateTimestamp=QDateTime::fromTime_t(settings->value("updatetimestamp", 0).toInt());
    settings->endGroup();
}

///<summary>Updates the feed information within the supplied QSettings instance.</summary>
///<param name="settings">Pointer to the QSettings instance that should be updated with the current values from the feed info.</param>
void CRSSFeedInfo::Save(QSettings* settings)
{
    settings->beginGroup(Title_for_save);
    settings->setValue("url", Url.toString());
    settings->setValue("new", NewItems);
    settings->setValue("count", ItemCount);
    settings->setValue("marked", Marked?1:0);
    settings->setValue("selected", Selected?1:0);
    settings->setValue("readtimestamp", LastReadTimestamp.toTime_t());
    settings->setValue("updatetimestamp", LastUpdateTimestamp.toTime_t());
    settings->endGroup();
}

///<summary>Associate a list item with this FeedInfo-instance.</summary>
///<param name="listItem">Pointer to a OData instance to associate with this feed info class.</param>
///<remarks>The item can be retrieved by calling GetAssociatedListItem. This can only be done from the thread the list item is associated with.</remarks>
void CRSSFeedInfo::AssociateListItem(OData* listItem)
{
	ListItem=listItem;
}

///<summary>Returns the list item associated with this feed.</summary>
///<returns>Returns a pointer to the list item associeated with this feed.</returns>
///<remarks>Can only be called by the read the list item is associated with.</remarks>
OData * CRSSFeedInfo::GetAssociatedListItem()
{
	if (ListItem!=NULL)
	{
		// Q_ASSERT(QThread::currentThreadId() == ListItem->thread()->currentThreadId());
		return ListItem;
	}
	else
		return NULL;
}
