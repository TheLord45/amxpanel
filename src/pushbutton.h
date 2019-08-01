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

#ifndef __PUSHBUTTON_H__
#define __PUSHBUTTON_H__

#include "panelstruct.h"
#include "palette.h"
#include "fontlist.h"
#include "icon.h"
#include "systemreserved.h"

namespace amx
{
	class PushButton : public SystemReserved
	{
		public:
			enum SCR_TYPE
			{
				SCR_NONE,
				SCR_TIME_STANDARD,
				SCR_TIME_AMPM,
				SCR_TIME_24,
				SCR_DATE_WEEKDAY,
				SCR_DATE_D_MONTH_Y,
				SCR_DATE_D_M,
				SCR_DATE_D_M_Y,
				SCR_DATE_M_D,
				SCR_DATE_M_D_Y,
				SCR_DATE_MONTH_D_Y,
				SCR_DATE_Y_M_D
			};

			PushButton(const BUTTON_T& bt, const std::vector<PDATA_T>& pal);
			~PushButton();

			void setFontClass(FontList *fl) { fontClass = fl; }
			void setIconClass(Icon *ic) { iconClass = ic; }
			void setPageID(int id) { pageID = id; }
			void setPalette(const std::vector<PDATA_T>& pal) { palette = pal; }
			std::string getStyle();
			std::string getWebCode();
			std::string getScriptCode();
			bool haveScript() { return hScript; }
			SCR_TYPE& getScriptType() { return scriptType; }
			std::string& getScriptCodeStart() { return scrStart; }
			std::string& getBargraphs() { return sBargraph; }
			bool haveBargraph() { return !sBargraph.empty(); }

			void setPageList(const std::vector<PAGE_T>& pl) { pageList = pl; }
			static bool getImageDimensions(const std::string fname, int *width, int *height);

		private:
			BUTTON_T button;
			FontList *fontClass;
			Icon *iconClass;
			int pageID;
			std::string btName;
			std::string scrStart;
			std::string sBargraph;
			bool hScript;
			SCR_TYPE scriptType;
			std::vector<PAGE_T> pageList;
			std::vector<PDATA_T> palette;
	};
}

#endif
