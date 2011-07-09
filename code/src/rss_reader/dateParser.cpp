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

#include <QRegExp>
#include <QDebug>

#include "dateParser.h"

///<summary>Retruns the index of a QString value in a list of strings.</summary>
///<param name="list">Array of pointers to null terminated strings. This array is terminated by a NULL pointer.</param>
///<param name="find">The element to find within the array of pointers to null terminated strings.</param>
///<returns>The index of the value find in the array of pointers to null terminated strings. Or -1 if the value could not be found.</returns>
///<remarks>The comparison is done case insensitive. The list-parameter must end with a NULL pointer!</remarks>
int CDateParser::IndexInList(char* list[], QString find)
{
	char* item;
	int iItemIndex=0;
	while ((item=list[iItemIndex])!=NULL)
	{
		if (find.compare(item, Qt::CaseInsensitive)==0)
			return iItemIndex;
			
		iItemIndex++;
	}
	
	return -1;
}

///<summary>Parses a RFC822 compliant time offset into the number of seconds from UTC.</summary>
///<param name="zone">The zone-part of a RFC822 compliant timestamp.</param>
///<returns>Returns the offset from UTC in seconds.</returns>
int CDateParser::rfc822TimeOffset(QString zone)
{
	int offset;
	int hours;
	int minutes;
	char* offsetMap[]={"ut", "gmt", "est", "edt", "cst", "cdt", "mst", "mdt", "pst", "pdt", "z", "ut", "a", "m", "n", "y", NULL};
	
	switch (zone[0].toAscii())
	{
		case '+':
		case '-':
			offset=zone.mid(1).toInt();
			hours=offset/100;	//Integer division to get the hours
			minutes=offset-(hours*100);	//The rest is the minutes part
			offset=((hours*60)+minutes)*60;
			
			//-- Negate the value if the offset is negative
			if (zone[0]=='-')
				offset*=-1;
			break;
		default:
			switch (IndexInList(offsetMap, zone))
			{
				case 0:		//UT
				case 1:		//GMT
				case 10:	//Z
				case 11:	//UT
					offset=0;
					break;
				case 2:		//EST
					offset=-5;
					break;
				case 3:		//EDT
					offset=-4;
					break;
				case 4:		//CST
					offset=-6;
					break;
				case 5:		//CDT
					offset=-5;
					break;
				case 6:		//MST
					offset=-7;
					break;
				case 7:		//MDT
					offset=-6;
					break;
				case 8:		//PST
					offset=-8;
					break;
				case 9:		//PDT
					offset=-7;
					break;
				case 12:	//A
					offset=-1;
					break;
				case 13:	//M
					offset=-12;
					break;
				case 14:	//N
					offset=+1;
					break;
				case 15:	//Y
					offset=+12;
					break;
			}
			offset*=60*60;	//Convert hours to seconds
			break;
	}	
	
	return offset;
}

///<summary>Parses a RFC3339 compliant time offset into the number of seconds from UTC.</summary>
///<param name="zone">The zone-part of a RFC3999 compliant timestamp.</param>
///<returns>Returns the offset from UTC in seconds.</returns>
int CDateParser::rfc3339TimeOffset(QString zone)
{
	int minutes;
	int hours;
	int offset;
	
	if ((zone[0]=='Z')||(zone[0]=='z'))
		return 0;
	
	hours=zone.mid(1, 2).toInt();
	minutes=zone.mid(4, 2).toInt();
	offset=((hours*60)+minutes)*60;
	if (zone[0]=='-') offset*=-1;
	
	return offset;
}

///<summary>Translates a RFC822 compliant month into a value.</summary>
///<param name="month">The month as a QString.</param>
///<returns>Returns the number of the month 1-12 or 0 on error.</returns>
int CDateParser::GetMonth(QString month)
{
	char* monthMap[]={"jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec", NULL};
	
	return IndexInList(monthMap, month)+1;
}

///<summary>Parses a RFC822 or RFC3339 compliant date into a QDateTime.</summary>
///<param name="dateString">The input date as a QString.</param>
///<returns>A QDateTime continaing the parsed date and time (only hours and minutes) translated to UTC.</returns>
QDateTime CDateParser::Parse(QString dateString)
{
	QDateTime parsedDT;
	int offset;

	QRegExp rfc822separator("([0-9]{1,2}) (jan|feb|mar|apr|may|jun|jul|aug|sep|oct|nov|dec){1} ([0-9]{4}) ([0-9]{2}):([0-9]{2}).* (ut|gmt|est|edt|cst|cdt|mst|mdt|pst|pdt|z|ut|a|m|n|y|\\+[0-9]{4}|\\-[0-9]{4})");
	rfc822separator.setCaseSensitivity(Qt::CaseInsensitive);
	rfc822separator.setMinimal(true);
	
	QRegExp rfc3339separator("([0-9]{4})-([0-9]{2})-([0-9]{2})T([0-9]{2}):([0-9]{2}):[0-9]{2}\\.?[0-9]*(Z|\\+[0-9]{2}:[0-9]{2}|\\-[0-9]{2}:[0-9]{2})");
	rfc3339separator.setCaseSensitivity(Qt::CaseInsensitive);
	rfc3339separator.setMinimal(true);
	
	//-- All Date values will be in UTC
	parsedDT.setTimeSpec(Qt::UTC);

	//-- If the string matches, it an RFC822 date/time string and can be converted here
	if (rfc822separator.indexIn(dateString)!=-1)
	{
		QDate datePart(rfc822separator.cap(3).toInt(), GetMonth(rfc822separator.cap(2)), rfc822separator.cap(1).toInt());
		QTime timePart(rfc822separator.cap(4).toInt(), rfc822separator.cap(5).toInt());
		offset=rfc822TimeOffset(rfc822separator.cap(6));

		parsedDT.setDate(datePart);
		parsedDT.setTime(timePart);
		parsedDT=parsedDT.addSecs(offset*(-1));
	}
	else if (rfc3339separator.indexIn(dateString)!=-1)
	{
		QDate datePart(rfc3339separator.cap(1).toInt(), rfc3339separator.cap(2).toInt(), rfc3339separator.cap(3).toInt());
		QTime timePart(rfc3339separator.cap(4).toInt(), rfc3339separator.cap(5).toInt());
		parsedDT.setDate(datePart);
		parsedDT.setTime(timePart);
		parsedDT=parsedDT.addSecs(rfc3339TimeOffset(rfc3339separator.cap(6))*(-1));
	}
	
	return parsedDT;
}
