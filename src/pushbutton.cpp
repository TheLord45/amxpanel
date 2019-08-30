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

#include <gd.h>
#include <iomanip>
#include "syslog.h"
#include "nameformat.h"
#include "config.h"
#include "pushbutton.h"
#include "trace.h"

extern Syslog *sysl;
extern Config *Configuration;

using namespace amx;
using namespace std;

PushButton::PushButton(const BUTTON_T& bt, const std::vector<PDATA_T>& pal)
		: button(bt),
		  palette(pal)

{
	sysl->TRACE(Syslog::ENTRY, "PushButton::PushButton(const BUTTON_T& bt, const std::vector<PDATA_T>& pal)");
	sysl->TRACE("PushButton::PushButton: Button: "+bt.na+", ID: "+to_string(bt.bi));
	fontClass = 0;
	iconClass = 0;

	if (button.ap == 0 && isSystemReserved(button.ad))
		btName = getButtonName(button.ad);
	else
		btName = "Button_"+to_string(bt.bi);

	hScript = false;
	scriptType = SCR_NONE;
}

PushButton::~PushButton()
{
	sysl->TRACE(Syslog::EXIT, "PushButton::PushButton(...)");
}

string PushButton::getStyle()
{
	DECL_TRACER("PushButton::getStyle()");
	sysl->TRACE("PushButton::getStyle: for page ID: "+to_string(pageID));

	string style, bgcolor, fgcolor, fillcolor;
	Palette pal;
	pal.setPalette(palette);

	for (size_t i = 0; i < button.sr.size(); i++)
	{
		bool hasChameleon = (!button.sr[i].mi.empty() && !button.sr[i].bm.empty() && button.sr[i].bs.empty());
		bool hasBorder = !button.sr[i].bs.empty();

		if (hasBorder || button.type == BARGRAPH)
			hasChameleon = false;

		if (button.ap == 0 && isSystemReserved(button.ad))
			style += "."+btName+to_string(button.sr[i].number)+" {\n";
		else	// Name: .button<number>_b<id>_<name>
			style += ".Page_"+to_string(pageID)+"_"+btName+"_"+to_string(button.sr[i].number)+" {\n";

		style += "  position: absolute;\n";
		style += "  left: "+to_string(button.lt)+"px;\n";
		style += "  top: "+to_string(button.tp)+"px;\n";
		style += "  width: "+to_string(button.wt)+"px;\n";
		style += "  height: "+to_string(button.ht)+"px;\n";

		if (Str::caseCompare(button.hs, "bounding") == 0)
			style += "  overflow: hidden;\n";

		if (hasBorder)
		{
			style += getBorderStyle(button.sr[i].bs);
			style += "  border-color: "+pal.colorToString(pal.getColor(button.sr[i].cb))+";\n";
		}
		else
			style += "  border: none;\n";

		if (!hasChameleon && button.sr[i].bm.length() && button.type != BARGRAPH)
		{
			style += "  background-image: url(images/"+NameFormat::toURL(button.sr[i].bm)+");\n";
			style += "  background-repeat: no-repeat;\n";
		}

		if ((button.fb == FB_MOMENTARY && i == 0) || (button.fb == FB_ALWAYS_ON && i == 1) ||
				(button.fb == FB_INV_CHANNEL && i == 1) || (button.fb == FB_CHANNEL && i == 0) ||
				(button.fb == FB_NONE && i == 0))
			style += "  display: inline;\n";
		else
			style += "  display: none;\n";

		if (!hasChameleon || hasBorder)
			style += "  background-color: "+pal.colorToString(pal.getColor(button.sr[i].cf))+";\n";

		style += "  color: "+pal.colorToString(pal.getColor(button.sr[i].ct))+";\n";
		style += "}\n";

		FONT_T font = fontClass->findFont(button.sr[i].fi);
	}

	return style;
}

