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

#include <QtCore>
#include <QByteArray>
#include <QXmlStreamReader>

class CRSSItem;

///<summary>Reperesents a rss feed and supplies methods for parsing the xml information.</summary>
class CRSSFeed
{
//-- Private structures and enums
private:
	enum ETagType {ettUnkown, ettItem, ettTitle, ettLink, ettSummary, ettContent, ettGuid, ettDate};
	struct SItemMapper
	{
		QString Tag;
		ETagType Type;
	};
	QString ErrorMessage;
	
//-- Private properties
private:
	QXmlStreamReader Xml;
	QString Title;
	int StreamLength;
	QTextStream* Log;
	
//-- Public properties
public:
	CRSSItem* NextItem();
	uint GetPosition(uint max);
	QString GetTitle();
	void SetLog(QTextStream* log);
	bool Error();
	QString GetErrorMessage();
	
//-- c'tor and d'tor
public:
  CRSSFeed(const QByteArray& rssStream);
  ~CRSSFeed();
};
