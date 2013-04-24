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

#include "syncHttp.h"
#include "itemStore.h"
#include "feedStore.h"
#include "htmlWriter.h"

#include "rssItem.h"

#define BLOCK_ELEMENTS_REGEXP "<(/?)(body|div)(.*)>"

//-- Defines a helper macro for loging
#define LOG(__line) { if (Log) (*Log) << __line << "\r\n"; }

//-- Defines a helper macro for the cancel flag
#define CANCELED() ((cancelPtr!=NULL)&&(*cancelPtr))

///<summary>Default c'tor</summary>
CRSSItem::CRSSItem() :
	Fetched(false),
	Log(NULL),
	CancelPtr(NULL)
{
}

///<summary>C'tor with deserialisation</summary>
///<param name="srcDevice">Device from which to deserialize this instance</param>
///<param name="cancelPtr">Zeiger auf eine Variable, welche als Abbruchflag genutzt werden soll. Ist dieser Parameter NULL, so wird die Instanz ohne Abbruchflag erstellt.</param>
CRSSItem::CRSSItem(QIODevice* srcDevice) :
	Log(NULL),
	CancelPtr(NULL)
{
	QDataStream inStream(srcDevice);
	
	inStream >> Title;
	inStream >> Link;
	inStream >> LocalLink;
	inStream >> Summary;
	inStream >> Content;
	inStream >> Guid;
	inStream >> Timestamp;
	inStream >> Fetched;
}

///<summary>Serializes the informatin for this rssItem into the given QIODevice</summary>
///<param name="dstDevice">Pointer to a QIODevice instance that recieves the serialized data.</param>
void CRSSItem::Serialize(QIODevice* dstDevice)
{
	QDataStream outStream(dstDevice);
	
	outStream << Title;
	outStream << Link;
	outStream << LocalLink;
	outStream << Summary;
	outStream << Content;
	outStream << Guid;
	outStream << Timestamp;
	outStream << Fetched;
}

///<summary>Checks if the items timestamp is within the given amount of days from now.</summary>
///<param name="keepDays">Number of days an item should be returned as valid.</param>
///<returns>Returns true if the timestamp of this item is within Now to Now-keepDays.</returns>
bool CRSSItem::IsInTime(int keepDays)
{
	return Timestamp.toLocalTime().addDays(keepDays)>=QDateTime::currentDateTime();
}

///<summary>Adds this feed item to the global index file.</summary>
///<param name="indexWriter">Referenze to a QFile instance used for writing the index-file.</param>
///<param name="lastReadTimestamp">Reference to a UTC Timestamp defining when the last element was read. This is used for showing the NEW-tag.</param>
void CRSSItem::AddToIndex(CHtmlWriter* indexWriter, const QDateTime& lastReadTimestamp)
{
	QString newFlag("");
	if (Timestamp>=lastReadTimestamp)
		newFlag=QObject::tr("new - ");
	
	indexWriter->Write("<HR>\n");
	if (LocalLink.isEmpty())	//If LocalLink is empty then fetching faild or was disabled - do not create a local link
	{
		indexWriter->Write(QString("<DIV CLASS=\"nfinfo\">%1 - [%3<A HREF=\"%2\">www</A>]</DIV>\n").arg(Timestamp.toLocalTime().toString(Qt::SystemLocaleLongDate)).arg(Link).arg(newFlag));
		indexWriter->Write(QString("<DIV CLASS=\"nftitle\">%1</DIV>\n").arg(Title));
	}
	else
	{
		indexWriter->Write(QString("<DIV CLASS=\"nfinfo\"><INPUT TYPE=\"radio\" NAME=\"item\" VALUE=\"%4\"/>%1 - [%3<A HREF=\"%2\">www</A>]</DIV>\n").arg(Timestamp.toLocalTime().toString(Qt::SystemLocaleLongDate)).arg(Link).arg(newFlag).arg(LocalLink));
		indexWriter->Write(QString("<DIV CLASS=\"nftitle\"><A HREF=\"%1\">%2</A></DIV>\n").arg(LocalLink).arg(Title));
	}
	indexWriter->Write("<DIV>");
	indexWriter->Write(Summary);
	indexWriter->Write("</DIV><BR/>");
}

