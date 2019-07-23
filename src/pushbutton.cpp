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

#include "syslog.h"
#include "nameformat.h"
#include "config.h"
#include "pushbutton.h"
#include <gd.h>
#include <iomanip>

extern Syslog *sysl;
extern Config *Configuration;

using namespace amx;
using namespace strings;

PushButton::PushButton(const BUTTON_T& bt, const std::vector<PDATA_T>& pal)
		: button(bt),
		  palette(pal)

{
	sysl->TRACE(Syslog::ENTRY, std::string("PushButton::PushButton(const BUTTON_T& bt, const std::vector<PDATA_T>& pal)"));
	sysl->TRACE(String("PushButton::PushButton: Button: ")+bt.na+", ID: "+bt.bi);
	fontClass = 0;
	iconClass = 0;

	if (button.ap == 0 && isSystemReserved(button.ad))
		btName = getButtonName(button.ad);
	else
		btName = String("Button_")+bt.bi;

	hScript = false;
	scriptType = SCR_NONE;
}

PushButton::~PushButton()
{
	sysl->TRACE(Syslog::EXIT, std::string("PushButton::PushButton(...)"));
}

String PushButton::getStyle()
{
	sysl->TRACE(Syslog::MESSAGE, std::string("PushButton::getStyle()"));
	sysl->TRACE(String("PushButton::getStyle: for page ID: ")+pageID);

	String style, bgcolor, fgcolor, fillcolor;
	Palette pal;
	pal.setPalette(palette);

	for (size_t i = 0; i < button.sr.size(); i++)
	{
		bool hasChameleon = (!button.sr[i].mi.empty() && !button.sr[i].bm.empty() && button.sr[i].bs.empty());
		bool hasBorder = !button.sr[i].bs.empty();

		if (hasBorder || button.type == BARGRAPH)
			hasChameleon = false;

		if (button.ap == 0 && isSystemReserved(button.ad))
			style += String(".")+btName+button.sr[i].number+" {\n";
		else	// Name: .button<number>_b<id>_<name>
			style += String(".Page_")+pageID+"_"+btName+"_"+button.sr[i].number+" {\n";

		style += "  position: absolute;\n";
		style += String("  left: ")+String(button.lt)+"px;\n";
		style += String("  top: ")+String(button.tp)+"px;\n";
		style += String("  width: ")+String(button.wt)+"px;\n";
		style += String("  height: ")+String(button.ht)+"px;\n";

//		if (button.zo > 0)
//			style += String("  z-index: ")+button.zo+";\n";

		if (button.hs.caseCompare("bounding") == 0)
			style += String("  overflow: hidden;\n");

		if (hasBorder)
		{
			style += getBorderStyle(button.sr[i].bs);
			style += String("  border-color: ")+pal.colorToString(pal.getColor(button.sr[i].cb))+";\n";
		}
		else
			style += "  border: none;\n";

/*		if (hasChameleon)	// Chameleon image?
		{
			String fname = createChameleonImage(Configuration->getHTTProot()+"/images/"+button.sr[i].mi, Configuration->getHTTProot()+"/images/"+button.sr[i].bm, pal.getColor(button.sr[i].cf), pal.getColor(button.sr[i].cb));

			if (!fname.empty())
				style += String("  background-image: url(")+fname+");\n";
			else
			{
				style += String("  background-image: url(images/")+NameFormat::toURL(button.sr[i].mi)+"), ";
				style += String("url(images/")+NameFormat::toURL(button.sr[i].bm)+");\n";
				style += "  background-blend-mode: screen;\n";
			}
		}
		else */if (!hasChameleon && button.sr[i].bm.length() && button.type != BARGRAPH)
		{
			style += String("  background-image: url(images/")+NameFormat::toURL(button.sr[i].bm)+");\n";
			style += "  background-repeat: no-repeat;\n";
		}

		if ((button.fb == FB_MOMENTARY && i == 0) || (button.fb == FB_ALWAYS_ON && i == 1) ||
				(button.fb == FB_INV_CHANNEL && i == 1) || (button.fb == FB_CHANNEL && i == 0) ||
				(button.fb == FB_NONE && i == 0))
			style += "  display: inline;\n";
		else
			style += "  display: none;\n";

		if (!hasChameleon || hasBorder)
			style += String("  background-color: ")+pal.colorToString(pal.getColor(button.sr[i].cf))+";\n";

		style += String("  color: ")+pal.colorToString(pal.getColor(button.sr[i].ct))+";\n";
		style += "}\n";

		FONT_T font = fontClass->findFont(button.sr[i].fi);

/*		if (font.number == button.sr[i].fi)
		{
			if (button.ap == 0 && isSystemReserved(button.ad))
				style += String(".")+btName+button.sr[i].number+"_font {\n";
			else
				style += String(".Page_")+pageID+"_"+btName+"_"+button.sr[i].number+"_font {\n";

			if (!button.sr[i].bs.empty())
			{
				style += getBorderStyle(button.sr[i].bs);
				style += "  border-color: rgba(0, 0, 0, 0);\n";
			}
			else
				style += "  border: none;\n";

			style += "  position: absolute;\n";

			if (button.sr[i].jt != ORI_ABSOLUT)
			{
				style += String("  left: 0px;\n");
				style += String("  top: 0px;\n");
			}

			style += String("  width: ")+button.wt+"px;\n";
			style += String("  height: ")+button.ht+"px;\n";
			style += String("  font-family: \"")+font.name+"\";\n";
			style += String("  font-size: ")+String(font.size)+"pt;\n";
			style += String("  font-style: ")+fontClass->getFontStyle(font.subfamilyName)+";\n";
			style += String("  font-weight: ")+fontClass->getFontWeight(font.subfamilyName)+";\n";

//			if (!button.sr[i].ww && (button.sr[i].jt == ORI_CENTER_LEFT || button.sr[i].jt == ORI_CENTER_MIDDLE || button.sr[i].jt == ORI_CENTER_RIGHT))
//				style += String("  line-height: ")+button.ht+"px;\n";

			switch(button.sr[i].jt)
			{
				case ORI_ABSOLUT:
					style += String("  left: ")+button.sr[i].tx+"px;\n";
					style += String("  top: ")+button.sr[i].ty+"px;\n";
				break;

				case ORI_TOP_LEFT:		style += "  text-align: left;\n  vertical-align: top;\n"; break;
				case ORI_TOP_MIDDLE:	style += "  text-align: center;\n  vertical-align: top;\n"; break;
				case ORI_TOP_RIGHT:		style += "  text-align: right;\n  vertical-align: top;\n"; break;
				case ORI_CENTER_LEFT:	style += "  text-align: left;\n  vertical-align: middle;\n"; break;
				case ORI_CENTER_MIDDLE:	style += "  text-align: center;\n  vertical-align: middle;\n"; break;
				case ORI_CENTER_RIGHT:	style += "  text-align: right;\n  vertical-align: middle;\n"; break;
				case ORI_BOTTOM_LEFT:	style += "  text-align: left;\n  vertical-align: bottom;\n"; break;
				case ORI_BOTTOM_MIDDLE:	style += "  text-align: center;\n  vertical-align: bottom;\n"; break;
				case ORI_BOTTOM_RIGHT:	style += "  text-align: right;\n  vertical-align: bottom;\n"; break;
			}

			style += "}\n";
		} */
	}

	return style;
}

