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

PushButton::PushButton(const BUTTON_T& bt, const std::vector<PDATA_T>& pal)
		: button(bt),
		  palette(pal)

{
	sysl->TRACE(Syslog::ENTRY, std::string("PushButton::PushButton(const BUTTON_T& bt, const std::vector<PDATA_T>& pal)"));
	sysl->TRACE(String("PushButton::PushButton: Button: ")+bt.na+", ID: "+bt.bi);
	onOff = false;
	state = 0;
	fontClass = 0;

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

void PushButton::setState(size_t s)
{
	if (s > 0 && s <= button.sr.size() && (button.type == MULTISTATE_GENERAL || button.type == MULTISTATE_BARGRAPH))
		state = s - 1;
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
		if (button.ap == 0 && isSystemReserved(button.ad))
			style += String(".")+btName+i+" {\n";
		else	// Name: .button<number>_b<id>_<name>
			style += String(".Page")+pageID+"_b"+i+"_"+btName+" {\n";

		style += "  position: absolute;\n";
		style += String("  left: ")+String(button.lt)+"px;\n";
		style += String("  top: ")+String(button.tp)+"px;\n";
		style += String("  width: ")+String(button.wt)+"px;\n";
		style += String("  height: ")+String(button.ht)+"px;\n";

		if (button.zo > 0)
			style += String("  z-index: ")+button.zo+";\n";

		if (!button.sr[i].bs.empty())
		{
			style += getBorderStyle(button.sr[i].bs);
			style += String("  border-color: ")+pal.colorToString(pal.getColor(button.sr[i].cb))+";\n";
		}
		else
			style += "  border: none;\n";

		if (button.sr[i].mi.length() && button.sr[i].bm.length())
		{
			style += String("  background-image: url(images/")+NameFormat::toURL(button.sr[i].mi)+"), ";
			style += String("url(images/")+NameFormat::toURL(button.sr[i].bm)+");\n";
			style += "  background-blend-mode: screen;\n";
		}
		else if (button.sr[i].bm.length())
			style += String("  background-image: url(images/")+NameFormat::toURL(button.sr[i].bm)+");\n";

		style += "  background-repeat: no-repeat;\n";

		if ((button.fb == FB_MOMENTARY && i == 0) || (button.fb == FB_ALWAYS_ON && i == 1) ||
				(button.fb == FB_INV_CHANNEL && i == 1) || (button.fb == FB_CHANNEL && i == 0) ||
				(button.fb == FB_NONE && i == 0))
			style += "  display: inline;\n";
		else
			style += "  display: none;\n";

		style += String("  background-color: ")+pal.colorToString(pal.getColor(button.sr[i].cf))+";\n";
		style += String("  color: ")+pal.colorToString(pal.getColor(button.sr[i].ct))+";\n";
		style += "}\n";

		FONT_T font = fontClass->findFont(button.sr[i].fi);

		if (font.number == button.sr[i].fi)
		{
			if (button.ap == 0 && isSystemReserved(button.ad))
				style += String(".")+btName+i+"_font {\n";
			else
				style += String(".Page")+pageID+"_b"+i+"_"+btName+"_font {\n";

			style += "  border: none;\n";
			style += "  position: absolute;\n";
			style += String("  left: 5%;\n");
			style += String("  width: 90%;\n");
			style += String("  height: 100%;\n");
			style += String("  font-family: ")+NameFormat::toValidName(font.name)+";\n";
			style += String("  font-size: ")+String(font.size)+"pt;\n";
			style += String("  font-style: ")+fontClass->getFontStyle(font.subfamilyName)+";\n";
			style += String("  font-weight: ")+fontClass->getFontWeight(font.subfamilyName)+";\n";

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
		}
	}

	return style;
}

String PushButton::getWebCode()
{
	sysl->TRACE(String("PushButton::getWebCode()"));
	String code, names;

	sysl->TRACE(String("PushButton::getWebCode: for page ID: ")+pageID);

	if (button.type == GENERAL && button.sr.size() >= 2)
	{
		if (button.ap == 0 && isSystemReserved(button.ad))
		{
			names = btName+"0',";
			names += btName+"1'";
		}
		else
		{
			names = String("'Page")+pageID+"_b0_"+btName+"',";
			names += String("'Page")+pageID+"_b1_"+btName+"'";
		}
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
			nm = btName+i;
		else
			nm = String("Page")+pageID+"_b"+i+"_"+btName;

		code += String("      <div id=\"")+nm+"\" class=\""+nm+"\"";

		if (!button.pfName.empty() && !button.pfType.empty())
		{
			int pid = findPage(button.pfName);
			sysl->TRACE(String("PushButton::getWebCode: Button ")+button.na+" show/hide popup page "+button.pfName+". This is page ID "+pid+".");

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

		if (button.sr[i].te.length() > 0)
		{
			code += String("         <span id=\"")+nm+"_font\" class=\""+nm+"_font\">";
			code += NameFormat::textToWeb(button.sr[i].te)+"</span>\n";
		}
		else if (button.ad > 0 && button.ap > 0)	// A ^TXT command could send text to this field
			code += String("         <span id=\"")+nm+"_font\" class=\""+nm+"_font\"></span>\n";

		code += "      </div>\n";
	}

	code += "   </a>\n";

	return code;
}

int amx::PushButton::findPage(const strings::String& name)
{
	sysl->TRACE(String("PushButton::findPage(const strings::String& name)"));

	for (size_t i = 0; i < pageList.size(); i++)
	{
		if (pageList[i].name.compare(name) == 0)
			return pageList[i].pageID;
	}

	return 0;
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
			code += String("\tdocument.getElementById('")+getButtonName(button.ad)+i+"_font').innerHTML = h + \":\" + m + \":\" + s;\n";

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
			code += String("\tdocument.getElementById('")+getButtonName(button.ad)+i+"_font').innerHTML = h + \":\" + m + \" \" + s;\n";

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
			code += String("\tdocument.getElementById('")+getButtonName(button.ad)+i+"_font').innerHTML = h + \":\" + m;\n";

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
			code += String("\tdocument.getElementById('")+getButtonName(button.ad)+i+"_font').innerHTML = mon + '/' + day;\n";

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
			code += String("\tdocument.getElementById('")+getButtonName(button.ad)+i+"_font').innerHTML = mon + '/' + day;\n";

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
			code += String("\tdocument.getElementById('")+getButtonName(button.ad)+i+"_font').innerHTML = day + '/' + mon;\n";

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
			code += String("\tdocument.getElementById('")+getButtonName(button.ad)+i+"_font').innerHTML = mon + '/' + day + '/' + year;\n";

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
			code += String("\tdocument.getElementById('")+getButtonName(button.ad)+i+"_font').innerHTML = day + '/' + mon + '/' + year;\n";

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
			code += String("\tdocument.getElementById('")+getButtonName(button.ad)+i+"_font').innerHTML = mon + ' ' + day + ', ' + year;\n";

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
			code += String("\tdocument.getElementById('")+getButtonName(button.ad)+i+"_font').innerHTML = day + ' ' + mon + ', ' + year;\n";

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
			code += String("\tdocument.getElementById('")+getBorderStyle(button.ad)+i+"_font').innerHTML = year + '-' + mon + '-' + day;\n";

		code += String("\tvar t = setTimeout(")+getFuncName(button.ad)+", 1000);\n";
		code += "}\n";
		hScript = true;
		scriptType = SCR_DATE_Y_M_D;
		scrStart = getFuncName(button.ad)+"();";
		return code;
	}

	return "";
}
