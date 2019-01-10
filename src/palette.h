/*
 * Copyright (C) 2018 by Andreas Theofilu <andreas@theosys.at>
 *
 * All rights reserved. No warranty, explicit or implicit, provided.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of Andreas Theofilu and his suppliers, if any.
 * The intellectual and technical concepts contained
 * herein are proprietary to Andreas Theofilu and its suppliers and
 * may be covered by European and Foreign Patents, patents in process,
 * and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Andreas Theofilu.
 */

#ifndef __PALETTE_H__
#define __PALETTE_H__

#include <vector>
#include "strings.h"
#include "panelstruct.h"

namespace amx
{
	typedef struct PDATA
	{
		int index;
		strings::String name;
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
			Palette(const strings::String& file);
			Palette(const std::vector<PALETTE_T>& pal);
			Palette(const std::vector<PALETTE_T>& pal, const strings::String& main);
			~Palette();

			void setPaletteFile(const strings::String& f);
			bool parsePalette();
			bool parsePalette(const strings::String& f);
			bool isOk() { return status; }
			unsigned long getColor(size_t idx);
			unsigned long getColor(const strings::String& name);
			strings::String colorToString(unsigned long col);
			std::vector<PDATA_T>& getPalette() { return palette; }
			void setPalette(const std::vector<PDATA_T>& pd) { palette = pd; status = true; }

		private:
			std::vector<PDATA_T> palette;
			std::vector<strings::String> paletteFiles;
			bool status;
	};
}

#endif