String PushButton::getWebCode()
{
	sysl->TRACE(String("PushButton::getWebCode()"));
	String code, names;

	sysl->TRACE(String("PushButton::getWebCode: for page ID: ")+pageID);

	if ((button.type == GENERAL && button.sr.size() >= 2) || button.type == BARGRAPH)
	{
		if (button.ap == 0 && isSystemReserved(button.ad))
		{
			names = btName+button.sr[0].number+"',";
			names += btName+button.sr[1].number+"'";
		}
		else
		{
			names = String("'Page")+pageID+"_b"+button.sr[0].number+"_"+btName+"',";
			names += String("'Page")+pageID+"_b"+button.sr[1].number+"_"+btName+"'";
		}
	}

	if (button.type == BARGRAPH)
	{
		int level;

		if (!sBargraph.empty())
			sBargraph += ",\n";

		if (button.rh > button.rl)
			level = button.rl;	// Initial value is lowest possible
		else
			level = button.rl;

		String nm;

		if (button.ap == 0 && isSystemReserved(button.ad))
			nm = btName;
		else
			nm = String("Page")+pageID+"_"+btName;

		sBargraph += String("\t{\"name\":\"")+nm+"\",\"pnum\":"+pageID+", \"bi\":"+button.bi+",\"ap\":"+button.ap+",\"ac\":"+button.ad;
		sBargraph += String(",\"cp\":")+button.cp+",\"ch\":"+button.ch+",\"lp\":"+button.lp+",\"lc\":"+button.lv;
		sBargraph += String(",\"rl\":")+button.rl+",\"rh\":"+button.rh+",\"lv\":"+button.lv+",\"level\":"+level;
		sBargraph += String(",\"dr\":\"")+button.dr+"\",\"states\":[\n";

		for (size_t i = 0; i < button.sr.size(); i++)
		{
			if (i > 0)
				sBargraph += ",\n";

			int mi_width, mi_height, bm_width, bm_height;

			if (button.sr[i].mi.length() > 0)
				getImageDimensions(Configuration->getHTTProot()+"/images/"+button.sr[i].mi, &mi_width, &mi_height);
			else
			{
				mi_width = 0;
				mi_height = 0;
			}

			if (button.sr[i].bm.length() > 0)
				getImageDimensions(Configuration->getHTTProot()+"/images/"+button.sr[i].bm, &bm_width, &bm_height);
			else
			{
				bm_width = 0;
				bm_height = 0;
			}

			sBargraph += String("\t\t{\"mi\":\"")+button.sr[i].mi+"\",\"mi_width\":"+mi_width+",\"mi_height\":"+mi_height+",\"cb\":\""+button.sr[i].cb+"\"";
			sBargraph += String(",\"cf\":\"")+button.sr[i].cf+"\",\"bm\":\""+button.sr[i].bm+"\",\"bm_width\":"+bm_width+",\"bm_height\":"+bm_height+"}";
		}

		sBargraph += "]}";
	}

	if (button.ap == 0 && isSystemReserved(button.ad))
		code = String("   <a href=\"#\" id=\"")+btName+"\">\n";
	else
		code = String("   <a href=\"#\" id=\"Page")+pageID+btName+"\">\n";

	for (size_t i = 0; i < button.sr.size(); i++)
	{
		// FIXME: Die unterschiedlichen button status berücksichtigen
		String nm;

		if (button.ap == 0 && isSystemReserved(button.ad))
			nm = btName+button.sr[i].number;
		else
			nm = String("Page")+pageID+"_b"+button.sr[i].number+"_"+btName;

		code += String("      <div id=\"")+nm+"\" class=\""+nm+"\"";

		if (button.pushFunc.size() > 0)
		{
			code += " onclick=\"";

			for (size_t j = 0; j < button.pushFunc.size(); j++)
			{
				sysl->TRACE(String("PushButton::getWebCode: Button ")+button.na+" show/hide popup page "+button.pushFunc[j].pfName+".");

				if (button.pushFunc[j].pfType.caseCompare("sShow") == 0)		// show popup
					code += String("showPopup('")+button.pushFunc[j].pfName+"');";
				else if (button.pushFunc[j].pfType.caseCompare("sHide") == 0)	// hide popup
					code += String("hidePopup('")+button.pushFunc[j].pfName+"');";
				else if (button.pushFunc[j].pfType.caseCompare("scGroup") == 0)	// hide group
					code += String("hideGroup('")+button.pushFunc[j].pfName+"');";
			}

			code += "\"";
		}
		else if (((button.type == GENERAL && button.fb == FB_MOMENTARY && (i == 0 || i == 1)) ||
				button.type == BARGRAPH) && button.cp > 0 && button.ch > 0)
		{
			code += String(" onmousedown=\"switchDisplay(")+names+",1,"+button.cp+","+button.ch+");\"";
			code += String(" onmouseup=\"switchDisplay(")+names+",0,"+button.cp+","+button.ch+");\"";
		}

		code += ">\n";

		if (button.type == GENERAL && !button.sr[i].mi.empty() && !button.sr[i].bm.empty())
		{
			int width, height;
			Palette pal;

			pal.setPalette(palette);
			getImageDimensions(Configuration->getHTTProot()+"/images/"+button.sr[i].mi, &width, &height);
			unsigned long col1 = pal.getColor(button.sr[i].cf);
			unsigned long col2 = pal.getColor(button.sr[i].cb);
			code += String("         <canvas id=\"")+nm+"_canvas\" width=\""+width+"px\" height=\""+height+"px\" style=\"";
			code += "position: absolute;left: 0px;top: 0px;\"></canvas>\n";
			code += "         <script>\n";
			code += "            var visProp = getHiddenProp();\n";
			code += "            if (visProp) {\n";
			code += "               var evtname = visProp.replace(/[H|h]idden/,'') + 'visibilitychange';\n";
			code += String("               document.getElementById('Page_")+pageID+"').addEventListener(evtname,function() { ";
			code += String("if (isVisible(document.getElementById('Page_")+pageID+"'))) drawButton(makeURL('images/"+button.sr[i].mi+"'),makeURL('images/"+button.sr[i].bm+"'),'"+nm+"',"+width+","+height+","+pal.colorToSArray(col1)+","+pal.colorToSArray(col2)+"); });\n";
			code += "            }\n         </script>\n";
		}

		if (iconClass != 0 && button.sr[i].ii > 0)		// Icon?
		{
			int width, height;
			String icoFile = iconClass->getFileFromID(button.sr[i].ii);
			bool stat = getImageDimensions(Configuration->getHTTProot()+"/images/"+icoFile, &width, &height);

			code += String("         <img id=\"")+nm+"_icon\" src=\"images/"+icoFile+"\"";

			if (button.sr[i].ji == 0)
			{
				if (stat)
				{
					sysl->TRACE(String("PushButton::getWebCode: width=")+width+", height="+height);

					if ((button.sr[i].ix + width) > button.wt)
					{
						button.sr[i].ix = (button.wt - width);

						if (button.sr[i].ix < 0)
						{
							button.sr[i].ix = 0;
							code += String(" width=\"")+button.wt+"px\"";
						}
						else
							code += String(" width=\"")+width+"px\"";
					}
					else
						code += String(" width=\"")+width+"px\"";

					if ((button.sr[i].iy + height) > button.ht)
					{
						button.sr[i].iy = (button.ht - height);

						if (button.sr[i].iy < 0)
						{
							button.sr[i].iy = 0;
							code += String(" height=\"")+button.ht+"px\"";
						}
						else
							code += String(" height=\"")+height+"px\"";
					}
					else
						code += String(" height=\"")+height+"px\"";
				}
			}

			switch(button.sr[i].ji)			// Icon justification
			{
				case 0:	code += String(" style=\"position: absolute;left: ")+button.sr[i].ix+"px;top: "+button.sr[i].iy+"px;\""; break;
				case 1:	code += " style=\"position: absolute;left: 0px;top: 0px;\""; break;

				default:
					code += String(" onload=\"posImage(this,'")+nm+"',"+button.sr[i].ji+");\"";
			}

			code += ">\n";
		}

		if (button.sr[i].te.length() > 0)
		{
			code += String("         <span id=\"")+nm+"_font\" class=\""+nm+"_font\">";
			code += NameFormat::textToWeb(button.sr[i].te)+"</span>\n";
		}
		else			// A ^TXT command could send text to this field
			code += String("         <span id=\"")+nm+"_font\" class=\""+nm+"_font\"></span>\n";

		code += "      </div>\n";
	}

	code += "   </a>\n";

	return code;
}

