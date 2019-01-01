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

#include "syslog.h"
#include "nameformat.h"
#include "config.h"
#include "pushbutton.h"

extern Syslog *sysl;
extern Config *Configuration;

using namespace amx;
using namespace strings;

PushButton::PushButton(const BUTTON_T& bt, const String& pfilename)
		: Palette(pfilename),
		  button(bt)

{
	sysl->TRACE(Syslog::ENTRY, std::string("PushButton::PushButton(const BUTTON_T& bt, const String& pfilename)"));
	onOff = false;
	state = 0;
	fontClass = 0;
	btName = String("Button_")+bt.bi;
	hScript = false;
	scriptType = SCR_NONE;
}

PushButton::~PushButton()
{
	sysl->TRACE(Syslog::EXIT, std::string("PushButton::PushButton(...)"));
}

void PushButton::setState(size_t s)
{
	if (s > 0 && s <= button.sr.size() && (button.type == MULTISTATE_GENERAL || button.type == MULTISTATE_BARGRAPH))
		state = s - 1;
}

String PushButton::getStyle()
{
	sysl->TRACE(Syslog::MESSAGE, std::string("PushButton::getStyle()"));

	String style, bgcolor, fgcolor, fillcolor;

	for (size_t i = 0; i < button.sr.size(); i++)
	{
		// Name: .button<number>-<name>
		style += String(".button")+String(i)+"-"+btName+" {\n";
		style += "  position: absolut;";
		style += String("  left: ")+String(button.lt)+";\n";
		style += String("  top: ")+String(button.tp)+";\n";
		style += String("  width: ")+String(button.wt)+";\n";
		style += String("  height: ")+String(button.ht)+";\n";
		style += "  border: none;\n";		// FIXME!

		if (button.sr[i].mi.length() && button.sr[i].bm.length())
		{
			style += String("  background-image: url(images/")+button.sr[i].mi+"), ";
			style += String("url(images/")+button.sr[i].bm+");\n";
			style += "  background-blend-mode: screen;\n";
		}
		else if (button.sr[i].bm.length())
			style += String("  background-image: url(images/")+button.sr[i].bm+");\n";

		style += "  background-repeat: no-repeat;\n";
		style += "  display: ";

		if (button.type == GENERAL && i == 0)
			style += (onOff) ? "none;\n" : "inline;\n";
		else if (button.type == MULTISTATE_GENERAL || button.type == MULTISTATE_BARGRAPH)
		{
			if ((int)i == state)
				style += "inline;\n";
			else
				style += "none;\n";
		}
		else	// FIXME!
			style += "inline;\n";

		style += String("  background-color: ")+colorToString(getColor(button.sr[i].cf))+";\n";
		style += String("  color: ")+colorToString(getColor(button.sr[i].ct))+";\n";
		style += "}\n";

		FONT_T font = fontClass->findFont(button.sr[i].fi);

		if (font.number == button.sr[i].fi)
		{
			style += String(".button")+String(i)+"-"+btName+"_font {\n";
			style += "  position: absolute;\n";
			style += "  border: none;\n";
			style += String("  font-family: ")+NameFormat::toValidName(font.name)+";\n";
			style += String("  font-size: ")+String(font.size)+"pt;\n";
			style += String("  font-style: ")+fontClass->getFontStyle(font.subfamilyName)+";\n";
			style += String("  font-weight: ")+fontClass->getFontWeight(font.subfamilyName)+";\n";

			if (button.sr[i].jt == 0)
			{
				style += String("  left: ")+String(button.sr[i].tx)+"pt;\n";
				style += String("  top: ")+String(button.sr[i].ty)+"pt;\n";
			}
			else
				style += "  text-align: center;\n";		// FIXME!

			style += "}\n";
		}
	}

	return style;
}

