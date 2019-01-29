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
	onOff = false;
	state = 0;
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
		bool hasChameleon = (!button.sr[i].mi.empty() && !button.sr[i].bm.empty() && button.sr[i].bs.empty());
		bool hasBorder = !button.sr[i].bs.empty();

		if (hasBorder)
			hasChameleon = false;

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

		if (button.hs.caseCompare("bounding") == 0)
			style += String("  overflow: hidden;\n");

		if (hasBorder)
		{
			style += getBorderStyle(button.sr[i].bs);
			style += String("  border-color: ")+pal.colorToString(pal.getColor(button.sr[i].cb))+";\n";
		}
		else
			style += "  border: none;\n";

		if (hasChameleon)	// Chameleon image?
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
		else if (button.sr[i].bm.length())
			style += String("  background-image: url(images/")+NameFormat::toURL(button.sr[i].bm)+");\n";

		style += "  background-repeat: no-repeat;\n";

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

		if (font.number == button.sr[i].fi)
		{
			if (button.ap == 0 && isSystemReserved(button.ad))
				style += String(".")+btName+i+"_font {\n";
			else
				style += String(".Page")+pageID+"_b"+i+"_"+btName+"_font {\n";

			if (!button.sr[i].bs.empty())
			{
				style += getBorderStyle(button.sr[i].bs);
				style += "  border-color: rgba(0, 0, 0, 0);\n";
			}
			else
				style += "  border: none;\n";

			style += "  position: absolute;\n";
//			style += String("  left: 5%;\n");
			style += String("  width: 100%;\n");
			style += String("  height: 100%;\n");
			style += String("  font-family: \"")+font.name+"\";\n";
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
	{
		code = String("   <a href=\"#\" id=\"Page")+pageID+btName+"\"";

		if (!button.pfName.empty() && !button.pfType.empty())
		{
			sysl->TRACE(String("PushButton::getWebCode: Button ")+button.na+" show/hide popup page "+button.pfName+".");

			if (button.pfType.caseCompare("sShow") == 0)		// show popup
				code += String(" onclick=\"showPopup('")+button.pfName+"');\"";
			else if (button.pfType.caseCompare("sHide") == 0)	// hide popup
				code += String(" onclick=\"hidePopup('")+button.pfName+"');\"";
			else if (button.pfType.caseCompare("scGroup") == 0)	// hide group
				code += String(" onclick=\"hideGroup('")+button.pfName+"');\"";
		}

		code += ">\n";
	}

	for (size_t i = 0; i < button.sr.size(); i++)
	{
		// FIXME: Die unterschiedlichen button status berücksichtigen
		String nm;

		if (button.ap == 0 && isSystemReserved(button.ad))
			nm = btName+i;
		else
			nm = String("Page")+pageID+"_b"+i+"_"+btName;

		code += String("      <div id=\"")+nm+"\" class=\""+nm+"\"";

		if (button.type == GENERAL && button.fb == FB_MOMENTARY && (i == 0 || i == 1))
		{
			code += String(" onmousedown=\"switchDisplay(")+names+",1,"+button.cp+","+button.ch+");\"";
			code += String(" onmouseup=\"switchDisplay(")+names+",0,"+button.cp+","+button.ch+");\"";
		}

		code += ">\n";

		if (iconClass != 0 && button.sr[i].ii > 0)		// Icon?
		{
			int width, height;
			String icoFile = iconClass->getFileFromID(button.sr[i].ii);

			code += String("         <img src=\"images/")+icoFile+"\"";

			if (button.sr[i].ji == 0)
			{
				if (getImageDimensions(Configuration->getHTTProot()+"/images/"+icoFile, &width, &height))
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
				case 1:	code += " style=\"align: left;vertical-align: top;\""; break;
				case 2:	code += " style=\"align: center;vertical-align: top;\""; break;
				case 3:	code += " style=\"align: right;vertical-align: top;\""; break;
				case 4:	code += " style=\"align: left;vertical-align: middle;\""; break;
				case 5:	code += " style=\"align: center;vertical-align: middle;\""; break;
				case 6:	code += " style=\"align: right;vertical-align: middle;\""; break;
				case 7:	code += " style=\"align: left;vertical-align: bottom;\""; break;
				case 8:	code += " style=\"align: center;vertical-align: bottom;\""; break;
				case 9:	code += " style=\"align: right;vertical-align: bottom;\""; break;

				default:
					code += String(" style=\"position: absolute;left: 0px;top: 0px;\"");
			}

			code += ">\n";
		}

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

bool done = false;
bool doIt = false;

String PushButton::createChameleonImage(const String bm1, const String bm2, unsigned long fill, unsigned long border)
{
	sysl->TRACE(String("PushButton::createChameleonImage(const String bm1, const String bm2)"));

	if (bm1.empty() || bm2.empty())
		return "";

	gdImagePtr im1 = gdImageCreateFromFile(bm1.data());

	if (im1 == 0)
	{
		sysl->TRACE(String("PushButton::createChameleonImage: Error opening image ")+bm1);
		return "";
	}

	gdImagePtr im2 = gdImageCreateFromFile(bm2.data());

	if (im2 == 0)
	{
		sysl->errlog(String("PushButton::createChameleonImage: Error opening image ")+bm2);
		gdImageDestroy(im1);
		return "";
	}

	int width = gdImageSX(im1);
    int height = gdImageSY(im1);

	gdImagePtr imNew = gdImageCreateTrueColor(width, height);

	if (imNew == 0)
	{
		sysl->errlog(String("PushButton::createChameleonImage: Error creating a true color image!"));
		gdImageDestroy(im1);
		gdImageDestroy(im2);
		return "";
	}

	int pixNew = 0, pix1 = 0, pix2 = 0;
	gdImageAlphaBlending(imNew, gdEffectOverlay);

	std::fstream fs;

	if (!done && width < 100 && height < 100 && !isGrey(fill))
	{
		String f = Configuration->getHTTProot()+"/piccolors.txt";
		fs.open(f.data(), std::ios::out | std::ios::trunc);

		if (!fs.is_open())
			doIt = false;
		else
		{
			doIt = true;
			fs << "Base image file: " << bm1 << std::endl;
			fs << "Mask image file: " << bm2 << std::endl;
			fs << "Dimension      : " << width << " x " << height << std::endl;
            fs << "Fill color     : " << NameFormat::toHex(fill, 8) << " [" << NameFormat::toHex(webColToGd(fill), 8) << "]" << std::endl;
            fs << "Border color   : " << NameFormat::toHex(border, 8) << " [" << NameFormat::toHex(webColToGd(border), 8) << "]" << std::endl << std::endl;
		}
	}

	for (int y = 0; y < height; y++)
	{
        String l, r;
        l.clear();
        r.clear();

		for (int x = 0; x < width; x++)
		{
			pix1 = gdImageGetTrueColorPixel(im1, x, y);
			pix2 = gdImageGetTrueColorPixel(im2, x, y);
			int base = getBaseColor(pix1, pix2, webColToGd(fill), webColToGd(border));
			pixNew = imgMultAlpha(pix2, base);

			if (doIt && !done)
			{
				l += NameFormat::toHex(pix1, 8) + " ";
				r += NameFormat::toHex(pix2, 8) + " ";
			}

			gdImageAlphaBlending(imNew, gdEffectReplace);	// switch to overwrite
			gdImageSetPixel(imNew, x, y, pixNew);			// set pixel
			gdImageAlphaBlending(imNew, gdEffectOverlay);	// switch back to overlay
		}

		if (doIt && !done)
            fs << l << " | " << r << std::endl;
	}

	gdImageDestroy(im1);
	gdImageDestroy(im2);
	gdImageSaveAlpha(imNew, 1);
	int rnd = 0;
	bool found = false;

	do
	{
		rnd = rand();
		found = false;

		for (size_t idx = 0; idx < tmpFiles.size(); idx++)
		{
			if (rnd == tmpFiles[idx])
			{
				found = true;
				break;
			}
		}
	}
	while(found);

	tmpFiles.push_back(rnd);
	String fname = String("chameleon/ChamImage_")+rnd+".png";
	String path = Configuration->getHTTProot()+"/"+fname;

	if (gdImageFile(imNew, path.data()) == GD_FALSE)
	{
		sysl->errlog(String("PushButton::createChameleonImage: Error writing an image to file ")+path);
		gdImageDestroy(imNew);
		return "";
	}

	if (doIt && !done)
	{
		fs << std::endl << "Wrote to file: " << path << std::endl << std::endl;

        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                fs << NameFormat::toHex(gdImageGetTrueColorPixel(imNew, x, y), 8) << " ";
            }

            fs << std::endl;
        }

		fs.close();
		done = true;
		doIt = false;
	}

	gdImageDestroy(imNew);
	return fname;
}

