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

#include <algorithm>

#include <ZLFile.h>
#include <ZLStringUtil.h>
#include <ZLDir.h>
#include <ZLUnicodeUtil.h>
#include <ZLFileImage.h>

#include "OEBBookReader.h"
#include "NCXReader.h"
#include "../xhtml/XHTMLReader.h"
#include "../util/MiscUtil.h"
#include "../../bookmodel/BookModel.h"
#include <QDebug>
#include <QByteArray>
#include <QDate>
#include <openssl/rsa.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>

static const int AES_KEY_SIZE = 16;

OEBBookReader::OEBBookReader(BookModel &model)
    : myModelReader(model)
{
    aesKey = (char *)malloc(AES_KEY_SIZE+1);
    memset(aesKey, 0, AES_KEY_SIZE+1);
}

OEBBookReader::~OEBBookReader()
{
    if (aesKey)
    {
        delete aesKey;
    }
}

static const std::string MANIFEST = "manifest";
static const std::string SPINE = "spine";
static const std::string GUIDE = "guide";
static const std::string TOUR = "tour";
static const std::string SITE = "site";

static const std::string ITEM = "item";
static const std::string ITEMREF = "itemref";
static const std::string REFERENCE = "reference";

static const std::string DATE_FORMAT = "yyyy-MM-dd";

void OEBBookReader::startElementHandler(const char *tag, const char **xmlattributes) {
	const std::string tagString = ZLUnicodeUtil::toLower(tag);
	if (MANIFEST == tagString) {
		myState = READ_MANIFEST;
	} else if (SPINE == tagString) {
		const char *toc = attributeValue(xmlattributes, "toc");
		if (toc != 0) {
			myNCXTOCFileName = myIdToHref[toc];
		}
		myState = READ_SPINE;
	} else if (GUIDE == tagString) {
		myState = READ_GUIDE;
	} else if (TOUR == tagString) {
		myState = READ_TOUR;
	} else if ((myState == READ_MANIFEST) && (ITEM == tagString)) {
		const char *id = attributeValue(xmlattributes, "id");
		const char *href = attributeValue(xmlattributes, "href");
		if ((id != 0) && (href != 0)) {
			myIdToHref[id] = href;
		}
	} else if ((myState == READ_SPINE) && (ITEMREF == tagString)) {
		const char *id = attributeValue(xmlattributes, "idref");
		if (id != 0) {
			const std::string &fileName = myIdToHref[id];
			if (!fileName.empty()) {
				myHtmlFileNames.push_back(fileName);
			}
		}
	} else if ((myState == READ_GUIDE) && (REFERENCE == tagString)) {
		const char *type = attributeValue(xmlattributes, "type");
		const char *title = attributeValue(xmlattributes, "title");
		const char *href = attributeValue(xmlattributes, "href");
		if (href != 0) {
			if (title != 0) {
				myGuideTOC.push_back(std::pair<std::string,std::string>(title, href));
			}
			static const std::string COVER_IMAGE = "other.ms-coverimage-standard";
			if ((type != 0) && (COVER_IMAGE == type)) {
				myModelReader.setMainTextModel();
				myModelReader.addImageReference(href);
				myModelReader.addImage(href, shared_ptr<const ZLImage>(new ZLFileImage("image/auto", myFilePrefix + href, 0)));
			}
		}
	} else if ((myState == READ_TOUR) && (SITE == tagString)) {
		const char *title = attributeValue(xmlattributes, "title");
		const char *href = attributeValue(xmlattributes, "href");
		if ((title != 0) && (href != 0)) {
			myTourTOC.push_back(std::pair<std::string,std::string>(title, href));
		}
	}
}

void OEBBookReader::endElementHandler(const char *tag) {
	const std::string tagString = ZLUnicodeUtil::toLower(tag);
	if ((MANIFEST == tagString) || (SPINE == tagString) || (GUIDE == tagString) || (TOUR == tagString)) {
		myState = READ_NONE;
	}
}