String PushButton::getWebCode()
{
	String code, names;

	if (button.type == GENERAL && button.sr.size() >= 2)
	{
		names = String("'button1-")+btName+"','";
		names += String("'button2-")+btName+"'";
	}

	code = String("<a href=\"")+Configuration->getHTTProot()+"/button_1?pg="+pageID+"&bt="+button.bi+"&press=\"> id=\"button_"+button.bi+"\">\n";

	for (size_t i = 0; i < button.sr.size(); i++)
	{
		String nm = String("button")+i+"-"+btName;
		code += String("<div id=\"")+nm+"\" class=\""+nm+"\"";

		if (!button.pfName.empty() && !button.pfType.empty())
		{
			int pid = parentPage.findPage(button.pfName);

			if (button.pfType.caseCompare("sShow") == 0)		// show popup
				code += String(" onclick=\"document.getElementById('Page_")+pid+"').style.display = 'inline';\"";
			else if (button.pfType.caseCompare("sHide") == 0)	// hide popup
				code += String(" onclick=\"document.getElementById('Page_")+pid+"').style.display = 'none';\"";
		}

		if (button.type == GENERAL && button.fb == FB_MOMENTARY && (i == 0 || i == 1))
		{
			code += String(" onmousedown=\"switchDisplay(")+names+",1,"+button.bi+");\"";
			code += String(" onmouseup=\"switchDisplay(")+names+",0,"+button.bi+");\"";
		}

		code += ">\n";

		if (button.sr[i].ct.length() > 0)
		{
			code += String("   <div class=\"")+nm+"_font\">";
			code += button.sr[i].ct+"</div>\n";
		}

		code += "</div>\n";
	}

	code += "</a>\n";

	return code;
}

