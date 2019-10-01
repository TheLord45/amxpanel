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

	if (button.ap == 0 && isSystemReserved(button.ad))
		btName = getButtonName(button.ad);
	else
		btName = "Button_"+to_string(bt.bi);
}

PushButton::~PushButton()
{
	sysl->TRACE(Syslog::EXIT, "PushButton::~PushButton()");
}

/*
 * The following method is part of a future refactoring!
 * The method creates the JSON structure to hold the data for bargraphs. This
 * structure will be written to a file.
 */
string PushButton::getWebCode()
{
	DECL_TRACER("PushButton::getWebCode()");
	string code;	// This is a relict!

	sysl->TRACE("PushButton::getWebCode: for page ID: "+to_string(pageID));

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
		sBargraph += ",\"dr\":\""+button.dr+"\",\"if\":\""+button._if+"\",\"states\":[\n";

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

			if (button.sr[i].bm.length() > 0 && !button.sr[i].dynamic)
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
