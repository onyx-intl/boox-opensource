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

#include <QtDebug>
#include <QCryptographicHash>
#include "rssItem.h"
#include "dateParser.h"

#include "rssFeed.h"

//-- Defines a helper macro for loging
#define LOG(__line) if (Log) (*Log) << __line << "\r\n";


///<summary>Constructor</summary>
///<param name="rssStream">Reference to a QByteArray instance that contains the xml of an RSS or Atom feed</param>
CRSSFeed::CRSSFeed(const QByteArray& rssStream) :
	Xml(rssStream),
	Log(NULL),
	ErrorMessage("")
{
	StreamLength=rssStream.length();	
}

///<summary>Destructor</summary>
CRSSFeed::~CRSSFeed()
{
}

///<summary>Advances to the next feed item within the xml file</summary>
///<returns>Returns a CRSSItem instance pointer or NULL if no more items are available</returns>
///<remarks>It is the responsibility of the caller to free the CRSSItem pointer</remarks>
CRSSItem* CRSSFeed::NextItem()
{	
	ETagType currentType;
	CRSSItem* nextItem;

	//-- Define the mapping of XML elements to Types
	SItemMapper itemMap[]={
		{"item", 				ettItem},			//RSS
		{"entry", 			ettItem},			//Atom
		{"title",				ettTitle},		//RSS + Atom
		{"link",				ettLink},			//RSS + Atom
		{"description",	ettSummary},	//RSS
		{"summary",			ettSummary},	//Atom
		{"content",			ettContent},	//Atom
		{"guid",				ettGuid},			//RSS
		{"id",					ettGuid}, 		//Atom
		{"pubDate",			ettDate},			//RSS
		{"updated",			ettDate},			//Atom
		{"date",				ettDate}			//Atom
	};
	
	//-- Das aktuelle Item auf NULL setzen
	nextItem=NULL;
	
	//-- Iterate over all XML elements
	while (!Xml.atEnd())
	{
		Xml.readNext();
		if (Xml.name().length()==0)
			continue;
			
		LOG("* Tag: \"" << (Xml.isEndElement()?"/":"") << Xml.name().toString() << "\"");
		
		//-- Map this item to an Element of ETagType
		currentType=ettUnkown;
		for (uint i=0; i<sizeof(itemMap)/sizeof(SItemMapper); i++)
		{
			if (Xml.name().compare(itemMap[i].Tag, Qt::CaseInsensitive)==0)
			{
				currentType=itemMap[i].Type;
				break;
			}
		}
		
		//-- If the nextItem is NULL...
		if (nextItem==NULL)
		{
			//-- ...and the Type is ettTitle then it's the feed's title
			if (currentType==ettTitle)
				this->Title=Xml.readElementText();
			
			//-- Ensure that all other elements are only valid if a start item has been found
			if (currentType!=ettItem)
				continue;			
		}
		
		//-- Item tags are handled specially
		if (currentType==ettItem)
		{
			if (Xml.isStartElement())
			{
				LOG("*** Start of item");
				nextItem=new CRSSItem();
				nextItem->SetLog(Log);
			}
			else
				break;	//The end of this item was reached, bail out
		}
		
		//-- Now switch based on the tag type (ignore end elements, because we're mostly interested in attributes and element text
		if (Xml.isStartElement())
		{
			switch (currentType)
			{
				case ettItem:
					break;
					
				case ettTitle:
					nextItem->Title=Xml.readElementText();
					
					LOG("  * Title element: " << nextItem->Title);
					break;
					
				case ettLink:					
					if (Xml.attributes().hasAttribute("href"))	//The link can be hidden inside a href attribute
						nextItem->Link=Xml.attributes().value("href").toString();
					else
						nextItem->Link=Xml.readElementText();
						
					LOG("  * Link element: " << nextItem->Link);
					break;
					
				case ettSummary:
					nextItem->Summary=Xml.readElementText();
					
					LOG("  * Summary element: " << nextItem->Summary);
					break;
					
				case ettContent:
					nextItem->Content=Xml.readElementText();
					
					LOG("  * Content element with length: " << nextItem->Content.length());
					break;
					
				case ettGuid:
					nextItem->Guid=QCryptographicHash::hash(Xml.readElementText().toAscii(), QCryptographicHash::Md5).toHex();
					
					LOG("  * Hash of guid is: " << nextItem->Guid);
					break;
					
				case ettDate:
					nextItem->Timestamp=CDateParser::Parse(Xml.readElementText());
					
					LOG("  * Parsed date is: " << nextItem->Timestamp.toString());
					break;
			}
		}
	}
		
	//-- If an error occured, log the error, save the error and exit
	if (Xml.error()!=QXmlStreamReader::NoError)
	{
		LOG("! Parse error: " << Xml.errorString());
		ErrorMessage=QString("Error: %1 (Line %2, Char %3)").arg(Xml.errorString()).arg(Xml.lineNumber()).arg(Xml.columnNumber());
		delete nextItem;
		nextItem=NULL;
	}
	
	//-- If the current Item has no Guid create one from the Title of the item
	if (nextItem!=NULL)
	{
		if (nextItem->Guid=="")
			nextItem->Guid=QCryptographicHash::hash(nextItem->Title.toAscii(), QCryptographicHash::Md5).toHex();
	}
	
	return nextItem;
}

///<summary>Retrieves the position within the XML file</summary>
///<param name="max">The maximum value for scaling the position</param>
///<returns>Returns a value between 0 and max to represet the current position within the XML file</returns>
uint CRSSFeed::GetPosition(uint max)
{
	return (Xml.characterOffset()*max)/StreamLength;
}

///<summary>Returns the title of the Feed</summary>
///<returns>The title of the feed extracted from the web site</returns>
///<remarks>This property is only valid after the first call to NextItem()</remarks>
QString CRSSFeed::GetTitle()
{
	return Title;
}

///<summary>Enables loging for this instance</summary>
///<param name="log">Pointer to a QTextStream instance to use for loging</param>
void CRSSFeed::SetLog(QTextStream* log)
{
	Log=log;
}

///<summary>Returns true if there was an error processing the last element.</summary>
///<returns>Returns true if NextItem returned NULL because of an error.</summary> 
///<remarks>You can call GetErrorMessage to get the details of the error.</remarks>
bool CRSSFeed::Error()
{
	return (ErrorMessage.length()>0);
}

///<summary>Returns a detailed error message if a parse error occured.</summary>
///<returns>Retruns a QString instance containing the error message describing the last error. If Error() returned false, an empty string will be returned.</returns>
QString CRSSFeed::GetErrorMessage()
{
	return ErrorMessage;
}