///<summary>Retruns a number that identifies the day of the timestamp.</summary>
///<returns>The returned number is unique and monothonic for each day from the 01.01.2000 on.</returns>
int CRSSItem::GetDayId()
{
	QDate date=Timestamp.toLocalTime().date();
	
	return date.day()+date.month()*31+(date.year()-2000)*400;
}

///<summary>Counts the number of matches of the supplied regular expression within the supplied string.</summary>
///<param name="inputString">Constant reference to the string that should be matched against regExp.</param>
///<param name="regExp">Constant reference to a regular expression that should be matched within inputString.</param>
///<returns>Returns the number of times regExp matched within inputString.</returns>
int CRSSItem::CountMatches(const QString &inputString, const QString &regExp)
{
	QRegExp matchExp(regExp, Qt::CaseInsensitive);
	int count=0;
	int position=0;
	
	matchExp.setMinimal(true);
	while ((position=matchExp.indexIn(inputString, position))>-1)
	{
		position+=matchExp.cap(0).length();
		count++;
	}
	
	return count;
}

///<summary>Calculates the weight of the supplied HTML.</summary>
///<param name="inputString">QString instance with a part of a HTML document that should be weighted.</param>
///<returns>Returns an integer containing the relative weight of the supplied HTML snippet.</returns>
int CRSSItem::CalcWeight(QString inputString)
{
	int htmlLength;
	int textLength;
	int position;	
	int weight;
	
	//-- Define the regular expressions to remove all tags for creating the text weight
	char* tagRemoveFilter[]=
	{
		"<a.*</a.*>",
		"<.*>",
		NULL
	};

	//-- Define the regular expression to remove characters from the HTML version to ease comparison
	char* htmlCleanFilter[]=
	{
		"[ \\f\\n\\r\\t\\v]+",
		NULL
	};

	//-- Initialize the weight to 0
	weight=0;
	
	//-- Penalty for a tags
	weight-=CountMatches(inputString, "<a.*>")*3;
	
	//-- If there are headlines in this div, make it worth 10 times the number of headlines
	weight+=CountMatches(inputString, "<(h[0-9]).*>")*10;
		
	
	RemoveByRegex(htmlCleanFilter, inputString);
	htmlLength=inputString.length();
	RemoveByRegex(tagRemoveFilter, inputString);
	textLength=inputString.length();	
	
	//-- If the text is shorter than 20 Chars, it can't be any important content.
	//   If htmlLength is 0 then we would get a div/0
	if ((textLength>20)&&(htmlLength>0))
	{
		weight+=(textLength*100)/htmlLength;
	}	

	return weight;
}

///<summary>Detects if the attributes portion of a tag contains any of the given hints.</summary>
///<param name="attributes">QString instance containing the attributes portion of a HTML tag.</param>
///<param name="hints">List of hints separated by a colon.</param>
///<returns>Returns true if one of the elements of the hints string is contained within attributes. False if not.</returns>
bool CRSSItem::HasHint(QString attributes, const QString& hints)
{
	if (hints.length()==0)
		return false;
		
	QStringList hintList=hints.split(",", QString::SkipEmptyParts);
	QStringListIterator hintListI(hintList);
	
	while (hintListI.hasNext())
	{
		if (attributes.indexOf(hintListI.next().trimmed(), Qt::CaseInsensitive)!=-1)
			return true;
	}
		
	return false;
}

///<summary>Copies the supplied noed to the resultPage string.</summary>
///<param name="resultPage">Reference to a QString instance. The append method is used to append the supplied node to this instance.</param>
///<param name="inputString">Reference to the whole HTML page. The node parameter contains positional references relative to this string.</param>
///<param name="node">Pointer to a CSectionNode instance specifying the portion of inputString to add to resultPage.</param>
///<param name="threshold">The portion specified by node is only added if the weight of this node is greater than this value.</param>
///<param name="linkThreshold">Currently not used. TBD!</param>
void CRSSItem::WriteNodes(QString &resultPage, QString &inputString, CSectionNode* node, int threshold, int linkThreshold)
{
	//-- If the node's weight is less then the threshold mark it as disposible
	if (node->Weight>threshold)
	{
		node->Kept=true;
		
		resultPage.append("<DIV>");
		resultPage.append(inputString.mid(node->ContStart, node->ContEnd-node->ContStart));
		resultPage.append("</DIV>");
	}
	else
	{
		//-- Analyse all the child nodes
		QListIterator<CSectionNode*> childrenI=node->GetChildren();
		while (childrenI.hasNext())
		{
			WriteNodes(resultPage, inputString, childrenI.next(), threshold, linkThreshold);
		}
	}
}