string PushButton::getWebCode()
{
	DECL_TRACER("PushButton::getWebCode()");
	string code, names;

	sysl->TRACE("PushButton::getWebCode: for page ID: "+to_string(pageID));

	if ((button.type == GENERAL && button.sr.size() >= 2) || button.type == BARGRAPH)
	{
		if (button.ap == 0 && isSystemReserved(button.ad))
		{
			names = btName+to_string(button.sr[0].number)+"',";
			names += btName+to_string(button.sr[1].number)+"'";
		}
		else
		{
			names = "'Page"+to_string(pageID)+"_b"+to_string(button.sr[0].number)+"_"+btName+"',";
			names += "'Page"+to_string(pageID)+"_b"+to_string(button.sr[1].number)+"_"+btName+"'";
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

		string nm;

		if (button.ap == 0 && isSystemReserved(button.ad))
			nm = btName;
		else
			nm = "Page"+to_string(pageID)+"_"+btName;

		sBargraph += "\t{\"name\":\""+nm+"\",\"pnum\":"+to_string(pageID)+", \"bi\":"+to_string(button.bi)+",\"ap\":"+to_string(button.ap)+",\"ac\":"+to_string(button.ad);
		sBargraph += ",\"cp\":"+to_string(button.cp)+",\"ch\":"+to_string(button.ch)+",\"lp\":"+to_string(button.lp)+",\"lc\":"+to_string(button.lv);
		sBargraph += ",\"rl\":"+to_string(button.rl)+",\"rh\":"+to_string(button.rh)+",\"lv\":"+to_string(button.lv)+",\"level\":"+to_string(level);
		sBargraph += ",\"dr\":\""+button.dr+"\",\"states\":[\n";

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

			sBargraph += "\t\t{\"mi\":\""+button.sr[i].mi+"\",\"mi_width\":"+to_string(mi_width)+",\"mi_height\":"+to_string(mi_height)+",\"cb\":\""+button.sr[i].cb+"\"";
			sBargraph += ",\"cf\":\""+button.sr[i].cf+"\",\"bm\":\""+button.sr[i].bm+"\",\"bm_width\":"+to_string(bm_width)+",\"bm_height\":"+to_string(bm_height)+"}";
		}

		sBargraph += "]}";
	}

	if (button.ap == 0 && isSystemReserved(button.ad))
		code = "   <a href=\"#\" id=\""+btName+"\">\n";
	else
		code = "   <a href=\"#\" id=\"Page"+to_string(pageID)+btName+"\">\n";

	for (size_t i = 0; i < button.sr.size(); i++)
	{
		// FIXME: Die unterschiedlichen button status berücksichtigen
		string nm;

		if (button.ap == 0 && isSystemReserved(button.ad))
			nm = btName+to_string(button.sr[i].number);
		else
			nm = "Page"+to_string(pageID)+"_b"+to_string(button.sr[i].number)+"_"+btName;

		code += "      <div id=\""+nm+"\" class=\""+nm+"\"";

		if (button.pushFunc.size() > 0)
		{
			code += " onclick=\"";

			for (size_t j = 0; j < button.pushFunc.size(); j++)
			{
				sysl->TRACE("PushButton::getWebCode: Button "+button.na+" show/hide popup page "+button.pushFunc[j].pfName+".");

				if (Str::caseCompare(button.pushFunc[j].pfType, "sShow") == 0)		// show popup
					code += "showPopup('"+button.pushFunc[j].pfName+"');";
				else if (Str::caseCompare(button.pushFunc[j].pfType, "sHide") == 0)	// hide popup
					code += "hidePopup('"+button.pushFunc[j].pfName+"');";
				else if (Str::caseCompare(button.pushFunc[j].pfType, "scGroup") == 0)	// hide group
					code += "hideGroup('"+button.pushFunc[j].pfName+"');";
			}

			code += "\"";
		}
		else if (((button.type == GENERAL && button.fb == FB_MOMENTARY && (i == 0 || i == 1)) ||
				button.type == BARGRAPH) && button.cp > 0 && button.ch > 0)
		{
			code += " onmousedown=\"switchDisplay("+names+",1,"+to_string(button.cp)+","+to_string(button.ch)+");\"";
			code += " onmouseup=\"switchDisplay("+names+",0,"+to_string(button.cp)+","+to_string(button.ch)+");\"";
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
			code += "         <canvas id=\""+nm+"_canvas\" width=\""+to_string(width)+"px\" height=\""+to_string(height)+"px\" style=\"";
			code += "position: absolute;left: 0px;top: 0px;\"></canvas>\n";
			code += "         <script>\n";
			code += "            var visProp = getHiddenProp();\n";
			code += "            if (visProp) {\n";
			code += "               var evtname = visProp.replace(/[H|h]idden/,'') + 'visibilitychange';\n";
			code += "               document.getElementById('Page_"+to_string(pageID)+"').addEventListener(evtname,function() { ";
			code += "if (isVisible(document.getElementById('Page_"+to_string(pageID)+"'))) drawButton(makeURL('images/"+button.sr[i].mi+"'),makeURL('images/"+button.sr[i].bm+"'),'"+nm+"',"+to_string(width)+","+to_string(height)+","+pal.colorToSArray(col1)+","+pal.colorToSArray(col2)+"); });\n";
			code += "            }\n         </script>\n";
		}

		if (iconClass != 0 && button.sr[i].ii > 0)		// Icon?
		{
			int width, height;
			string icoFile = iconClass->getFileFromID(button.sr[i].ii);
			bool stat = getImageDimensions(Configuration->getHTTProot()+"/images/"+icoFile, &width, &height);

			code += "         <img id=\""+nm+"_icon\" src=\"images/"+icoFile+"\"";

			if (button.sr[i].ji == 0)
			{
				if (stat)
				{
					sysl->TRACE("PushButton::getWebCode: width="+to_string(width)+", height="+to_string(height));

					if ((button.sr[i].ix + width) > button.wt)
					{
						button.sr[i].ix = (button.wt - width);

						if (button.sr[i].ix < 0)
						{
							button.sr[i].ix = 0;
							code += " width=\""+to_string(button.wt)+"px\"";
						}
						else
							code += " width=\""+to_string(width)+"px\"";
					}
					else
						code += " width=\""+to_string(width)+"px\"";

					if ((button.sr[i].iy + height) > button.ht)
					{
						button.sr[i].iy = (button.ht - height);

						if (button.sr[i].iy < 0)
						{
							button.sr[i].iy = 0;
							code += " height=\""+to_string(button.ht)+"px\"";
						}
						else
							code += " height=\""+to_string(height)+"px\"";
					}
					else
						code += " height=\""+to_string(height)+"px\"";
				}
			}

			switch(button.sr[i].ji)			// Icon justification
			{
				case 0:	code += " style=\"position: absolute;left: "+to_string(button.sr[i].ix)+"px;top: "+to_string(button.sr[i].iy)+"px;\""; break;
				case 1:	code += " style=\"position: absolute;left: 0px;top: 0px;\""; break;

				default:
					code += " onload=\"posImage(this,'"+nm+"',"+to_string(button.sr[i].ji)+");\"";
			}

			code += ">\n";
		}

		if (button.sr[i].te.length() > 0)
		{
			code += "         <span id=\""+nm+"_font\" class=\""+nm+"_font\">";
			code += NameFormat::textToWeb(button.sr[i].te)+"</span>\n";
		}
		else			// A ^TXT command could send text to this field
			code += "         <span id=\""+nm+"_font\" class=\""+nm+"_font\"></span>\n";

		code += "      </div>\n";
	}

	code += "   </a>\n";

	return code;
}

