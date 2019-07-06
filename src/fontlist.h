/*
 * Copyright (C) 2018 by Andreas Theofilu <andreas@theosys.at>
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

#ifndef __FONTLIST_H__
#define __FONTLIST_H__

#include <vector>
#include "strings.h"

namespace amx
{
	typedef struct FONT
	{
		int number;
		strings::String file;
		int fileSize;
		int faceIndex;
		strings::String name;
		strings::String subfamilyName;
		strings::String fullName;
		int size;
		int usageCount;
	}FONT_T;

	class FontList
	{
		public:
			FontList(const strings::String& file);
			~FontList();

			strings::String getFontStyles();
			FONT_T& findFont(int idx);
			bool isOk() { return status; }
			strings::String getFontStyle(const strings::String& fs);
			strings::String getFontWeight(const strings::String& fw);
			bool serializeToJson();

		private:
			bool exist(const strings::String& ff);
            void fillSysFonts();

			std::vector<FONT_T> fontList;
			std::vector<strings::String> fontFaces;
			FONT_T emptyFont;
			bool status;
	};
}

#endif
