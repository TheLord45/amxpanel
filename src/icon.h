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

#ifndef __ICON_H__
#define __ICON_H__

#include <vector>
#include "strings.h"

namespace amx
{
	typedef struct ICON
	{
		int index;
		strings::String file;
        int width;
        int height;
	}ICON_T;

	class Icon
	{
		public:
			Icon(const strings::String& file);
			~Icon();

			bool isOk() { return status; }
			strings::String getFileName(size_t idx);
			int getID(size_t idx);
			int getWidth(size_t idx);
			int getHeight(size_t idx);
			strings::String getFileFromID(int id);
			size_t numIcons() { return icons.size(); }

		private:
			std::vector<ICON_T> icons;
			bool status;
	};
}

#endif
