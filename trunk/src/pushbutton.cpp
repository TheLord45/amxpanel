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
#include "pushbutton.h"

extern Syslog *sysl;

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
		style += String(".button")+String(i)+"-"+NameFormat::toValidName(button.na)+" {\n";
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
			style += String(".button")+String(i)+"-"+NameFormat::toValidName(button.na)+"_font {\n";
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
		names = String("'button1-")+NameFormat::toValidName(button.na)+"','";
		names += String("'button2-")+NameFormat::toValidName(button.na)+"'";
	}

	for (size_t i = 0; i < button.sr.size(); i++)
	{
		String nm = String("button")+String(i)+"-"+NameFormat::toValidName(button.na);
		code += String("<div id=\"")+nm+"\" class=\""+nm+"\"";
		
		if (button.type == GENERAL && button.fb == FB_MOMENTARY && (i == 0 || i == 1))
		{
			code += String(" onmousedown=\"switchDisplay(")+names+",1);\"";
			code += String(" onmouseup=\"switchDisplay(")+names+",0);\"";
		}

		code += ">\n";

		if (button.sr[i].ct.length() > 0)
		{
			code += String("   <div class=\"")+nm+"_font\">";
			code += button.sr[i].ct+"</div>\n";
		}

		code += "</div>\n";
	}

	return code;
}
