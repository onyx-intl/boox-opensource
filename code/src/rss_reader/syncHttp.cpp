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

#include <QNetworkRequest>
#include <QEventLoop>
#include <memory>
#include "exitCodes.h"
#include "rssLoaderThread.h"
#include "dataCache.h"

#include "syncHttp.h"

///<summary>c'tor</summary>
///<param name="parent">Pointer to a CRSSLoaderThread instance that is used for suspending the execution via WaitForExit.</param>
CSyncHttp::CSyncHttp(CRSSLoaderThread* parent) :
  QObject(parent),
	Loop(this)
{
  Manager=new QNetworkAccessManager(this);
	
  connect(Manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(OnFinished(QNetworkReply*)));
}
  
///<summary>d'tor</summary>
CSyncHttp::~CSyncHttp()
{
}

///<summary>Slot to call if the Request is finished</summary>
///<param name="reply">Pointer to a NetworkReply containing the retrieved data.</param>
void CSyncHttp::OnFinished(QNetworkReply* reply)
{
	((QThread*)parent())->exit(EXIT_SYNCHTTP);
}

///<summary>Fetches the supplied URL synchronously</summary>
///<param name="url">Reference to the URL that has to be fetched. It may be changed if there was a redirect returned by the original URL.</param>
///<returns>Returns the gotten content as a QByteArray</returns>
QByteArray CSyncHttp::Get(QUrl& url)
{
	QNetworkRequest request;
	std::auto_ptr<QNetworkReply> reply;
	unsigned char retry=5;

	do
	{
		//-- If the data was previously cached, return the cached data and continue
		if (Cache.PrepareData(url))
		{
			LastError=QNetworkReply::NoError;
			LastErrorMessage="";
			return Cache.Fetch();		
		}

		//-- Construct request
		request.setUrl(url);
		request.setRawHeader("User-Agent", "Flash Systems NewsFlash");

		//-- Submit get-request
		reply = std::auto_ptr<QNetworkReply>(Manager->get(request));
		
		//-- Start a loop to handle the Finish-Slot
		((CRSSLoaderThread*)parent())->WaitForExit(EXIT_SYNCHTTP);
		
		//-- If the destination was redirected, follow the redirect
		switch (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt())
		{
			case 301:
			case 302:
			case 303:
			case 307:
				url=reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
				retry--;
				break;
				
			//-- Eny other value will leave the loop and process the error message if neccessary
			default:
				retry=(unsigned char)0xFF;
		}
	}
	while ((retry<5));	//Loop as long as retry is <5. This happens if everything is ok or if the unsigned value rolls over after x tries
	
	//-- Throw an exception if the download failed
	LastError=reply->error();
	LastErrorMessage=reply->errorString();
	
	//-- Store the reply data because readAll can only be called once
	QByteArray replyData=reply->readAll();
	
	//-- If the operation succeeded, store the reply data into the cache
	if (LastError==QNetworkReply::NoError)
		Cache.AddToCache(url, replyData);
	
	//-- Return all Data
	return replyData;
}

///<summary>Returns the error code for the last Get operation</summary>
///<returns>QNetworkReply::NetworkError variable with the error code from the last Get operation</returns>
QNetworkReply::NetworkError CSyncHttp::GetLastError()
{
	return LastError;
}

///<summary>Returns the error message for the last Get operation</summary>
///<returns>QString instance with the error message from the last Get operation</returns>
QString CSyncHttp::GetLastErrorMessage()
{
	return LastErrorMessage;
}