///<summary>Creates a tree of all div-sections within the supplied HTML document.</summary>
///<param name="rootNode">Pointer to a reference of the root node for the created tree. All items will be children of the root node.</param>
///<param name="inputString">Reference to a QString instance containing the input string to parse.</param>
///<param name="hint">Reference to a QString instance containing the hint-list to use for weight calculation.</param>
///<param name="threshold">Threshold as specified in the feed config.</param>
///<param name="cancelPtr">Zeiger auf eine Variable, welche als Cancel-Flag genutzt wird.</param>
///<returns>Returns -1 if the request was cancled or 0 TBD!</returns>
int CRSSItem::CreateSectionList(CSectionNode* &rootNode, QString& inputString, const QString& hint, int threshold, volatile bool* cancelPtr)
{
	int start=0;
	int tagLength;
	int prevTagEnd=-1;	
	CSectionNode* currentNode=NULL;	
	QRegExp blockElements;
	
	//-- Set the root node to NULL
	rootNode=NULL;

	//-- Initialize the RegEx
	blockElements.setCaseSensitivity(Qt::CaseInsensitive);
	blockElements.setMinimal(true);
	blockElements.setPattern(BLOCK_ELEMENTS_REGEXP);

	//-- Enum all Tags
	while ((start=blockElements.indexIn(inputString, start))!=-1)
	{
		tagLength=blockElements.matchedLength();	//cap(0).length();		
		
		//-- Add everything from the end of the last tag to this new tag to the parents content
		if ((prevTagEnd>=0)&&(start-prevTagEnd-1>0))
			currentNode->Content+=inputString.mid(prevTagEnd, start-prevTagEnd-1);
		
		//-- If cap(1) is a / then it was an end tag otherwise it was a start tag
		if (blockElements.cap(1)[0]!='/')
		{
			//-- Start tag
			currentNode=new CSectionNode(currentNode);
			if (currentNode==NULL)
				return -1;
				
			currentNode->Start=start;
			currentNode->ContStart=start+tagLength;
			currentNode->Tag=blockElements.cap(2).toLower();
			currentNode->Weight=(HasHint(blockElements.cap(3), hint)?threshold*2:0);
			
			//-- If this is the first start tag, save it as the root node
			if (rootNode==NULL)
				rootNode=currentNode;
		}
		else
		{
			if (currentNode!=NULL)
			{
				//-- End tag
				if (currentNode->Tag==blockElements.cap(2).toLower())
				{
					currentNode->Weight+=CalcWeight(currentNode->Content);
					
					//-- If debug loging is off, clear the Content-String and free the memory
					if (!Log)
						currentNode->Content.clear();
					
					currentNode->End=start+tagLength;
					currentNode->ContEnd=start-1;
					currentNode=currentNode->GetParent();				
				}
			}
			
			//-- If the tree is empty, then all tags are closed (superfluous end tag) and we add the rest to the root node
			if (currentNode==NULL)
				currentNode=rootNode;			
		}
		
		if (CANCELED())
			return -1;
		
		start+=tagLength;
		prevTagEnd=start;
	}
	
	//-- Dump the tree as debug output
	if (Log)
	{
		LOG("Threshold: " << threshold);
	
		if (rootNode==NULL)
			LOG("No root node found!")
		else
			rootNode->Dump(Log);
	}
	
	return 0;
}

