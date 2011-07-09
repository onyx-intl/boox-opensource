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

#include "dataCache.h"
#include <QtCore>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QByteArray>

class CRSSLoaderThread;	//This is neccessary to avoid some nasty circular references

///<summary>Allows synchronous downloading of data despite the asyncrhonous nature of QNetworkAccessManager.</summary>
class CSyncHttp :
  QObject
{
  Q_OBJECT
  
//-- Slots
public slots:
  void OnFinished(QNetworkReply* reply);
  
//-- Private properties
private:
	CDataCache Cache;
  QNetworkAccessManager* Manager;
	QEventLoop Loop;
	QNetworkReply::NetworkError LastError;
	QString LastErrorMessage;

//-- c'tor and d'tor
public:
	CSyncHttp(CRSSLoaderThread* parent);
  ~CSyncHttp();
  
//-- Public Methods
public:
  QByteArray Get(QUrl& url);
	QNetworkReply::NetworkError GetLastError();
	QString GetLastErrorMessage();
};