String PushButton::getScriptCode()
{
	String code;

	// This is for displaying the time
	if (button.ap == 0 && button.ad == 141)		// System time standard
	{
		code += "function startTimeStandard() {\n";
		code += "\tvar today = new Date();\n";
		code += "\tvar h = today.getHours();\n";
		code += "\tvar m = today.getMinutes();\n";
		code += "\tvar s = today.getSeconds();\n";
		code += "\th = checkTime(h);\n";
		code += "\tm = checkTime(m);\n";
		code += "\ts = checkTime(s);\n";
		code += "\tdocument.getElementById('"+btName+"').innerHTML = h + \":\" + m + \":\" + s;\n";
		code += "\tvar t = setTimeout(startTimeStandard, 500);\n";
		code += "}\n";
		hScript = true;
		scriptType = SCR_TIME_STANDARD;
		return code;
	}

	if (button.ap == 0 && button.ad == 142)		// System time AM/PM
	{
		code += "function startTimeAMPM() {\n";
		code += "\tvar today = new Date();\n";
		code += "\tvar h = today.getHours();\n";
		code += "\tvar m = today.getMinutes();\n";
		code += "\tvar s = \"AM\"\n";
		code += "\tif (h > 12) {\n";
		code += "\t\th = h - 12;\n";
		code += "\t\ts = \"PM\";\n\t}\n
		code += "\th = checkTime(h);\n";
		code += "\tm = checkTime(m);\n";
		code += "\tdocument.getElementById('"+btName+"').innerHTML = h + \":\" + m + \" \" + s;\n";
		code += "\tvar t = setTimeout(startTimeAMPM, 500);\n";
		code += "}\n";
		hScript = true;
		scriptType = SCR_TIME_AMPM;
		return code;
	}

	if (button.ap == 0 && button.ad == 143)		// System time 24 hour
	{
		code += "function startTime24() {\n";
		code += "\tvar today = new Date();\n";
		code += "\tvar h = today.getHours();\n";
		code += "\tvar m = today.getMinutes();\n";
		code += "\th = checkTime(h);\n";
		code += "\tm = checkTime(m);\n";
		code += "\tdocument.getElementById('"+btName+"').innerHTML = h + \":\" + m;\n";
		code += "\tvar t = setTimeout(startTime24, 500);\n";
		code += "}\n";
		hScript = true;
		scriptType = SCR_TIME_24;
		return code;
	}

	// This is for displaying the date
	if (button.ap == 0 && button.ad == 151)		// System date: weekday
	{
		code += "function startDate_151() {\n";
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
		code += "\tdocument.getElementById('"+btName+"').innerHTML = mon + '/' + day;\n";
		code += "\tvar t = setTimeout(startDate_151, 1000);\n";
		code += "}\n";
		hScript = true;
		scriptType = SCR_DATE_WEEKDAY;
		return code;
	}

	if (button.ap == 0 && button.ad == 152)		// System date: mm/dd
	{
		code += "function startDate_152() {\n";
		code += "\tvar today = new Date();\n";
		code += "\tvar mon = today.getMonth() + 1;\n";
		code += "\tvar day = today.getDate();\n";
		code += "\tmon = checkTime(mon);\n";
		code += "\tday = checkTime(day);\n";
		code += "\tdocument.getElementById('"+btName+"').innerHTML = mon + '/' + day;\n";
		code += "\tvar t = setTimeout(startDate_152, 1000);\n";
		code += "}\n";
		hScript = true;
		scriptType = SCR_DATE_M_D;
		return code;
	}

	if (button.ap == 0 && button.ad == 153)		// System date: dd/mm
	{
		code += "function startDate_153() {\n";
		code += "\tvar today = new Date();\n";
		code += "\tvar mon = today.getMonth() + 1;\n";
		code += "\tvar day = today.getDate();\n";
		code += "\tmon = checkTime(mon);\n";
		code += "\tday = checkTime(day);\n";
		code += "\tdocument.getElementById('"+btName+"').innerHTML = day + '/' + mon;\n";
		code += "\tvar t = setTimeout(startDate_153, 1000);\n";
		code += "}\n";
		hScript = true;
		scriptType = SCR_DATE_D_M;
		return code;
	}

	if (button.ap == 0 && button.ad == 154)		// System date: mm/dd/yyyy
	{
		code += "function startDate_154() {\n";
		code += "\tvar today = new Date();\n";
		code += "\tvar mon = today.getMonth() + 1;\n";
		code += "\tvar day = today.getDate();\n";
		code += "\tvar year = today.getFullYear();\n";
		code += "\tmon = checkTime(mon);\n";
		code += "\tday = checkTime(day);\n";
		code += "\tdocument.getElementById('"+btName+"').innerHTML = mon + '/' + day + '/' + year;\n";
		code += "\tvar t = setTimeout(startDate_154, 1000);\n";
		code += "}\n";
		hScript = true;
		scriptType = SCR_DATE_M_D_Y;
		return code;
	}

	if (button.ap == 0 && button.ad == 155)		// System date: dd/mm/yyyy
	{
		code += "function startDate_155() {\n";
		code += "\tvar today = new Date();\n";
		code += "\tvar mon = today.getMonth() + 1;\n";
		code += "\tvar day = today.getDate();\n";
		code += "\tvar year = today.getFullYear();\n";
		code += "\tmon = checkTime(mon);\n";
		code += "\tday = checkTime(day);\n";
		code += "\tdocument.getElementById('"+btName+"').innerHTML = day + '/' + mon + '/' + year;\n";
		code += "\tvar t = setTimeout(startDate_155, 1000);\n";
		code += "}\n";
		hScript = true;
		scriptType = SCR_DATE_D_M_Y;
		return code;
	}

	if (button.ap == 0 && button.ad == 156)		// System date: month dd, yyyy
	{
		code += "function startDate_156() {\n";
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
		code += "\tdocument.getElementById('"+btName+"').innerHTML = mon + ' ' + day + ', ' + year;\n";
		code += "\tvar t = setTimeout(startDate_156, 1000);\n";
		code += "}\n";
		hScript = true;
		scriptType = SCR_DATE_MONTH_D_Y;
		return code;
	}

	if (button.ap == 0 && button.ad == 157)		// System date: dd month, yyyy
	{
		code += "function startDate_157() {\n";
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
		code += "\tdocument.getElementById('"+btName+"').innerHTML = day + ' ' + mon + ', ' + year;\n";
		code += "\tvar t = setTimeout(startDate_157, 1000);\n";
		code += "}\n";
		hScript = true;
		scriptType = SCR_DATE_D_MONTH_Y;
		return code;
	}

	if (button.ap == 0 && button.ad == 158)		// System date: yyyy-mm-dd
	{
		code += "function startDate_158() {\n";
		code += "\tvar today = new Date();\n";
		code += "\tvar mon = today.getMonth() + 1;\n";
		code += "\tvar day = today.getDate();\n";
		code += "\tvar year = today.getFullYear();\n";
		code += "\tmon = checkTime(mon);\n";
		code += "\tday = checkTime(day);\n";
		code += "\tdocument.getElementById('"+btName+"').innerHTML = year + '-' + mon + '-' + day;\n";
		code += "\tvar t = setTimeout(startDate_158, 1000);\n";
		code += "}\n";
		hScript = true;
		scriptType = SCR_DATE_Y_M_D;
		return code;
	}

	return "";
}
