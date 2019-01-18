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

#ifndef __PUSHBUTTON_H__
#define __PUSHBUTTON_H__

#include "panelstruct.h"
#include "strings.h"
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

			void setSwitch(bool s) { onOff = s; }
			void setState(size_t s);
			void setFontClass(FontList *fl) { fontClass = fl; }
			void setIconClass(Icon *ic) { iconClass = ic; }
			void setPageID(int id) { pageID = id; }
			void setPalette(const std::vector<PDATA_T>& pal) { palette = pal; }
			strings::String getStyle();
			strings::String getWebCode();
			strings::String getScriptCode();
			bool haveScript() { return hScript; }
			SCR_TYPE getScriptType() { return scriptType; }
			strings::String getScriptCodeStart() { return scrStart; }

			void setPageList(const std::vector<PAGE_T>& pl) { pageList = pl; }
			int findPage(const strings::String& name);
			bool getImageDimensions(const strings::String fname, int *width, int *height);
			strings::String createChameleonImage(const strings::String bm1, const strings::String bm2);

		private:
			BUTTON_T button;
			bool onOff;
			int state;
			FontList *fontClass;
			Icon *iconClass;
			int pageID;
			strings::String btName;
			strings::String scrStart;
			bool hScript;
			SCR_TYPE scriptType;
			std::vector<PAGE_T> pageList;
			std::vector<PDATA_T> palette;
	};
}

#endif
