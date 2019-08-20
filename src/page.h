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

#ifndef __PAGE_H__
#define __PAGE_H__

#include <vector>
#include "panelstruct.h"
#include "pushbutton.h"
#include "icon.h"

namespace amx
{
	class Page
	{
		public:
			Page();
			Page(const std::string& file);
			~Page();

			void setFileName(const std::string& f) { pageFile = f; }
			std::string getFileName() { return pageFile; }
			bool parsePage();
			bool isOk() { return status; }
			std::string& getStyleCode();
			std::string& getWebCode();
			int getPageID() { return page.pageID; }
			std::string& getPageName() { return page.name; }
			PAGETYPE getType() { return page.type; }
			std::string& getGroupName() { return page.group; }
			bool getModal() { return (page.modal == 0) ? false : true; }
			std::string& getScriptCode() { return scriptCode; }
			std::string& getScriptStart() { return scrStart; }
			std::string& getBtArray() { return btArray; }
			bool haveBtArray() { return !btArray.empty(); }
			std::string& getBargraphs() { return sBargraphs; }
			bool haveBargraphs() { return !sBargraphs.empty(); }
			PAGE_T& getPageData() { return page; }

			void setPalette(Palette *pal) { paletteClass = pal; }
			void setParentSize(int w, int h) { totalWidth = w; totalHeight = h; }
			void setFontClass(FontList *fl) { fontClass = fl; }
			void setIconClass(Icon *ic) { iconClass = ic; }
			void setPageList(const std::vector<PAGE_T>& p) { pgList = p; }
			void setProject(PROJECT_T *prj) { Project = prj; }

			void serializeToFile();

		private:
			void clear();
			void generateButtons();
			TEXT_ORIENTATION iToTo(int t);

			PAGE_T page;
			std::vector<PAGE_T> pgList;
			bool status;
			bool buttonsDone;
			bool styleDone;
			bool webDone;
			std::string webBuffer;
			std::vector<std::string> btWebBuffer;
			std::string styleBuffer;
			std::string scriptCode;
			std::string scrStart;
			std::string paletteFile;
			std::string pageFile;
			std::string btArray;
			std::string sBargraphs;
			int totalWidth;
			int totalHeight;
			FontList *fontClass;
			Palette *paletteClass;
			Icon *iconClass;
			PROJECT_T *Project;
	};
}

#endif