///<summary>Removes a list of regular expressions from a given string.</summary>
///<param name="filters">Pointer to an array of null terminated strings containing the regular expressions that specify the portions of the string to remove. The list must be terminated by a NULL pointer!</param>
///<param name="string">Reference to the string that should be cleaned by the supplied regular expressions.</param>
void CRSSItem::RemoveByRegex(char* filters[], QString& string)
{
	int filterIndex=0;
	char* filterExpression;
	QRegExp filterRegExp;
	
	filterRegExp.setCaseSensitivity(Qt::CaseInsensitive);
	filterRegExp.setMinimal(true);
	
	while (filterExpression=filters[filterIndex++])
	{
		filterRegExp.setPattern(filterExpression);
		string.replace(filterRegExp, "");
	}
}

///<summary>Downloads all the images referenced within the page to the local store</summary>
///<param name="baseUrl">The base url for this page. This is used to resolve relative image paths.</param>
///<param name="itemStore">Pointer to the item store that will be used for storing the downloaded images.</param>
///<param name="syncHttp">Pointer to the CSyncHttp instance that will be used for downloading the images.</param>
///<param name="page">Reference to the page-html that will be changed to reflect the local path of the images.</param>
///<param name="storeAbsolute">Use absolue path for storage (must be set to true if the image is not refernce within the HTML-document created by CItemStore::StoreIndex</param>
///<param name="skipNonLocalImages">Skips all images that are not on the local site (uses the last two URL parts for this).</param>
///<param name="cancelPtr">Zeiger auf eine Variable, welche als Cancel-Flag genutzt wird.</param>
///<remarks>This function assumes that the page referenced by the page parameter is stored within the same path as the downloaded images.</remarks>
///<returns>Return true if all images could be fetched or false otherwise.</returns>
bool CRSSItem::FetchImages(QUrl baseUrl, CItemStore* itemStore, CSyncHttp* syncHttp, QString& page, bool storeAbsolute, bool skipNonLocalImages, volatile bool* cancelPtr)
{
	int pos=0;
	QRegExp imageFinder("<img[ \\n].*src *=\"([^\"]*)\".*>", Qt::CaseInsensitive);
	QUrl imageUrl;
	QString localFile;
	QByteArray image;
	QRegExp baseHostPartExp("([^.]+\\.[^.]+)$");
	QString baseHostPart("");
	QString imageBaseHostPart;
	QMap<QString, QString> storedImages;
		
	imageFinder.setMinimal(true);
	
	//-- Get the baseHostPart of the current feed/site to allow skipping non local images
	if (skipNonLocalImages)
	{
		if (baseHostPartExp.indexIn(baseUrl.encodedHost())!=-1)
			baseHostPart=baseHostPartExp.cap(1);
	}
	
	//-- Find and download all images
	while ((pos=imageFinder.indexIn(page, pos))!=-1)
	{
		//-- Allow canceling
		if (CANCELED())
			return false;
		
		//-- Find the next image by adding the length of the capture to the position
		pos+=imageFinder.cap(0).length();
		
		//-- Do not download local file urls!
		if (imageFinder.cap(1).indexOf("file:", 0, Qt::CaseInsensitive)==0)
			continue;
		
		if (!storedImages.contains(imageFinder.cap(1)))
		{
			imageUrl=baseUrl.resolved(imageFinder.cap(1));
			
			//-- If the baseHostPart is 0, then it should not be used
			if (baseHostPart.length()>0)
			{			
				//-- Get the base host part of this image to allow skipping it, if it's not from the same site
				if (baseHostPartExp.indexIn(imageUrl.encodedHost())!=-1)
				{
					imageBaseHostPart=baseHostPartExp.cap(1);
					
					if (baseHostPart!=imageBaseHostPart)
					{
						storedImages.insert(imageFinder.cap(1), "");
						if (Log)
							LOG("Skiped non local image: " << imageFinder.cap(1));
						continue;
					}
				}
			}
			
			localFile=itemStore->GetLocalName(imageUrl);
			
			image=image=syncHttp->Get(imageUrl);
			itemStore->StoreElement(localFile, image);//!Check
			
			if (storeAbsolute)
				storedImages.insert(imageFinder.cap(1), itemStore->GetParentFeedStore()->GetWriterRelUrlForFeedItem(itemStore, localFile).toString());
			else
				storedImages.insert(imageFinder.cap(1), localFile);
		}
	}
	
	//-- Now replace all references to the images with the local representation
	QMapIterator<QString, QString> si(storedImages);
	while (si.hasNext())
	{
		si.next();
		page.replace(si.key(), si.value());

		//-- Allow canceling
		if (CANCELED())
			return false;
	}
	
	return true;
}