std::string OEBBookReader::keyFileName(const std::string &oebFileName) const
{
    ZLFile oebFile = ZLFile(oebFileName);
    oebFile.forceArchiveType(ZLFile::ZIP);
    shared_ptr<ZLDir> zipDir = oebFile.directory(false);
    if (!zipDir) {
        return false;
    }

    std::string keyName("");
    std::vector<std::string> fileNames;
    zipDir->collectFiles(fileNames, false);
    for (std::vector<std::string>::const_iterator it = fileNames.begin(); it != fileNames.end(); ++it) {
        if (ZLStringUtil::stringEndsWith(*it, "key")) {
            keyName = zipDir->itemPath(*it);
        }
    }

    return keyName;
}

bool OEBBookReader::keyFileContent(const std::string name, char *buffer,
        size_t maxSize) const
{
    // Read content
    shared_ptr<ZLInputStream> inputStream = ZLFile(name).inputStream();
    if (inputStream->open())
    {
        size_t realSize = inputStream->read(buffer, maxSize);
        inputStream->close();
        if (realSize <= 0)
        {
            return false;
        }
    }
    else
    {
        return false;
    }
    return true;
}

std::string OEBBookReader::privateKey()
{
#ifdef WIN32
    return std::string("C:\\key.private");
#else
    return std::string("/usr/share/key.private");
#endif
}

bool OEBBookReader::rsaDecrypt(char *encryptedMessage, char *plain) const
{
    std::string privateKeyName = OEBBookReader::privateKey();
    FILE *keyFile = fopen(privateKeyName.c_str(), "r");
    RSA *privKey = PEM_read_RSAPrivateKey(keyFile, 0, 0, 0);
    fclose(keyFile);

    unsigned char inbuffer[RSA_size(privKey)];
    unsigned char outbuffer[RSA_size(privKey)];
    BIO *b64 = BIO_new(BIO_f_base64());
    BIO *bp = BIO_new_mem_buf(encryptedMessage, -1);
    bp = BIO_push(b64, bp);
    BIO_read(bp, inbuffer, RSA_size(privKey));
    BIO_free_all(bp);

    int len = RSA_private_decrypt(RSA_size(privKey), inbuffer, outbuffer,
            privKey, RSA_PKCS1_PADDING);
    outbuffer[len] = '\0';

    if (len > 1)
    {
        memcpy(plain, outbuffer, len+1);
    }
    else
    {
        return false;
    }

    RSA_free(privKey);
    return true;
}

QDate OEBBookReader::extractFromDate(char *plain)
{
    return QDate::fromString(QString::fromLatin1(plain, 10), DATE_FORMAT.c_str());
}

QDate OEBBookReader::extractToDate(char *plain)
{
    QString to(plain);
    to = to.mid(10, 10);
    return QDate::fromString(to, DATE_FORMAT.c_str());
}

bool OEBBookReader::checkValidDate(char *plain)
{
    QDate from = extractFromDate(plain);
    QDate to = extractToDate(plain);
    return (QDate::currentDate() >= from) && (QDate::currentDate() <= to);
}

