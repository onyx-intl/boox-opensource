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

///<summary>Represents a status update that is sent from CRSSLoaderThread to the main window via the OnStatus slot.</summary>
class CStatus
{
//-- Public enums and structs
public:
	enum EStateFlags {sfNone=0x00, sfFailed=0x01, sfCountUpdate=0x02, sfFinished=0x04};
	
//-- Private properties
private:
    QString Status;
    int Completion;
    int NewItems;
    int ItemCount;
    EStateFlags Flags;
  
//-- c'tor and d'tor
public:
    CStatus();
    CStatus(const QString status, int completion, EStateFlags flags=sfNone);
    CStatus(const QString status, int completion, int newItems, int itemCount, EStateFlags flags=sfNone);
    CStatus(const CStatus& source);
  
//-- Public methods
public:
    QString GetStatus() const;
    int GetCompletion() const;
    int GetItemCount() const;
    int GetNewItems() const;
    EStateFlags GetFlags() const;
};

Q_DECLARE_METATYPE(CStatus);