char* CRSSItem::parseEncoding(const QByteArray &data)
{
    QDomDocument dom;
    dom.setContent(data);
    QString encoding_str;
    QDomNodeList node_lists = dom.elementsByTagName("meta");
    for(int i = 0; i < node_lists.length(); i++)
    {
        QDomNode node = node_lists.item(i);
        encoding_str =  node.toElement().attribute("content");
        int index = encoding_str.indexOf("charset");
        if(index > 0)
        {
            encoding_str = encoding_str.right(encoding_str.length() - index - QString("charset=").length());
        }
        if(!encoding_str.isEmpty())
            break;

    }

    if( encoding_str.isEmpty())
    {
        QString data_str(data);
        data_str.replace("doctype", "DOCTYPE");

        QDomDocument dom;
        dom.setContent(data_str.toLatin1());
        QDomNodeList node_lists = dom.elementsByTagName("meta");
        for(int i = 0; i < node_lists.length(); i++)
        {
            QDomNode node = node_lists.item(i);
            encoding_str =  node.toElement().attribute(QString("charset"));
            if(!encoding_str.isEmpty())
                break;
        }
    }

    //  if the result of parse is empty or the length out of range then using default the encoding
    if(encoding_str.isEmpty() || encoding_str.length() > 20 || encoding_str.length() < 5)
    {
        encoding_str = "utf-8";
    }
    qDebug() << "encoding_str  = " << encoding_str;

    QByteArray ba = encoding_str.toLatin1();
    return ba.data();
}

///<summary>Tries to detect the codec for the content in QByteArray and converts it into a Unicode string</summary>
///<param name="data">Byte data for analysis and conversion</param>
///<returns>Returns the content of the HTML page inside the QByteArray instance as a QString instance.</returns>
///<remarks>This function is neccessary because codecForHtml is not working on all feeds</remarks> 
QString CRSSItem::ConvertToUnicode(QByteArray data)
{
	QTextCodec* defaultCodec=QTextCodec::codecForName(parseEncoding(data));
	QTextCodec* codec=QTextCodec::codecForHtml(data, defaultCodec);
	QString convResult;
	
	//-- Convert to QString with the correct codec or with ISO-8859-1
	convResult=codec->toUnicode(data);
	
	//-- The default encoding was returned, now we have to check, if a meta header specifies something different
	if (codec==defaultCodec)
	{
		QRegExp metaEncoding("<meta[ \\t]*http-equiv[ \\t]*=[ \\t]*\"content-type\"[ \\t]*content[ \\t]*=[ \\t]*\"text/html *; *charset *= *(.*) *\"", Qt::CaseInsensitive);
		metaEncoding.setMinimal(true);
		
		if (metaEncoding.indexIn(convResult)!=-1)
		{
			//-- If a new codec is found for the content-type specified inside the meta tag, use this codec and translate the page again
			codec=QTextCodec::codecForName(metaEncoding.cap(1).toAscii());
			if (codec!=NULL)
				convResult=codec->toUnicode(data);
		}
	}
	
	return convResult;
}