/// Check key file in epub file format. This file is the validation source,
/// containing the valid date and AES key.
ZLFile::DRMStatus OEBBookReader::checkKeyFile(const std::string &path) const
{
    qDebug("path: %s", path.c_str());

    const int MAX_SIZE = 200;
    char buffer[MAX_SIZE];
    memset(buffer, 0, MAX_SIZE);
    std::string fileName = keyFileName(path);
    if (fileName.empty())
    {
        return ZLFile::NOT_DRM;
    }

    bool success = keyFileContent(fileName, buffer, MAX_SIZE);
    if (success)
    {
        QByteArray array(buffer);
        QByteArray converted = array.toBase64();
        int esize = converted.size();
        char encrypted[esize+1];
        memcpy(encrypted, converted.data(), esize);
        encrypted[esize] = '\n';

        char plain[MAX_SIZE];
        memset(plain, 0, MAX_SIZE);
        bool decrypted = rsaDecrypt(encrypted, plain);
        if (decrypted)
        {
            printf("plain: %s\n", plain);
            bool dateValid = checkValidDate(plain);
            qDebug() << "date valid? " << dateValid;
            if (dateValid)
            {
                char keyArray[AES_KEY_SIZE+1];
                const int startIndex = 10*2;
                for (int i=0; i<AES_KEY_SIZE; i++)
                {
                    keyArray[i] = plain[startIndex+i];
                }
                keyArray[AES_KEY_SIZE] = '\0';
                printf("aes key: %s\n", keyArray);
                strcpy(aesKey, keyArray);
            }
            else
            {
                return ZLFile::DRM_FAILED;
            }
        }
        else
        {
            return ZLFile::DRM_FAILED;
        }
    }
    else
    {
        return ZLFile::DRM_FAILED;
    }
    return ZLFile::DRM;
}

bool OEBBookReader::readBook(const std::string &origin_path, const std::string &fileName) {
	myFilePrefix = MiscUtil::htmlDirectoryPrefix(fileName);

	myIdToHref.clear();
	myHtmlFileNames.clear();
	myNCXTOCFileName.erase();
	myTourTOC.clear();
	myGuideTOC.clear();
	myState = READ_NONE;

	ZLFile::DRMStatus drmStatus = checkKeyFile(origin_path);
    qDebug() << "DRM status: " << drmStatus;
    if (ZLFile::DRM_FAILED == drmStatus)
    {
        return false;
    }

	if (!readDocument(fileName, std::string(aesKey))) {
		return false;
	}

	myModelReader.setMainTextModel();
	myModelReader.pushKind(REGULAR);

	XHTMLReader xhtmlReader(myModelReader);
	for (std::vector<std::string>::const_iterator it = myHtmlFileNames.begin(); it != myHtmlFileNames.end(); ++it) {
		if (it != myHtmlFileNames.begin()) {
			myModelReader.insertEndOfSectionParagraph();
		}
		xhtmlReader.readFile(myFilePrefix, *it, *it, std::string(aesKey));
	}

	generateTOC();

	return true;
}

void OEBBookReader::generateTOC() {
	if (!myNCXTOCFileName.empty()) {
		NCXReader ncxReader(myModelReader);
		if (ncxReader.readDocument(myFilePrefix + myNCXTOCFileName)) {
			const std::map<int,NCXReader::NavPoint> navigationMap = ncxReader.navigationMap();
			if (!navigationMap.empty()) {
				size_t level = 0;
				for (std::map<int,NCXReader::NavPoint>::const_iterator it = navigationMap.begin(); it != navigationMap.end(); ++it) {
					const NCXReader::NavPoint &point = it->second;
					int index = myModelReader.model().label(point.ContentHRef).ParagraphNumber;
					while (level > point.Level) {
						myModelReader.endContentsParagraph();
						--level;
					}
					while (++level <= point.Level) {
						myModelReader.beginContentsParagraph(-2);
						myModelReader.addContentsData("...");
					}
					myModelReader.beginContentsParagraph(index);
					myModelReader.addContentsData(point.Text);
				}
				while (level > 0) {
					myModelReader.endContentsParagraph();
					--level;
				}
				return;
			}
		}
	}

	std::vector<std::pair<std::string,std::string> > &toc = myTourTOC.empty() ? myGuideTOC : myTourTOC;
	for (std::vector<std::pair<std::string,std::string> >::const_iterator it = toc.begin(); it != toc.end(); ++it) {
		int index = myModelReader.model().label(it->second).ParagraphNumber;
		if (index != -1) {
			myModelReader.beginContentsParagraph(index);
			myModelReader.addContentsData(it->first);
			myModelReader.endContentsParagraph();
		}
	}
}
