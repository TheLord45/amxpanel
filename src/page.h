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

#ifndef __PAGE_H__
#define __PAGE_H__

#include <vector>
#include "strings.h"
#include "panelstruct.h"
#include "pushbutton.h"

namespace amx
{
	class Page
	{
		public:
			Page();
			Page(const strings::String& file);
			~Page();

			void setFileName(const strings::String& f) { pageFile = f; }
			strings::String getFileName() { return pageFile; }
			bool parsePage();
			bool isOk() { return status; }
			strings::String& getStyleCode();
			strings::String& getWebCode();
			int getPageID() { return page.pageID; }
			strings::String& getPageName() { return page.name; }
			PAGETYPE getType() { return page.type; }
			strings::String& getGroupName() { return page.group; }
			strings::String& getScriptCode() { return scriptCode; }
			strings::String& getScriptStart() { return scrStart; }

			void setPalette(Palette *pal) { paletteClass = pal; }
//			void setPaletteFile(const strings::String& fname) { paletteFile = fname; }
			void setParentSize(int w, int h) { totalWidth = w; totalHeight = h; }
			void setFontClass(FontList *fl) { fontClass = fl; }
			void setPageList(const std::vector<PAGE_T>& p) { pgList = p; }
			void setProject(PROJECT_T *prj) { Project = prj; }

			int findPage(const strings::String& name);

		private:
			void clear();
			void generateButtons();
			strings::String isoToUTF(const strings::String& str);
			TEXT_ORIENTATION iToTo(int t);

			PAGE_T page;
			std::vector<PAGE_T> pgList;
			bool status;
			bool buttonsDone;
			bool styleDone;
			bool webDone;
			strings::String webBuffer;
			std::vector<strings::String> btWebBuffer;
			strings::String styleBuffer;
			strings::String scriptCode;
			strings::String scrStart;
			strings::String paletteFile;
			strings::String pageFile;
			int totalWidth;
			int totalHeight;
			FontList *fontClass;
			Palette *paletteClass;
			PROJECT_T *Project;
	};
}

#endif