String PushButton::getScriptCode()
{
	sysl->TRACE(String("PushButton::getScriptCode()"));

	if (button.ap != 0)
		return "";

	String code;

	// This is for displaying the time
	if (button.ad == 141)		// System time standard
	{
		code += String("function ")+getFuncName(button.ad)+"() {\n";
		code += "\tvar today = new Date();\n";
		code += "\tvar h = today.getHours();\n";
		code += "\tvar m = today.getMinutes();\n";
		code += "\tvar s = today.getSeconds();\n";
		code += "\th = checkTime(h);\n";
		code += "\tm = checkTime(m);\n";
		code += "\ts = checkTime(s);\n";

		for (size_t i = 0; i < button.sr.size(); i++)
			code += String("\tdocument.getElementById('")+getButtonName(button.ad)+button.sr[i].number+"_font').innerHTML = h + \":\" + m + \":\" + s;\n";

		code += String("\tvar t = setTimeout(")+getFuncName(button.ad)+", 500);\n";
		code += "}\n";
		hScript = true;
		scriptType = SCR_TIME_STANDARD;
		scrStart = getFuncName(button.ad)+"();";
		return code;
	}

	if (button.ad == 142)		// System time AM/PM
	{
		code += String("function ")+getFuncName(button.ad)+"() {\n";
		code += "\tvar today = new Date();\n";
		code += "\tvar h = today.getHours();\n";
		code += "\tvar m = today.getMinutes();\n";
		code += "\tvar s = \"AM\"\n";
		code += "\tif (h > 12) {\n";
		code += "\t\th = h - 12;\n";
		code += "\t\ts = \"PM\";\n\t}\n";
		code += "\th = checkTime(h);\n";
		code += "\tm = checkTime(m);\n";

		for (size_t i = 0; i < button.sr.size(); i++)
			code += String("\tdocument.getElementById('")+getButtonName(button.ad)+button.sr[i].number+"_font').innerHTML = h + \":\" + m + \" \" + s;\n";

		code += String("\tvar t = setTimeout(")+getFuncName(button.ad)+", 500);\n";
		code += "}\n";
		hScript = true;
		scriptType = SCR_TIME_AMPM;
		scrStart = getFuncName(button.ad)+"();";
		return code;
	}

	if (button.ad == 143)		// System time 24 hour
	{
		code += String("function ")+getFuncName(button.ad)+"() {\n";
		code += "\tvar today = new Date();\n";
		code += "\tvar h = today.getHours();\n";
		code += "\tvar m = today.getMinutes();\n";
		code += "\th = checkTime(h);\n";
		code += "\tm = checkTime(m);\n";

		for (size_t i = 0; i < button.sr.size(); i++)
			code += String("\tdocument.getElementById('")+getButtonName(button.ad)+button.sr[i].number+"_font').innerHTML = h + \":\" + m;\n";

		code += String("\tvar t = setTimeout(")+getFuncName(button.ad)+", 500);\n";
		code += "}\n";
		hScript = true;
		scriptType = SCR_TIME_24;
		scrStart = getFuncName(button.ad)+"();";
		return code;
	}

	// This is for displaying the date
	if (button.ad == 151)		// System date: weekday
	{
		code += String("function ")+getFuncName(button.ad)+"() {\n";
		code += "\tvar today = new Date();\n";
		code += "\tswitch(today.getDay()) {\n";
		code += "\t\tcase 0: return 'Sunday'; break;\n";
		code += "\t\tcase 1: return 'Monday'; break;\n";
		code += "\t\tcase 2: return 'Thuesday'; break;\n";
		code += "\t\tcase 3: return 'Wednesday'; break;\n";
		code += "\t\tcase 4: return 'Thursday'; break;\n";
		code += "\t\tcase 5: return 'Friday'; break;\n";
		code += "\t\tcase 6: return 'Saturday'; break;\n";
		code += "\t}\n";

		for (size_t i = 0; i < button.sr.size(); i++)
			code += String("\tdocument.getElementById('")+getButtonName(button.ad)+button.sr[i].number+"_font').innerHTML = mon + '/' + day;\n";

		code += String("\tvar t = setTimeout(")+getFuncName(button.ad)+", 1000);\n";
		code += "}\n";
		hScript = true;
		scriptType = SCR_DATE_WEEKDAY;
		scrStart = getFuncName(button.ad)+"();";
		return code;
	}

	if (button.ad == 152)		// System date: mm/dd
	{
		code += String("function ")+getFuncName(button.ad)+"() {\n";
		code += "\tvar today = new Date();\n";
		code += "\tvar mon = today.getMonth() + 1;\n";
		code += "\tvar day = today.getDate();\n";
		code += "\tmon = checkTime(mon);\n";
		code += "\tday = checkTime(day);\n";

		for (size_t i = 0; i < button.sr.size(); i++)
			code += String("\tdocument.getElementById('")+getButtonName(button.ad)+button.sr[i].number+"_font').innerHTML = mon + '/' + day;\n";

		code += String("\tvar t = setTimeout(")+getFuncName(button.ad)+", 1000);\n";
		code += "}\n";
		hScript = true;
		scriptType = SCR_DATE_M_D;
		scrStart = getFuncName(button.ad)+"();";
		return code;
	}

	if (button.ad == 153)		// System date: dd/mm
	{
		code += String("function ")+getFuncName(button.ad)+"() {\n";
		code += "\tvar today = new Date();\n";
		code += "\tvar mon = today.getMonth() + 1;\n";
		code += "\tvar day = today.getDate();\n";
		code += "\tmon = checkTime(mon);\n";
		code += "\tday = checkTime(day);\n";

		for (size_t i = 0; i < button.sr.size(); i++)
			code += String("\tdocument.getElementById('")+getButtonName(button.ad)+button.sr[i].number+"_font').innerHTML = day + '/' + mon;\n";

		code += String("\tvar t = setTimeout(")+getFuncName(button.ad)+", 1000);\n";
		code += "}\n";
		hScript = true;
		scriptType = SCR_DATE_D_M;
		scrStart = getFuncName(button.ad)+"();";
		return code;
	}

	if (button.ad == 154)		// System date: mm/dd/yyyy
	{
		code += String("function ")+getFuncName(button.ad)+"() {\n";
		code += "\tvar today = new Date();\n";
		code += "\tvar mon = today.getMonth() + 1;\n";
		code += "\tvar day = today.getDate();\n";
		code += "\tvar year = today.getFullYear();\n";
		code += "\tmon = checkTime(mon);\n";
		code += "\tday = checkTime(day);\n";

		for (size_t i = 0; i < button.sr.size(); i++)
			code += String("\tdocument.getElementById('")+getButtonName(button.ad)+button.sr[i].number+"_font').innerHTML = mon + '/' + day + '/' + year;\n";

		code += String("\tvar t = setTimeout(")+getFuncName(button.ad)+", 1000);\n";
		code += "}\n";
		hScript = true;
		scriptType = SCR_DATE_M_D_Y;
		scrStart = getFuncName(button.ad)+"();";
		return code;
	}

	if (button.ad == 155)		// System date: dd/mm/yyyy
	{
		code += String("function ")+getFuncName(button.ad)+"() {\n";
		code += "\tvar today = new Date();\n";
		code += "\tvar mon = today.getMonth() + 1;\n";
		code += "\tvar day = today.getDate();\n";
		code += "\tvar year = today.getFullYear();\n";
		code += "\tmon = checkTime(mon);\n";
		code += "\tday = checkTime(day);\n";

		for (size_t i = 0; i < button.sr.size(); i++)
			code += String("\tdocument.getElementById('")+getButtonName(button.ad)+button.sr[i].number+"_font').innerHTML = day + '/' + mon + '/' + year;\n";

		code += String("\tvar t = setTimeout(")+getFuncName(button.ad)+", 1000);\n";
		code += "}\n";
		hScript = true;
		scriptType = SCR_DATE_D_M_Y;
		scrStart = getFuncName(button.ad)+"();";
		return code;
	}

	if (button.ad == 156)		// System date: month dd, yyyy
	{
		code += String("function ")+getFuncName(button.ad)+"() {\n";
		code += "\tvar today = new Date();\n";
		code += "\tvar mon = today.getMonth();\n";
		code += "\tvar day = today.getDate();\n";
		code += "\tvar year = today.getFullYear();\n";
		code += "\tswitch(mon) {\n";
		code += "\t\tcase 0: mon = 'Januray'; break;\n";
		code += "\t\tcase 1: mon = 'February'; break;\n";
		code += "\t\tcase 2: mon = 'March'; break;\n";
		code += "\t\tcase 3: mon = 'April'; break;\n";
		code += "\t\tcase 4: mon = 'May'; break;\n";
		code += "\t\tcase 5: mon = 'June'; break;\n";
		code += "\t\tcase 6: mon = 'July'; break;\n";
		code += "\t\tcase 7: mon = 'August'; break;\n";
		code += "\t\tcase 8: mon = 'September'; break;\n";
		code += "\t\tcase 9: mon = 'October'; break;\n";
		code += "\t\tcase 10: mon = 'November'; break;\n";
		code += "\t\tcase 11: mon = 'December'; break;\n";
		code += "\t}\n";
		code += "\tday = checkTime(day);\n";

		for (size_t i = 0; i < button.sr.size(); i++)
			code += String("\tdocument.getElementById('")+getButtonName(button.ad)+button.sr[i].number+"_font').innerHTML = mon + ' ' + day + ', ' + year;\n";

		code += String("\tvar t = setTimeout(")+getFuncName(button.ad)+", 1000);\n";
		code += "}\n";
		hScript = true;
		scriptType = SCR_DATE_MONTH_D_Y;
		scrStart = getFuncName(button.ad)+"();";
		return code;
	}

	if (button.ad == 157)		// System date: dd month, yyyy
	{
		code += String("function ")+getFuncName(button.ad)+"() {\n";
		code += "\tvar today = new Date();\n";
		code += "\tvar mon = today.getMonth();\n";
		code += "\tvar day = today.getDate();\n";
		code += "\tvar year = today.getFullYear();\n";
		code += "\tswitch(mon) {\n";
		code += "\t\tcase 0: mon = 'Januray'; break;\n";
		code += "\t\tcase 1: mon = 'February'; break;\n";
		code += "\t\tcase 2: mon = 'March'; break;\n";
		code += "\t\tcase 3: mon = 'April'; break;\n";
		code += "\t\tcase 4: mon = 'May'; break;\n";
		code += "\t\tcase 5: mon = 'June'; break;\n";
		code += "\t\tcase 6: mon = 'July'; break;\n";
		code += "\t\tcase 7: mon = 'August'; break;\n";
		code += "\t\tcase 8: mon = 'September'; break;\n";
		code += "\t\tcase 9: mon = 'October'; break;\n";
		code += "\t\tcase 10: mon = 'November'; break;\n";
		code += "\t\tcase 11: mon = 'December'; break;\n";
		code += "\t}\n";
		code += "\tday = checkTime(day);\n";

		for (size_t i = 0; i < button.sr.size(); i++)
			code += String("\tdocument.getElementById('")+getButtonName(button.ad)+button.sr[i].number+"_font').innerHTML = day + ' ' + mon + ', ' + year;\n";

		code += String("\tvar t = setTimeout(")+getFuncName(button.ad)+", 1000);\n";
		code += "}\n";
		hScript = true;
		scriptType = SCR_DATE_D_MONTH_Y;
		scrStart = getFuncName(button.ad)+"();";
		return code;
	}

	if (button.ad == 158)		// System date: yyyy-mm-dd
	{
		code += String("function ")+getFuncName(button.ad)+"() {\n";
		code += "\tvar today = new Date();\n";
		code += "\tvar mon = today.getMonth() + 1;\n";
		code += "\tvar day = today.getDate();\n";
		code += "\tvar year = today.getFullYear();\n";
		code += "\tmon = checkTime(mon);\n";
		code += "\tday = checkTime(day);\n";

		for (size_t i = 0; i < button.sr.size(); i++)
			code += String("\tdocument.getElementById('")+getBorderStyle(button.ad)+button.sr[i].number+"_font').innerHTML = year + '-' + mon + '-' + day;\n";

		code += String("\tvar t = setTimeout(")+getFuncName(button.ad)+", 1000);\n";
		code += "}\n";
		hScript = true;
		scriptType = SCR_DATE_Y_M_D;
		scrStart = getFuncName(button.ad)+"();";
		return code;
	}

	return "";
}

bool PushButton::getImageDimensions(const String fname, int* width, int* height)
{
	sysl->TRACE(String("PushButton::getImageDimensions(const String fname, int* width, int* height)"));

	gdImagePtr im = gdImageCreateFromFile(fname.data());

	if (im == 0)
	{
		sysl->errlog(String("PushButton::getImageDimensions: Error opening image ")+fname);
		*width = 0;
		*height = 0;
		return false;
	}

	*width = gdImageSX(im);
	*height = gdImageSY(im);
	gdImageDestroy(im);
	return true;
}
