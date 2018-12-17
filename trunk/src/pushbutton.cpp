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
		style = String(".button")+String(i)+"-"+NameFormat::toValidName(button.na)+" {\n";
		style += "  position: absolut;";
		style += "  border: none;\n";

		if (button.sr.size() >= 1)
		{
			int idx = (onOff) ? 1 : 0;
			style += colorToString(getColor(button.sr[idx].cb));
			style += ";\n";
			style += "  color: ";
			style += colorToString(getColor(button.sr[idx].ct));
			style += ";\n";
			style += "  padding: 1px 1px;\n";
			style += "  text-align: center;\n";
			style += "  text-decoration: none;\n";
			style += "  display: inline-block;\n";
			style += "  font-size: 16px;\n";
			style += "  margin: 1px 1px;\n";
			style += "  cursor: pointer;\n";
			style += "}\n\n";
			style += "</style>\n";
		}
	}

	return style;
}
