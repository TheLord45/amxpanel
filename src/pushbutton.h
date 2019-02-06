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
			SCR_TYPE& getScriptType() { return scriptType; }
			strings::String& getScriptCodeStart() { return scrStart; }
			strings::String& getBargraphs() { return sBargraph; }
			bool haveBargraph() { return !sBargraph.empty(); }

			void setPageList(const std::vector<PAGE_T>& pl) { pageList = pl; }
			int findPage(const strings::String& name);
			bool getImageDimensions(const strings::String fname, int *width, int *height);

			/**
			 * \brief
			 * This function creates a new image consisting of a base image and
			 * a mask image.
			 * \note
			 * PNG images consist of four separate color channels (Alpha, Red,
			 * Green and Blue), and chameleon images use the button/state's Fill
			 * and Border color assignments to determine the colors used in the
			 * image as follows:
			 *
			 * <ul>
			 * <li>The Alpha channel (RGB = 0, 0, 0) defines the overall shape
			 * mask for the state. This is represented on the screen as black.
			 * Black (transparent) areas of the image will not be seen on the
			 * panel.</li>
			 * <li>The Red channel (RGB = 255, 0, 0) defines the area that will
			 * be filled with the state?s Fill color (set via the Fill Color
			 * State property).</li>
			 * <li>The Green channel (RGB = 0, 255, 0) defines the area that
			 * will be filled with the state?s Border color (set via the Border
			 * Color State property).</li>
			 * <li>The Blue channel is not used.</li>
			 * </ul>
			 *
			 * @param bm1
			 * Path and name of the file containing the base image.
			 *
			 * @param bm2
			 * Path and name of the file containing the mask image.
			 *
			 * @param fill
			 * The color used for the red channel.
			 *
			 * @param border
			 * The color used for the green channel.
			 *
			 * @return
			 * On success, the function returns the name of the newly created
			 * PNG image. Otherwise an empty string is returned.
			 */
			static strings::String createChameleonImage(const strings::String bm1, const strings::String bm2, unsigned long fill, unsigned long border);

		private:
			int blend(int base, int mask);
			/**
			 * Die Maske unter \a pix1 definiert über den roten und/oder grünen Farbkanal,
			 * welche Farbe verwendet wird. Ist der rote Farbkanal gesetzt, wird die
			 * Farbe unter \a fill zurückgegeben. Ist der grüne Farbkanal gesetzt, wird die
			 * Farbe unter \a border zurück gegeben. Der blaue Farbkanal wird nicht
			 * verwendet.
			 * Ist der Alpha-Kanal auf 0x7f (127) gesetzt und sowohl der rote als auch der
			 * grüne Farbkanal gleich 0, dann ist das Pixel nicht sichtbar.
			 * 
			 * Der Parameter \a pix2 definiert (optional) eine Maske. Diese
			 * besteht vorwiegend aus einem Alpha-Kanal und einem Wert pro
			 * Farbkanal, welcher ein Pixel dunkler erscheinen lassen kann.
			 * 
			 * Die beiden Paremeter \a pix1 und \a pix2 sind jeweils ein Pixel
			 * im Format einer \b GD Farbe.
			 * 
			 * @param pix1
			 * Maske die definiert welche Farbe zurückgegeben werden soll.
			 * 
			 * @param pix2
			 * Maske welche die Helligkeit oder Transparenz eines Pixels bestimmt.
			 * 
			 * @param fill
			 * Diese Farbe wird zurückgegeben, wenn in der Maske \a pix1 der
			 * rote Farbkanal > 0 gesetzt ist.
			 * 
			 * @param border
			 * Diese Farbe wird zurückgegeben, wenn in der Maske \a pix1 der
			 * grüne Farbkanal > 0 gesetzt ist.
			 * 
			 * @return
			 * Eine Farbe oder ein transparanetes Pixel.
			 */
			static int getBaseColor(int pix1, int pix2, int fill, int border);
			static int webColToGd(unsigned long col);

			BUTTON_T button;
			bool onOff;
			int state;
			FontList *fontClass;
			Icon *iconClass;
			int pageID;
			strings::String btName;
			strings::String scrStart;
			strings::String sBargraph;
			bool hScript;
			SCR_TYPE scriptType;
			std::vector<PAGE_T> pageList;
			std::vector<PDATA_T> palette;
			std::vector<int> tmpFiles;
	};
}

#endif