string PushButton::getScriptCode()
{
	DECL_TRACER("PushButton::getScriptCode()");

	if (button.ap != 0)
		return "";

	string code;

	// This is for displaying the time
	if (button.ad == 141)		// System time standard
	{
		code += "function "+getFuncName(button.ad)+"() {\n";
		code += "\tvar today = new Date();\n";
		code += "\tvar h = today.getHours();\n";
		code += "\tvar m = today.getMinutes();\n";
		code += "\tvar s = today.getSeconds();\n";
		code += "\th = checkTime(h);\n";
		code += "\tm = checkTime(m);\n";
		code += "\ts = checkTime(s);\n";

		for (size_t i = 0; i < button.sr.size(); i++)
			code += "\tdocument.getElementById('"+getButtonName(button.ad)+to_string(button.sr[i].number)+"_font').innerHTML = h + \":\" + m + \":\" + s;\n";

		code += "\tvar t = setTimeout("+getFuncName(button.ad)+", 500);\n";
		code += "}\n";
		hScript = true;
		scriptType = SCR_TIME_STANDARD;
		scrStart = getFuncName(button.ad)+"();";
		return code;
	}

	if (button.ad == 142)		// System time AM/PM
	{
		code += "function "+getFuncName(button.ad)+"() {\n";
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
			code += "\tdocument.getElementById('"+getButtonName(button.ad)+to_string(button.sr[i].number)+"_font').innerHTML = h + \":\" + m + \" \" + s;\n";

		code += "\tvar t = setTimeout("+getFuncName(button.ad)+", 500);\n";
		code += "}\n";
		hScript = true;
		scriptType = SCR_TIME_AMPM;
		scrStart = getFuncName(button.ad)+"();";
		return code;
	}

	if (button.ad == 143)		// System time 24 hour
	{
		code += "function "+getFuncName(button.ad)+"() {\n";
		code += "\tvar today = new Date();\n";
		code += "\tvar h = today.getHours();\n";
		code += "\tvar m = today.getMinutes();\n";
		code += "\th = checkTime(h);\n";
		code += "\tm = checkTime(m);\n";

		for (size_t i = 0; i < button.sr.size(); i++)
			code += "\tdocument.getElementById('"+getButtonName(button.ad)+to_string(button.sr[i].number)+"_font').innerHTML = h + \":\" + m;\n";

		code += "\tvar t = setTimeout("+getFuncName(button.ad)+", 500);\n";
		code += "}\n";
		hScript = true;
		scriptType = SCR_TIME_24;
		scrStart = getFuncName(button.ad)+"();";
		return code;
	}

	// This is for displaying the date
	if (button.ad == 151)		// System date: weekday
	{
		code += "function "+getFuncName(button.ad)+"() {\n";
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
			code += "\tdocument.getElementById('"+getButtonName(button.ad)+to_string(button.sr[i].number)+"_font').innerHTML = mon + '/' + day;\n";

		code += "\tvar t = setTimeout("+getFuncName(button.ad)+", 1000);\n";
		code += "}\n";
		hScript = true;
		scriptType = SCR_DATE_WEEKDAY;
		scrStart = getFuncName(button.ad)+"();";
		return code;
	}

	if (button.ad == 152)		// System date: mm/dd
	{
		code += "function "+getFuncName(button.ad)+"() {\n";
		code += "\tvar today = new Date();\n";
		code += "\tvar mon = today.getMonth() + 1;\n";
		code += "\tvar day = today.getDate();\n";
		code += "\tmon = checkTime(mon);\n";
		code += "\tday = checkTime(day);\n";

		for (size_t i = 0; i < button.sr.size(); i++)
			code += "\tdocument.getElementById('"+getButtonName(button.ad)+to_string(button.sr[i].number)+"_font').innerHTML = mon + '/' + day;\n";

		code += "\tvar t = setTimeout("+getFuncName(button.ad)+", 1000);\n";
		code += "}\n";
		hScript = true;
		scriptType = SCR_DATE_M_D;
		scrStart = getFuncName(button.ad)+"();";
		return code;
	}

	if (button.ad == 153)		// System date: dd/mm
	{
		code += "function "+getFuncName(button.ad)+"() {\n";
		code += "\tvar today = new Date();\n";
		code += "\tvar mon = today.getMonth() + 1;\n";
		code += "\tvar day = today.getDate();\n";
		code += "\tmon = checkTime(mon);\n";
		code += "\tday = checkTime(day);\n";

		for (size_t i = 0; i < button.sr.size(); i++)
			code += "\tdocument.getElementById('"+getButtonName(button.ad)+to_string(button.sr[i].number)+"_font').innerHTML = day + '/' + mon;\n";

		code += "\tvar t = setTimeout("+getFuncName(button.ad)+", 1000);\n";
		code += "}\n";
		hScript = true;
		scriptType = SCR_DATE_D_M;
		scrStart = getFuncName(button.ad)+"();";
		return code;
	}

	if (button.ad == 154)		// System date: mm/dd/yyyy
	{
		code += "function "+getFuncName(button.ad)+"() {\n";
		code += "\tvar today = new Date();\n";
		code += "\tvar mon = today.getMonth() + 1;\n";
		code += "\tvar day = today.getDate();\n";
		code += "\tvar year = today.getFullYear();\n";
		code += "\tmon = checkTime(mon);\n";
		code += "\tday = checkTime(day);\n";

		for (size_t i = 0; i < button.sr.size(); i++)
			code += "\tdocument.getElementById('"+getButtonName(button.ad)+to_string(button.sr[i].number)+"_font').innerHTML = mon + '/' + day + '/' + year;\n";

		code += "\tvar t = setTimeout("+getFuncName(button.ad)+", 1000);\n";
		code += "}\n";
		hScript = true;
		scriptType = SCR_DATE_M_D_Y;
		scrStart = getFuncName(button.ad)+"();";
		return code;
	}

	if (button.ad == 155)		// System date: dd/mm/yyyy
	{
		code += "function "+getFuncName(button.ad)+"() {\n";
		code += "\tvar today = new Date();\n";
		code += "\tvar mon = today.getMonth() + 1;\n";
		code += "\tvar day = today.getDate();\n";
		code += "\tvar year = today.getFullYear();\n";
		code += "\tmon = checkTime(mon);\n";
		code += "\tday = checkTime(day);\n";

		for (size_t i = 0; i < button.sr.size(); i++)
			code += "\tdocument.getElementById('"+getButtonName(button.ad)+to_string(button.sr[i].number)+"_font').innerHTML = day + '/' + mon + '/' + year;\n";

		code += "\tvar t = setTimeout("+getFuncName(button.ad)+", 1000);\n";
		code += "}\n";
		hScript = true;
		scriptType = SCR_DATE_D_M_Y;
		scrStart = getFuncName(button.ad)+"();";
		return code;
	}

	if (button.ad == 156)		// System date: month dd, yyyy
	{
		code += "function "+getFuncName(button.ad)+"() {\n";
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
			code += "\tdocument.getElementById('"+getButtonName(button.ad)+to_string(button.sr[i].number)+"_font').innerHTML = mon + ' ' + day + ', ' + year;\n";

		code += "\tvar t = setTimeout("+getFuncName(button.ad)+", 1000);\n";
		code += "}\n";
		hScript = true;
		scriptType = SCR_DATE_MONTH_D_Y;
		scrStart = getFuncName(button.ad)+"();";
		return code;
	}

	if (button.ad == 157)		// System date: dd month, yyyy
	{
		code += "function "+getFuncName(button.ad)+"() {\n";
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
			code += "\tdocument.getElementById('"+getButtonName(button.ad)+to_string(button.sr[i].number)+"_font').innerHTML = day + ' ' + mon + ', ' + year;\n";

		code += "\tvar t = setTimeout("+getFuncName(button.ad)+", 1000);\n";
		code += "}\n";
		hScript = true;
		scriptType = SCR_DATE_D_MONTH_Y;
		scrStart = getFuncName(button.ad)+"();";
		return code;
	}

	if (button.ad == 158)		// System date: yyyy-mm-dd
	{
		code += "function "+getFuncName(button.ad)+"() {\n";
		code += "\tvar today = new Date();\n";
		code += "\tvar mon = today.getMonth() + 1;\n";
		code += "\tvar day = today.getDate();\n";
		code += "\tvar year = today.getFullYear();\n";
		code += "\tmon = checkTime(mon);\n";
		code += "\tday = checkTime(day);\n";

		for (size_t i = 0; i < button.sr.size(); i++)
			code += "\tdocument.getElementById('"+getBorderStyle(to_string(button.ad))+to_string(button.sr[i].number)+"_font').innerHTML = year + '-' + mon + '-' + day;\n";

		code += "\tvar t = setTimeout("+getFuncName(button.ad)+", 1000);\n";
		code += "}\n";
		hScript = true;
		scriptType = SCR_DATE_Y_M_D;
		scrStart = getFuncName(button.ad)+"();";
		return code;
	}

	return "";
}

bool PushButton::getImageDimensions(const string fname, int* width, int* height)
{
	DECL_TRACER("PushButton::getImageDimensions(const String fname, int* width, int* height)");

	gdImagePtr im = gdImageCreateFromFile(fname.c_str());

	if (im == 0)
	{
		sysl->errlog("PushButton::getImageDimensions: Error opening image "+fname);
		*width = 0;
		*height = 0;
		return false;
	}

	*width = gdImageSX(im);
	*height = gdImageSY(im);
	gdImageDestroy(im);
	return true;
}
