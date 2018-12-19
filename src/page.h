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
			Page(const strings::String& file);
			~Page();

			bool isOk() { return status; }
			strings::String& getStyleCode();
			strings::String& getWebCode();
			int getPageID() { return page.pageID; }
			strings::String& getPageName() { return page.name; }
			PAGETYPE getType() { return page.type; }
			strings::String& getGroupName() { return page.group; }

			void setPaletteFile(const strings::String& fname) { paletteFile = fname; }
			void setParentSize(int w, int h) { totalWidth = w; totalHeight = h; }
			void setFontClass(FontList *fl) { fontClass = fl; }

		private:
			void clear();
			void generateButtons();

			PAGE_T page;
			std::vector<PushButton> buttons;
			bool status;
			strings::String webBuffer;
			strings::String styleBuffer;
			strings::String paletteFile;
			int totalWidth;
			int totalHeight;
            FontList *fontClass;
	};
}

#endif
