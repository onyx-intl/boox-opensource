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
#include <QDateTime>

///<summary>Static class for parsing RFC822 and RFC3339 compliant date/time structures</summary>
class CDateParser
{
	//-- Private, static functions
	private:
		static int rfc822TimeOffset(QString zone);
		static int rfc3339TimeOffset(QString zone);
		static int GetMonth(QString month);
		static int IndexInList(char* list[], QString find);
		
	//-- Public, static functions
	public:
		static QDateTime Parse(QString dateString);
};
