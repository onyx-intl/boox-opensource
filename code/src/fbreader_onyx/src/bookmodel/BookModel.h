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

#ifndef __BOOKMODEL_H__
#define __BOOKMODEL_H__

#include <map>
#include <string>

#include <ZLTextModel.h>
#include <ZLTextParagraph.h>
#include <ZLUserData.h>
#include "../description/BookDescription.h"

class ZLImage;

class ContentsModel : public ZLTextTreeModel {

public:
	void setReference(const ZLTextTreeParagraph *paragraph, int reference);
	int reference(const ZLTextTreeParagraph *paragraph) const;

private:
	std::map<const ZLTextTreeParagraph*,int> myReferenceByParagraph;
};

class BookModel : public ZLUserDataHolder {

public:
	struct Label {
		Label(shared_ptr<ZLTextModel> model, int paragraphNumber) : Model(model), ParagraphNumber(paragraphNumber) {}

		const shared_ptr<ZLTextModel> Model;
		const int ParagraphNumber;
	};

    enum OpenStatus {
        OPEN_NORMAL = 0x01,
        OPEN_DATE_INVALID = 0xFD,
        OPEN_DECRYPTION_FAILED = 0xFE,
        OPEN_UNKNOWN_ERROR = 0xFF,
    };

public:
	BookModel(const BookDescriptionPtr description);
	~BookModel();

	const std::string &fileName() const;

	shared_ptr<ZLTextModel> bookTextModel() const;
	shared_ptr<ZLTextModel> contentsModel() const;

	const ZLImageMap &imageMap() const;
	Label label(const std::string &id) const;

	const BookDescriptionPtr description() const;

	// for DRM content
	bool drm() const;
	OpenStatus openStatus() const;
	void setDRM(bool isDRM);
	void setOpenStatus(OpenStatus openStatus);

private:
	const BookDescriptionPtr myDescription;
	shared_ptr<ZLTextModel> myBookTextModel;
	shared_ptr<ZLTextModel> myContentsModel;
	ZLImageMap myImages;
	std::map<std::string,shared_ptr<ZLTextModel> > myFootnotes;
	std::map<std::string,Label> myInternalHyperlinks;
	bool isDRM;
	OpenStatus myOpenStatus;

friend class BookReader;
};

inline shared_ptr<ZLTextModel> BookModel::bookTextModel() const { return myBookTextModel; }
inline shared_ptr<ZLTextModel> BookModel::contentsModel() const { return myContentsModel; }
inline const ZLImageMap &BookModel::imageMap() const { return myImages; }
inline const BookDescriptionPtr BookModel::description() const { return myDescription; }
inline bool BookModel::drm() const { return isDRM; }
inline BookModel::OpenStatus BookModel::openStatus() const
{
    return myOpenStatus;
}

#endif /* __BOOKMODEL_H__ */
