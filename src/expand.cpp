/*
 * Copyright (C) 2019 by Andreas Theofilu <andreas@theosys.at>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

#include <iostream>
#include <fstream>
#if __GNUC__ < 9
   #if __cplusplus < 201703L
      #warning "Your C++ compiler seems to have no support for C++17 standard!"
   #endif
   #include <experimental/filesystem>
   namespace fs = std::experimental::filesystem;
#else
   #include <filesystem>
   namespace fs = std::filesystem;
#endif
#include "expand.h"
#include <assert.h>

using namespace std;

void Expand::setFileName (const string &fn)
{
	fname.assign(fn);
}

int Expand::unzip()
{
	if (fname.empty())
		return -1;

	string target(fname+".temp");
	FILE *source, *dest;

	source = fopen(fname.c_str(), "rb");

	if (!source)
	{
		cerr << "Error opening file" << fname << "!" << endl;
		return -1;
	}

	dest = fopen(target.c_str(), "wb");

	if (!dest)
	{
		fclose(source);
		cerr << "Error creating the temporary file " << target << "!" << endl;
		return -1;
	}

	int ret;
	unsigned have;
	z_stream strm;
	unsigned char in[CHUNK];
	unsigned char out[CHUNK];

	// allocate inflate state
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	ret = inflateInit2(&strm, 32+MAX_WBITS);

	if (ret != Z_OK)
	{
		zerr(ret);
		fclose(source);
		fclose(dest);
		fs::remove(target);
		return ret;
	}

	do
	{
		strm.avail_in = fread(in, 1, CHUNK, source);

		if (ferror(source))
		{
			(void)inflateEnd(&strm);
			cerr << "Error reading from file " << fname << "!" << endl;
			fclose(source);
			fclose(dest);
			fs::remove(target);
			return Z_ERRNO;
		}

		if (strm.avail_in == 0)
			break;

		strm.next_in = in;
		// run inflate() on input until output buffer not full
        do
		{
			strm.avail_out = CHUNK;
			strm.next_out = out;
			ret = inflate(&strm, Z_NO_FLUSH);
			assert(ret != Z_STREAM_ERROR);	// state not clobbered

			switch (ret)
			{
				case Z_NEED_DICT:
					ret = Z_DATA_ERROR;		// and fall through
					// fall through
				case Z_DATA_ERROR:
				case Z_MEM_ERROR:
					(void)inflateEnd(&strm);
					fclose(source);
					fclose(dest);
					fs::remove(target);
					zerr(ret);
					return ret;
			}

			have = CHUNK - strm.avail_out;

			if (fwrite(out, 1, have, dest) != have || ferror(dest))
			{
				(void)inflateEnd(&strm);
				cerr << "Error on writing to file " << target << "!" << endl;
				fclose(source);
				fclose(dest);
				fs::remove(target);
				return Z_ERRNO;
			}
		}
		while (strm.avail_out == 0);
		// done when inflate() says it's done
	}
	while (ret != Z_STREAM_END);
	// clean up and return
	(void)inflateEnd(&strm);
	fclose(source);
	fclose(dest);
	fs::remove(fname);
	fs::rename(target, fname);
	return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

void Expand::zerr(int ret)
{
    switch (ret)
	{
		case Z_ERRNO:
			cerr << "Error reading or writing a file!" << endl;
		break;

		case Z_STREAM_ERROR:
			cerr << "invalid compression level" << endl;
		break;

		case Z_DATA_ERROR:
			cerr << "invalid or incomplete deflate data" << endl;
		break;

		case Z_MEM_ERROR:
			cerr << "out of memory" << endl;
		break;

		case Z_VERSION_ERROR:
			cerr << "zlib version mismatch!" << endl;
		break;

		default:
			if (ret != Z_OK)
				cerr << "Unknown error " << to_string(ret) << "!" << endl;
    }
}