/*
 * Die Maske unter pix1 definiert über den roten und/oder grünen Farbkanal,
 * welche Farbe verwendet wird. Ist der rote Farbkanal gesetzt, wird die
 * Farbe unter "fill" zurückgegeben. Ist der grübe Farbkanal gesetzt, wird die
 * Farbe unter "border" zurück gegeben. Der blaue Farbkanal wird nicht
 * verwendet.
 * Ist der Alpha-Kanal auf 0x7f (127) gesetzt und sowohl der rote als auch der
 * grüne Farbkanal gleich 0, dann ist das Pixel nicht sichtbar.
 */
int PushButton::getBaseColor(int pix1, int pix2, int fill, int border)
{
	int alpha = gdTrueColorGetAlpha(pix1);
	int red = gdTrueColorGetRed(pix1);
	int green = gdTrueColorGetGreen(pix1);

	if (alpha == 127)
		return pix2;

	if (red && green)
	{
		int r1 = gdTrueColorGetRed(fill);
		int g1 = gdTrueColorGetGreen(fill);
		int b1 = gdTrueColorGetBlue(fill);
		int a1 = gdTrueColorGetAlpha(fill);
		int r2 = gdTrueColorGetRed(border);
		int g2 = gdTrueColorGetGreen(border);
		int b2 = gdTrueColorGetBlue(border);
		int a2 = gdTrueColorGetAlpha(border);
		int newR = (r1 + r2) / 2;
		int newG = (g1 + g2) / 2;
		int newB = (b1 + b2) / 2;
		int newA = (a1 + a2) / 2;
		return gdTrueColorAlpha(newR, newG, newB, newA);
	}

	if (red)
		return fill;

	if (green)
		return border;

	return 0x7f000000;
}

