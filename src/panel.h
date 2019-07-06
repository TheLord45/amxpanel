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

#ifndef __PANEL_H__
#define __PANEL_H__

#include "panelstruct.h"
#include "palette.h"
#include "icon.h"
#include "fontlist.h"

namespace amx
{
	class Panel
	{
		public:
			Panel();
			Panel(const PROJECT& prj, Palette *pPalet, Icon *pIco, FontList *pFL);
			~Panel();

			bool isOk() { return status; }
			std::vector<strings::String> getPageFileNames();

		protected:
			PROJECT_T& getProject() { return Project; }
			FontList *getFontList() { return pFontLists; }
			Palette *getPalettes() { return pPalettes; }
			Icon *getIconClass() { return pIcons; }

		private:
			void readProject();
			void setVersionInfo(const strings::String& name, const strings::String& value);
			void setProjectInfo(const strings::String& name, const strings::String& value, const strings::String& attr);
			void setSupportFileList(const strings::String& name, const strings::String& value);
			void setPanelSetup(const strings::String& name, const strings::String& value);
			void setPageList(const strings::String& name, const strings::String& value);
			void setResourceList(const strings::String& name, const strings::String& value, const strings::String& attr);
			void setFwFeatureList(const strings::String& name, const strings::String& value);
			void setPaletteList(const strings::String& name, const strings::String& value);
			DateTime& getDate(const strings::String& dat, DateTime& dt);

			PROJECT_T Project;
			Palette *pPalettes;
			Icon *pIcons;
			FontList *pFontLists;
			bool status;
	};
}

#endif
