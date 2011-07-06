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

#include <QtGui>


#include "onyx/ui/content_view.h"
using namespace ui;

///<summary>Stores all information about a rss feed needed by NewsFlash.</summary>
class CRSSFeedInfo
{
//-- Public properties (stored for this item)
public:
	OData * ListItem;
    QUrl Url;
	QString Title;
	QString Title_for_save;
	int NewItems;
	int ItemCount;
	int KeepDays;
	QString Hint;
	int Threshold;
	bool Debug;
	bool FetchLink;
	bool Marked;
	bool Selected;
	bool SkipNonLocalImages;
	QDateTime LastReadTimestamp;
	QDateTime LastUpdateTimestamp;
	
//-- Public methods
public:
	void Load(QSettings* settings, const QString& section);
	void Save(QSettings* settings);
  
//-- c'tor and d'tor
public:
  CRSSFeedInfo();
  CRSSFeedInfo(QString title, QString url, int newItems, int itemCount, int keepDays, QString hint, int threshold);
  CRSSFeedInfo(const CRSSFeedInfo &source);
  ~CRSSFeedInfo();
  
//-- Public methods
public:
  void AssociateListItem(OData* listItem);
  OData* GetAssociatedListItem();
};

Q_DECLARE_METATYPE(CRSSFeedInfo);
Q_DECLARE_METATYPE(CRSSFeedInfo*);

