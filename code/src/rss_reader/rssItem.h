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

#pragma once

#include "sectionTree.h"
#include <QtGui>
#include <QRegExp>
#include <QUrl>

class CHtmlWriter;
class CItemStore;
class CSyncHttp;

class CRSSItem
{
//-- Private structures and enums
private:
	
//-- Public properties
public:
  QString Title;
	QString Link;
	QString LocalLink;
	QString Summary;
	QString Content;
	QString Guid;
	QDateTime Timestamp;
	bool Fetched;
	
//-- Private methods
private:
	int CalcWeight(QString inputString);
	bool HasHint(QString attributes, const QString& hints);
	int CountMatches(const QString &inputString, const QString &regExp);
	int CreateSectionList(CSectionNode* &rootNode, QString& inputString, const QString& hint, int threshold, volatile bool* cancelPtr);
	void WriteNodes(QString &resultPage, QString &inputString, CSectionNode* node, int threshold, int linkThreshold);
	char* parseEncoding(const QByteArray & data);
	
//-- Private properties
private:
	QTextStream* Log;
	volatile bool* CancelPtr;
	
//-- Public methods
public:
	void AddToIndex(CHtmlWriter* indexWriter, const QDateTime& lastReadTimestamp);
	void RemoveByRegex(char* filters[], QString& string);
	bool FetchDetailsLink(CItemStore* itemStore, CSyncHttp* syncHttp, const QString& hint, int threshold, bool skipNonLocalImages, volatile bool* cancelPtr);
	bool FetchSummary(CItemStore* itemStore, CSyncHttp* syncHttp, bool skipNonLocalImages, volatile bool* cancelPtr);
	bool FetchImages(QUrl baseUrl, CItemStore* itemStore, CSyncHttp* syncHttp, QString& page, bool storeAbsolute, bool skipNonLocalImages, volatile bool* cancelPtr);
	void Serialize(QIODevice* dstDevice);
	bool IsInTime(int keepDays);
	void SetLog(QTextStream* log);
	int GetDayId();
	QString ConvertToUnicode(QByteArray data);
  
//-- c'tor and d'tor
public:
  CRSSItem();
	CRSSItem(QIODevice* srcDevice);
	
//-- Disallow copying
private:
  CRSSItem(const CRSSItem& source) {}
};
