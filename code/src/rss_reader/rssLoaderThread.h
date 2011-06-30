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
#include "status.h"
#include "rssFeedInfo.h"

class CRSSFeedInfo;

///<summary>Thread that coordinates all of the downloading and feed updating via a global queue.</summary>
class CRSSLoaderThread : public QThread
{
  Q_OBJECT
  
//-- Private properties
private:
      QQueue<CRSSFeedInfo> FeedList;
      volatile bool Busy;
  
//-- Slots
public slots:
    void OnUpdateFeed(CRSSFeedInfo feed);

//-- Signals
signals:
    void sendStatus(CRSSFeedInfo feed, CStatus status);
    void sendDone();

//-- Public properties
public:
    volatile bool Cancel;

//-- Public methods
public:
    bool IsBusy();
	virtual void WaitForExit(int exitCode);

//-- Private methods
private:
    void Working();
    void UpdateFeed(CRSSFeedInfo& feed);
    void UpdateFeeds();
    void run();
  
//-- c'tor and d'tor
public:
    CRSSLoaderThread();
    ~CRSSLoaderThread();
};
