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

#include <string.h>
#include <stdlib.h>
#include "zlib.h"
#include <stdio.h>

#include <algorithm>

#include "../ZLInputStream.h"
#include "ZLZDecompressor.h"
#include <openssl/aes.h>
#include <openssl/evp.h>

#include <QByteArray>
#include <QDebug>

#define CHECK_ERR(err, msg) { \
    if (err != Z_OK) { \
        fprintf(stderr, "%s error: %d\n", msg, err); \
        exit(1); \
    } \
}

const size_t IN_BUFFER_SIZE = 2048;
const size_t OUT_BUFFER_SIZE = 32768;

static const size_t AES_KEY_LENGTH_BITS = 128;

ZLZDecompressor::ZLZDecompressor(size_t size) : myAvailableSize(size) {
	myZStream = new z_stream;
	memset(myZStream, 0, sizeof(z_stream));
	inflateInit2(myZStream, -MAX_WBITS);

	myInBuffer = new char[IN_BUFFER_SIZE];
	myOutBuffer = new char[OUT_BUFFER_SIZE];
}

ZLZDecompressor::~ZLZDecompressor() {
	delete[] myInBuffer;
	delete[] myOutBuffer;

	inflateEnd(myZStream);
	delete myZStream;
}

unsigned char *aes_decrypt(EVP_CIPHER_CTX *e, unsigned char *ciphertext, int *len)
{
    int p_len = *len, f_len = 0;
    unsigned char *plaintext = (unsigned char *)malloc(p_len);

    EVP_DecryptInit_ex(e, NULL, NULL, NULL, NULL);
    EVP_DecryptUpdate(e, plaintext, &p_len, ciphertext, *len);
    EVP_DecryptFinal_ex(e, plaintext + p_len, &f_len);

    *len = p_len + f_len;
    return plaintext;
}

void ZLZDecompressor::uncompress(Byte *compr, uLong comprLen, Byte *uncompr,
        uLong uncomprLen)
{
    int err;
    z_stream d_stream; /* decompression stream */

    strcpy((char*)uncompr, "garbage");

    d_stream.zalloc = (alloc_func)0;
    d_stream.zfree = (free_func)0;
    d_stream.opaque = (voidpf)0;

    d_stream.next_in  = compr;
    d_stream.avail_in = 0;
    d_stream.next_out = uncompr;

    err = inflateInit(&d_stream);
    CHECK_ERR(err, "inflateInit");

    while (d_stream.total_out < uncomprLen && d_stream.total_in < comprLen) {
        d_stream.avail_in = d_stream.avail_out = 1; /* force small buffers */
        err = inflate(&d_stream, Z_NO_FLUSH);
        if (err == Z_STREAM_END) break;
        CHECK_ERR(err, "inflate");
    }

    err = inflateEnd(&d_stream);
    CHECK_ERR(err, "inflateEnd");

}

size_t ZLZDecompressor::decompress(ZLInputStream &stream, char *buffer,
        size_t maxSize, const std::string &aesKey) {
	while ((myBuffer.length() < maxSize) && (myAvailableSize > 0)) {
		size_t size = std::min(myAvailableSize, (size_t)IN_BUFFER_SIZE);

		myZStream->next_in = (Bytef*)myInBuffer;
		myZStream->avail_in = stream.read(myInBuffer, size);

		if (myZStream->avail_in == size) {
			myAvailableSize -= size;
		} else {
			myAvailableSize = 0;
		}
		while (myZStream->avail_in == 0) {
			break;
		}
		while (myZStream->avail_in > 0) {
			myZStream->avail_out = OUT_BUFFER_SIZE;
			myZStream->next_out = (Bytef*)myOutBuffer;
			int code = ::inflate(myZStream, Z_SYNC_FLUSH);
			if ((code != Z_OK) && (code != Z_STREAM_END)) {
				break;
			}
			if (OUT_BUFFER_SIZE == myZStream->avail_out) {
				break;
			}
			myBuffer.append(myOutBuffer, OUT_BUFFER_SIZE - myZStream->avail_out);
			if (code == Z_STREAM_END) {
				myAvailableSize = 0;
				stream.seek(-myZStream->avail_in, false);
				break;
			}
		}
	}

	// decrypt myInBuffer if aesKey is not empty
    if (!aesKey.empty() && myBuffer.length() > 0)
    {
        qDebug("key not empty");
        unsigned char * key = (unsigned char *)aesKey.data();

        EVP_CIPHER_CTX d_ctx;
        EVP_CIPHER_CTX_init(&d_ctx);
        EVP_DecryptInit_ex(&d_ctx, EVP_aes_128_cbc(), NULL, key, key);

        char *gzcompressed;
        gzcompressed = (char *)aes_decrypt(&d_ctx,
                (unsigned char *)myBuffer.data(), &len);
        EVP_CIPHER_CTX_cleanup(&d_ctx);

        uLong comprLen = len;
        uLong uncomprLen = comprLen*2;

        Byte *uncompr  = (Byte*)calloc((uInt)uncomprLen, 1);

        uncompress((Byte *)gzcompressed, comprLen, uncompr, uncomprLen);
        free(gzcompressed);

        qDebug("inflate result: %s\n", uncompr);
        qDebug() << "inflate size: " << uncomprLen;

        myBuffer.clear();
        myBuffer.append((char *)uncompr, uncomprLen);
        free(uncompr);
    }

	size_t realSize = std::min(maxSize, myBuffer.length());
	if (buffer != 0) {
		memcpy(buffer, myBuffer.data(), realSize);
	}
	myBuffer.erase(0, realSize);
	return realSize;
}
