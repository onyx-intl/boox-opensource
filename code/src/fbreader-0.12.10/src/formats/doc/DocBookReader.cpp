/*
* Copyright (C) 2004-2009 Geometer Plus <contact@geometerplus.com>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
* 02110-1301, USA.
*/

#include <cctype>

#include <ZLStringUtil.h>

#include "DocBookReader.h"
#include "../../bookmodel/BookModel.h"
#include "handlers.h"

DocBookReader::DocBookReader(BookModel &model, const std::string &encoding)
: myBookReader(model)
{
}

static const size_t maxBufferSize = 1024;

bool DocBookReader::readDocument(const std::string &fileName)
{
    myImageIndex = 0;
    myFootnoteIndex = 1;

    myBookReader.reset();
    myBookReader.setMainTextModel();
    myBookReader.pushKind(STRONG);
    myBookReader.beginParagraph();

    // create parser
    wvWare::SharedPtr<wvWare::Parser> parser( wvWare::ParserFactory::createParser( fileName ) );
    if ( !parser || !parser->isOk() )
    {
        std::cerr << "Error: Couldn't create a parser for this document" << std::endl;
        return false;
    }

    setBookReader(this);

    MyPictureHandler pic_handler;
    MyTextHandler text_handler;
    MyInlineReplacementHandler ir_handler;

    // Install handlers.
    parser->setPictureHandler( &pic_handler );
    parser->setTextHandler( &text_handler );
    parser->setInlineReplacementHandler( &ir_handler );

    if ( !parser->parse() )
    {
        std::cerr << "Error: The parser failed" << std::endl;
        return false;
    }
    std::cout << "Done." << std::endl;
    return true;
}
/*
void DocBookReader::setFontProperty(FontProperty property) {
    if (!myCurrentState.ReadText) {
        //DPRINT("change style not in text.\n");
        return;
    }
    flushBuffer();

    switch (property) {
        case FONT_BOLD:
            if (myState.Bold) {
                myBookReader.pushKind(STRONG);
            } else {
                myBookReader.popKind();
            }
            myBookReader.addControl(STRONG, myState.Bold);
            break;
        case FONT_ITALIC:
            if (myState.Italic) {
                if (!myState.Bold) {				
                    //DPRINT("add style emphasis.\n");
                    myBookReader.pushKind(EMPHASIS);
                    myBookReader.addControl(EMPHASIS, true);
                } else {
                    //DPRINT("add style emphasis and strong.\n");
                    myBookReader.popKind();
                    myBookReader.addControl(STRONG, false);

                    myBookReader.pushKind(EMPHASIS);
                    myBookReader.addControl(EMPHASIS, true);
                    myBookReader.pushKind(STRONG);
                    myBookReader.addControl(STRONG, true);
                }
            } else {
                if (!myState.Bold) {				
                    //DPRINT("remove style emphasis.\n");
                    myBookReader.addControl(EMPHASIS, false);
                    myBookReader.popKind();
                } else {
                    //DPRINT("remove style strong n emphasis, add strong.\n");
                    myBookReader.addControl(STRONG, false);
                    myBookReader.popKind();
                    myBookReader.addControl(EMPHASIS, false);
                    myBookReader.popKind();

                    myBookReader.pushKind(STRONG);
                    myBookReader.addControl(STRONG, true);
                }
            }
            break;
        case FONT_UNDERLINED:
            break;
    }
}

void DocBookReader::newParagraph() {
    flushBuffer();
    myBookReader.endParagraph();
    myBookReader.beginParagraph();
    if (myState.Alignment != ALIGN_UNDEFINED) {
        setAlignment();
    }
}

void DocBookReader::setEncoding(int) {
}

void DocBookReader::setAlignment() {
    ZLTextStyleEntry entry;
    entry.setAlignmentType(myState.Alignment);
    myBookReader.addControl(entry);
}
*/