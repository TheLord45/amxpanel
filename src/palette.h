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

#ifndef __PALETTE_H__
#define __PALETTE_H__

#include <vector>
#include "panelstruct.h"

namespace amx
{
	typedef struct PDATA
	{
		int index;
		std::string name;
		unsigned long color;

		void clear()
		{
			index = 0;
			name.clear();
			color = 0;
		}
	}PDATA_T;

	class Palette
	{
		public:
			Palette();
			Palette(const std::string& file);
			Palette(const std::vector<PALETTE_T>& pal);
			Palette(const std::vector<PALETTE_T>& pal, const std::string& main);
			~Palette();

			void setPaletteFile(const std::string& f);
			bool parsePalette();
			bool parsePalette(const std::string& f);
			bool isOk() { return status; }
			unsigned long getColor(size_t idx);
			unsigned long getColor(const std::string& name);
			std::string colorToString(unsigned long col);
			std::string colorToSArray(unsigned long col);
			std::vector<PDATA_T>& getPalette() { return palette; }
			void setPalette(const std::vector<PDATA_T>& pd) { palette = pd; status = true; }
			std::string getJson();

		private:
			std::vector<PDATA_T> palette;
			std::vector<std::string> paletteFiles;
			bool status;
	};
}

#endif
