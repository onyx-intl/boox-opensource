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

#include "htmlWriter.h"

///<summary>Constructor</summary>
///<param name="outFile">Pointer to a QFile instance that is used as the output file</param>
///<param name="headerResource">Name of the resource containing the text to insert between <HEAD> and </HEAD></param>
///<param name="bodyAttributes">Attributes to add to the BODY-tag</param>
///<param name="title">The text to set between the TITLE-tags</param>
///<remarks>CHtmlWriter adopts the given QFile pointer and deletes it on exit. CHtmlWriter creates an HTML-Header and an appropriate BODY-tags.</remarks>
CHtmlWriter::CHtmlWriter(QFile* outFile, QString headerResource, QString bodyAttributes, QString title) :
	OutFile(outFile)
{
	Write("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"\n\"http://www.w3.org/TR/html4/loose.dtd\">\n");
	Write("<HTML>\n");
	Write("<HEAD>\n");
	Write("<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=UTF-8\" />\n");
	
	//-- If headerResource is "" don't try to write one
	if (!headerResource.isEmpty())
	{
		QResource hdrResource(headerResource);
		Write(QString::fromUtf8((char*)hdrResource.data(), hdrResource.size()));
	}
	
	Write("<TITLE>"); //! I don't like this. I think I should make this more reusable. (Remove all this from the c'tor!)
	Write(title);
	Write("</TITLE>\n</HEAD>\n");
	Write(QString("<BODY %1>\n").arg(bodyAttributes));
	Write("<H1>");
	Write(title);
	Write("</H1>\n");
}

///<summary>Destructor</summary>
CHtmlWriter::~CHtmlWriter()
{
	Write("</BODY>\n</HTML>\n");
	delete OutFile;
}

///<summary>Writes text between the BODY-tags of this HTML-page</summary>
///<param name="out">The text to put between the body elements</param>
void CHtmlWriter::Write(const QString out)
{
	OutFile->write(out.toUtf8());
}

///<summary>Writes text between the BODY-tags of this HTML-page</summary>
///<param name="out">Pointer to a null terminated string to be put between the body elements</param>
void CHtmlWriter::Write(char* out)
{
	QString outInstance(out);
	Write(outInstance);
}
