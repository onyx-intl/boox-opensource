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

#ifndef __MOBIPOCKETSTREAM_H__
#define __MOBIPOCKETSTREAM_H__

#include "PalmDocStream.h"

class ZLFile;

class MobipocketStream : public PalmDocStream {

public:
	MobipocketStream(ZLFile &file);
	bool open();

	const std::string &error() const;

	std::pair<int,int> imageLocation(int index);

	bool hasExtraSections() const;

private:
    bool isPdb() { return is_pdb_; }

private:
	int myBaseSize;
	enum {
		ERROR_NONE,
		ERROR_UNKNOWN,
		ERROR_COMPRESSION,
		ERROR_ENCRIPTION,
	} myErrorCode;

    bool is_pdb_;

    struct MobiHeader
    {
        unsigned char identifier[4];
        unsigned int  header_len;
        unsigned int  mobi_type;
        unsigned int  text_encoding;
        unsigned int  unique_id;
        unsigned int  ver1;             // ? Not sure.
        unsigned char reserved2[40];
        unsigned int  first_non_book_index;
        unsigned int  full_name_offset;
        unsigned int  full_name_len;
        unsigned int  language;
        unsigned char unknown1[8];
        unsigned int  ver2;
        unsigned int  first_image_index;
        unsigned char unknown2[16];
        unsigned int  exth_flag;
    };
    MobiHeader mobiHeader;
};

#endif /* __MOBIPOCKETSTREAM_H__ */
