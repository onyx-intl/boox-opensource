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

#ifndef __OEBBOOKREADER_H__
#define __OEBBOOKREADER_H__

#include <map>
#include <vector>
#include <string>

#include <ZLXMLReader.h>
#include <ZLFile.h>

#include "../../bookmodel/BookReader.h"
#include <QDate>

class OEBBookReader : public ZLXMLReader {

public:
	OEBBookReader(BookModel &model);
	~OEBBookReader();
	bool readBook(const std::string &origin_path, const std::string &fileName);

private:
	void startElementHandler(const char *tag, const char **attributes);
	void endElementHandler(const char *tag);

	void generateTOC();

	static std::string privateKey();

	bool rsaDecrypt(char *encryptedMessage, char *plain) const;
	std::string keyFileName(const std::string &oebFileName) const;
	bool keyFileContent(const std::string name, char *buffer,
	        size_t maxSize) const;
	ZLFile::DRMStatus checkKeyFile(const std::string &path) const;

    static QDate extractFromDate(char *plain);
    static QDate extractToDate(char *plain);
    static bool checkValidDate(char *plain);

private:
	enum ReaderState {
		READ_NONE,
		READ_MANIFEST,
		READ_SPINE,
		READ_GUIDE,
		READ_TOUR
	};

	BookReader myModelReader;
	ReaderState myState;

	std::string myFilePrefix;
	std::map<std::string,std::string> myIdToHref;
	std::vector<std::string> myHtmlFileNames;
	std::string myNCXTOCFileName;
	std::vector<std::pair<std::string,std::string> > myTourTOC;
	std::vector<std::pair<std::string,std::string> > myGuideTOC;

	char *aesKey;
};

#endif /* __OEBBOOKREADER_H__ */
