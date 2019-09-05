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
		PROJECT_T Project{};
		Palette *pPalettes{nullptr};
		Icon *pIcons{nullptr};
		FontList *pFontLists{nullptr};
		bool status{false};
		bool localPalette{false};
		bool localIcon{false};
		bool localFontList{false};

		public:
			Panel() = default;
			Panel (PROJECT_T &prj, Palette *pPalet, Icon *pIco, FontList *pFL);
			~Panel();

			bool isOk() { return status; }
			std::vector<std::string> getPageFileNames();
			void readProject();

		protected:
			PROJECT_T& getProject() { return Project; }
			FontList *getFontList() { return pFontLists; }
			Palette *getPalettes() { return pPalettes; }
			Icon *getIconClass() { return pIcons; }

		private:
			void setVersionInfo(const std::string& name, const std::string& value);
			void setProjectInfo(const std::string& name, const std::string& value, const std::string& attr);
			void setSupportFileList(const std::string& name, const std::string& value);
			void setPanelSetup(const std::string& name, const std::string& value);
			void setPageList(const std::string& name, const std::string& value);
			void setResourceList(const std::string& name, const std::string& value, const std::string& attr);
			void setFwFeatureList(const std::string& name, const std::string& value);
			void setPaletteList(const std::string& name, const std::string& value);
			DateTime& getDate(const std::string& dat, DateTime& dt);
	};
}

#endif
