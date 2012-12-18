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

#include <ZLStringUtil.h>
#include <ZLFile.h>
#include <ZLInputStream.h>

#include "DocPlugin.h"
#include "DocBookReader.h"

#include "../../description/BookDescription.h"


bool DocPlugin::providesMetaInfo() const {
    return false;
}

bool DocPlugin::acceptsFile(const ZLFile &file) const {
    return file.extension() == "doc";
}

bool DocPlugin::readDescription(const std::string &path, BookDescription &description) const {
    /*
    ZLFile file(path);
    //fb::shared_ptr<ZLInputStream> stream = file.inputStream();
    fb::shared_ptr<ZLInputStream> stream = new RtfReaderStream(path, 50000);

    if (stream.isNull()) {
        return false;
    }

    detectEncodingAndLanguage(book, *stream);

    if (!RtfDescriptionReader(book).readDocument(path)) {
        return false;
    }
    */

    return true;
}

bool DocPlugin::readModel(const BookDescription &desc, BookModel &model) const {
    return DocBookReader(model, desc.encoding()).readDocument(desc.fileName());
}

const std::string &DocPlugin::iconName() const {
    static const std::string ICON_NAME = "doc";
    return ICON_NAME;
}

FormatInfoPage *DocPlugin::createInfoPage(ZLOptionsDialog&, const std::string&) {
    return 0;
}