int PushButton::webColToGd(unsigned long col)
{
	int r1 = (col & 0xff000000) >> 24;
	int g1 = (col & 0x00ff0000) >> 16;
	int b1 = (col & 0x0000ff00) >> 8;
	int a1 = 0x007f - ((col & 0x000000ff) / 2);
	return gdTrueColorAlpha(r1, g1, b1, a1);
}

bool PushButton::isGrey(unsigned long col)
{
	int r1 = (col & 0xff000000) >> 24;
	int g1 = (col & 0x00ff0000) >> 16;
	int b1 = (col & 0x0000ff00) >> 8;

	if (r1 == g1 && r1 == b1)
		return true;

	return false;
}

bool PushButton::isGrey(int col)
{
	int r1 = gdTrueColorGetRed(col);
	int g1 = gdTrueColorGetGreen(col);
	int b1 = gdTrueColorGetBlue(col);

	if (r1 == g1 && r1 == b1)
		return true;

	return false;
}

int PushButton::imgMultAlpha(int mask, int img)
{
	int rm = gdTrueColorGetRed(mask);
	int gm = gdTrueColorGetGreen(mask);
	int bm = gdTrueColorGetBlue(mask);
	int am = gdTrueColorGetAlpha(mask);
	int ri = gdTrueColorGetRed(img);
	int gi = gdTrueColorGetGreen(img);
	int bi = gdTrueColorGetBlue(img);
	int ai = gdTrueColorGetAlpha(img);

	if (ai == 127)
		return mask;

	double rem = (double)rm  * (255 - am * 2) / 255.0;
	double gem = (double)gm  * (255 - am * 2) / 255.0;
	double bem = (double)bm  * (255 - am * 2) / 255.0;
	int newR = (int)(((double)ri + ((double)ri * rem / 255.0) / 2.0));
	int newG = (int)(((double)gi + ((double)gi * gem / 255.0) / 2.0));
	int newB = (int)(((double)bi + ((double)bi * bem / 255.0) / 2.0));
//	int newR = (int)((double)ri * rem / 127.0);
//	int newG = (int)((double)gi * gem / 127.0);
//	int newB = (int)((double)bi * bem / 127.0);

	if (newR > 255)
		newR = 255;

	if (newR < 0)
		newR = 0;

	if (newG > 255)
		newG = 255;

	if (newG < 0)
		newG = 0;

	if (newB > 255)
		newB = 255;

	if (newB < 0)
		newB = 0;

	return gdTrueColorAlpha(newR, newG, newB, 0);
}