///<summary>Downloads the page referenced by the details-link to the local disk</summary>
///<param name="itemStore">Pointer to the CItemStore instance that will be used for string the information</param>
///<param name="syncHttp">Pointer to the CSyncHttp instance that should be used for downloading the page and the images</param>
///<param name="hint">Hint for the detection of the content DIV</param>
///<param name="threshold">Threshold for content detection</param>
///<param name="skipNonLocalImages">Skips all images that are not on the local site (uses the last two URL parts for this).</param>
///<param name="cancelPtr">Zeiger auf eine Variable, welche als Cancel-Flag genutzt wird.</param>
///<returns>Returns true if the page could be stored successfully or false otherwise.</returns>
bool CRSSItem::FetchDetailsLink(CItemStore* itemStore, CSyncHttp* syncHttp, const QString& hint, int threshold, bool skipNonLocalImages, volatile bool* cancelPtr)
{
	QString errorMessage;
	QString page;
	CSectionNode* rootNode=NULL;
	int medianLinkCount;
	bool downloadSucceeded;
	QUrl url(Link);

	//-- Define the regular expressions to remove all unwanted tags
	char* filterExpressions[]=
	{
		"<!--.*-->",
		"<head.*</head.*>",
		"<meta.*>",
		"<base.*>",
		"<link.*>",
		"<script.*</script.*>",
		"<embed.*</embed.*>",
		"<iframe.*</iframe.*>",
		"<object.*</object.*>",
		"<form.*</form.*>",
		NULL
	};
	
	LOG("-------- " << url.toString() << " --------");
	
	//-- Load the page
	//   Use own scope to free the ByteArray as soon as possible
	{
		QByteArray fetchResult=syncHttp->Get(url);
    page=ConvertToUnicode(fetchResult);
	}
	
	RemoveByRegex(filterExpressions, page);
		
	medianLinkCount=CreateSectionList(rootNode, page, hint, threshold, cancelPtr);
	downloadSucceeded=(medianLinkCount>=0);
	
	if (downloadSucceeded)
	{
		QString resultPage("<HTML><HEAD><META HTTP-EQUIV=\"content-type\" CONTENT=\"text/html; charset=utf-8\">");
		resultPage.append("<STYLE TYPE=\"text/css\">\n");
		resultPage.append("body {font-size: 125%; }\n");
		resultPage.append("</STYLE>\n<TITLE>");
		resultPage.append(Title);
		resultPage.append("</TITLE></HEAD><BODY onkeyup=\"if (event.keyCode==13) history.back();\">");
		if (rootNode)
			WriteNodes(resultPage, page, rootNode, threshold, medianLinkCount);
		else
			resultPage.append("<H1>Error in content detector. No content found.</H1>");
		resultPage.append("</BODY></HTML>");
		page=resultPage;
		
		downloadSucceeded&=FetchImages(url, itemStore, syncHttp, page, false, skipNonLocalImages, cancelPtr);
	}
	
	//-- Save the translated page as an index.html file and store the local URL for
	//   integration into the link
	if (downloadSucceeded)
	{
		LocalLink=itemStore->StoreIndex(page).toString();
	
		//-- Mark this feed as fetched
		Fetched=true;
	}
	
	//-- Delete the node tree
	if (rootNode!=NULL)
		delete rootNode;
	
	return downloadSucceeded;
}

///<summary>Fetches the images referenced by the summary of this feed.</summary>
///<param name="itemStore">Pointer to a CItemStore instance used for storing the images referenced within the summary of the feed.</param>
///<param name="syncHttp">Pointer to a CSyncHttp instance used for downloading the images referenced within the summary of the feed.</param>
///<param name="skipNonLocalImages">Skips all images that are not on the local site (uses the last two URL parts for this).</param>
///<param name="cancelPtr">Zeiger auf eine Variable, welche als Cancel-Flag genutzt wird.</param>
///<returns>Returns true if the images where downloaded successfully or false if an error occured.</returns>
bool CRSSItem::FetchSummary(CItemStore* itemStore, CSyncHttp* syncHttp, bool skipNonLocalImages, volatile bool* cancelPtr)
{
	//-- Define the regular expressions to remove all unwanted tags
	char* filterExpressions[]=
	{
		"<!--.*-->",
		"<head.*</head.*>",
		"<meta.*>",
		"<base.*>",
		"<link.*>",
		"<script.*</script.*>",
		"<embed.*</embed.*>",
		"<iframe.*</iframe.*>",
		"<object.*</object.*>",
		"<form.*</form.*>",
		NULL
	};

	RemoveByRegex(filterExpressions, Summary);
	
	return FetchImages(QUrl(Link), itemStore, syncHttp, Summary, true, skipNonLocalImages, cancelPtr);
}

///<summary>Enables loging for this instance</summary>
///<param name="log">Pointer to a QTextStream instance to use for loging</param>
void CRSSItem::SetLog(QTextStream* log)
{
	Log=log;
}
