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

#include <string>
#include <iostream>
#include <locale>
#include <libxml++/libxml++.h>
#include <libxml++/parsers/textreader.h>
#include <glibmm.h>
#ifdef __APPLE__
   #include <boost/asio.hpp>
#else
   #include <asio.hpp>
#endif
#include "datetime.h"
#include "config.h"
#include "syslog.h"
#include "palette.h"
#include "nameformat.h"
#include "page.h"

extern Syslog *sysl;
extern Config *Configuration;

using namespace std;
using namespace amx;
using namespace strings;

amx::Page::Page(const strings::String& file)
{
	sysl->TRACE(Syslog::ENTRY, std::string("Page::Page(const strings::String& file)"));
	status = false;
	paletteFile = "pal_001.xma";
	clear();
	String uri = "file://";
	uri.append(Configuration->getHTTProot());
	uri.append("/panel/");
	uri.append(file);

	try
	{
        xmlpp::TextReader reader(uri.toString());

        while(reader.read())
        {
            String name = string(reader.get_name());

            if (name.caseCompare("page") == 0 && reader.has_attributes())
            {
                String attr = string(reader.get_attribute(0));

                if (attr.caseCompare("page") == 0)
                    page.type = PAGE;
                else if (attr.caseCompare("subpage") == 0)
                    page.type = SUBPAGE;
                else
                    page.type = PNONE;
            }
            else if (name.caseCompare("pageID") == 0)
                page.pageID = atoi(reader.get_value().c_str());
            else if (name.caseCompare("name") == 0)
                page.name = reader.get_value();
            else if (name.caseCompare("left") == 0)
                page.left = atoi(reader.get_value().c_str());
            else if (name.caseCompare("top") == 0)
                page.top = atoi(reader.get_value().c_str());
            else if (name.caseCompare("width") == 0)
                page.width = atoi(reader.get_value().c_str());
            else if (name.caseCompare("height") == 0)
                page.height = atoi(reader.get_value().c_str());
            else if (name.caseCompare("group") == 0)
                page.group = reader.get_value();
            else if (name.caseCompare("showEffect") == 0)
                page.showEffect = atoi(reader.get_value().c_str());
            else if (name.caseCompare("showTime") == 0)
                page.showTime = atoi(reader.get_value().c_str());
            else if (name.caseCompare("hideEffect") == 0)
                page.hideEffect = atoi(reader.get_value().c_str());
            else if (name.caseCompare("hideEffect") == 0)
                page.hideTime = atoi(reader.get_value().c_str());
            else if (name.caseCompare("button") == 0 && reader.has_attributes())
            {
                BUTTON_T button;
                String attr = string(reader.get_attribute(0));

                if (attr.caseCompare("general") == 0)
                    button.type = GENERAL;
                else if (attr.caseCompare("multi-state general") == 0)
                    button.type = MULTISTATE_GENERAL;
                else if (attr.caseCompare("bargraph") == 0)
                    button.type = BARGRAPH;
                else if (attr.caseCompare("multi-state bargraph") == 0)
                    button.type = MULTISTATE_BARGRAPH;
                else if (attr.caseCompare("joistick") == 0)
                    button.type = JOISTICK;
                else if (attr.caseCompare("text input") == 0)
                    button.type = TEXT_INPUT;
                else if (attr.caseCompare("computer control") == 0)
                    button.type = COMPUTER_CONTROL;
                else if (attr.caseCompare("take note") == 0)
                    button.type = TAKE_NOTE;
                else if (attr.caseCompare("sub-page view") == 0)
                    button.type = SUBPAGE_VIEW;

                page.buttons.push_back(button);
            }

            if (reader.get_depth() == 3)
            {
                if (name.caseCompare("bi") == 0)
                    page.buttons.back().bi = atoi(reader.get_value().c_str());
                else if (name.caseCompare("na") == 0)
                    page.buttons.back().na = reader.get_value();
                else if (name.caseCompare("lt") == 0)
                    page.buttons.back().lt = atoi(reader.get_value().c_str());
                else if (name.caseCompare("tp") == 0)
                    page.buttons.back().tp = atoi(reader.get_value().c_str());
                else if (name.caseCompare("wt") == 0)
                    page.buttons.back().wt = atoi(reader.get_value().c_str());
                else if (name.caseCompare("ht") == 0)
                    page.buttons.back().ht = atoi(reader.get_value().c_str());
                else if (name.caseCompare("zo") == 0)
                    page.buttons.back().zo = atoi(reader.get_value().c_str());
                else if (name.caseCompare("bs") == 0 && reader.has_value())
                    page.buttons.back().bs = reader.get_value();
                else if (name.caseCompare("fb") == 0 && reader.has_value())
                    page.buttons.back().fb = reader.get_value();
                else if (name.caseCompare("ap") == 0)
                    page.buttons.back().ap = atoi(reader.get_value().c_str());
                else if (name.caseCompare("cp") == 0)
                    page.buttons.back().cp = atoi(reader.get_value().c_str());
                else if (name.caseCompare("ch") == 0)
                    page.buttons.back().ch = atoi(reader.get_value().c_str());
                else if (name.caseCompare("lp") == 0)
                    page.buttons.back().lp = atoi(reader.get_value().c_str());
                else if (name.caseCompare("va") == 0)
                    page.buttons.back().va = atoi(reader.get_value().c_str());
                else if (name.caseCompare("rv") == 0)
                    page.buttons.back().rv = atoi(reader.get_value().c_str());
                else if (name.caseCompare("rl") == 0)
                    page.buttons.back().rl = atoi(reader.get_value().c_str());
                else if (name.caseCompare("rh") == 0)
                    page.buttons.back().rh = atoi(reader.get_value().c_str());
                else if (name.caseCompare("sr") == 0 && reader.has_attributes())
                {
                    SR_T sr;
                    sr.number = atoi(reader.get_attribute(0).c_str());
                    page.buttons.back().sr.push_back(sr);
                }
            }

            if (reader.get_depth() == 4)
            {
                if (name.caseCompare("bs") == 0)
                    page.buttons.back().sr.back().bs = reader.get_value();
                else if (name.caseCompare("cb") == 0)
                    page.buttons.back().sr.back().cb = reader.get_value();
                else if (name.caseCompare("cf") == 0)
                    page.buttons.back().sr.back().cf = reader.get_value();
                else if (name.caseCompare("ct") == 0)
                    page.buttons.back().sr.back().ct = reader.get_value();
                else if (name.caseCompare("ec") == 0)
                    page.buttons.back().sr.back().ec = reader.get_value();
                else if (name.caseCompare("bm") == 0)
                {
                    page.buttons.back().sr.back().bm = reader.get_value();

                    if (reader.has_attributes())
                    {
                        if (reader.get_attribute(0).compare("0") == 0)
                            page.buttons.back().sr.back().dynamic = false;
                        else
                            page.buttons.back().sr.back().dynamic = true;
                    }
                }
                else if (name.caseCompare("ji") == 0)
                    page.buttons.back().sr.back().ji = atoi(reader.get_value().c_str());
                else if (name.caseCompare("ix") == 0)
                    page.buttons.back().sr.back().ix = atoi(reader.get_value().c_str());
                else if (name.caseCompare("iy") == 0)
                    page.buttons.back().sr.back().iy = atoi(reader.get_value().c_str());
                else if (name.caseCompare("jt") == 0)
                    page.buttons.back().sr.back().jt = atoi(reader.get_value().c_str());
                else if (name.caseCompare("tx") == 0)
                    page.buttons.back().sr.back().tx = atoi(reader.get_value().c_str());
                else if (name.caseCompare("ty") == 0)
                    page.buttons.back().sr.back().ty = atoi(reader.get_value().c_str());
                else if (name.caseCompare("fi") == 0)
                    page.buttons.back().sr.back().fi = atoi(reader.get_value().c_str());
            }
        }

        reader.close();
    }
    catch (xmlpp::internal_error& e)
    {
        sysl->errlog(string("Page::Page: ")+e.what());
        status = false;
    }

    status = true;
}

Page::~Page()
{
    sysl->TRACE(Syslog::EXIT, std::string("Page::Page(...)"));
}

String& Page::getStyleCode()
{
	sysl->TRACE(String("Page::getStyleCode()"));
	Palette pal(paletteFile);

	if (!status || styleBuffer.length() > 0)
		return styleBuffer;

	if (page.type == SUBPAGE)
	{
		styleBuffer = String(".")+page.name+" {\n";
		styleBuffer += "  display: block;\n";		// Show the popup
		styleBuffer += "  position: fixed;\n";		// Fixed position, don't move
		styleBuffer += "  z-index: 1;\n";			// Display on top
		styleBuffer += String("  left: ")+String(page.left)+"px;\n";
		styleBuffer += String("  top: ")+String(page.top)+"px;\n";
		styleBuffer += String("  width: ")+String(page.width)+"px;\n";
		styleBuffer += String("  height: ")+String(page.height)+"px;\n";
		styleBuffer += "  overflow: auto;\n";		// Enable scroll if needed

		if (page.sr.size() > 0)
		{
			styleBuffer += String("  background-color: ")+pal.colorToString(pal.getColor(page.sr[0].cb))+";\n";
			styleBuffer += String("  color: ")+pal.colorToString(pal.getColor(page.sr[0].cf))+";\n";
		}

		if (page.showEffect && page.showTime)
		{
			styleBuffer += String("  animation-name: ani-")+NameFormat::toValidName(page.name)+";\n";
			styleBuffer += String("  animation-duration: ")+String((double)page.showTime / 10.0)+"s;\n";
			styleBuffer += "}\n";
			styleBuffer += String("@keyframes ani-")+NameFormat::toValidName(page.name)+" {\n";

			switch (page.showEffect)
			{
				case 1:		// top
					styleBuffer += String("  from { top: -")+String(page.top)+"px; opacity: 0;\n";
					styleBuffer += String("  to { top: 0; opacity: 1;\n");
				break;
				case 2:		// left
					styleBuffer += String("  from { left: -")+String(page.left)+"px; opacity: 0;\n";
					styleBuffer += String("  to { left: 0; opacity: 1;\n");
				break;
				case 3:		// rght
					styleBuffer += String("  from { right: -")+String(page.left+page.width)+"px; opacity: 0;\n";
					styleBuffer += String("  to { right: 0; opacity: 1;\n");
				break;
				case 4:		// bottom
					styleBuffer += String("  from { top: ")+String(totalHeight)+"px; opacity: 0;\n";
					styleBuffer += String("  to { top: ")+String(totalHeight-page.height)+"; opacity: 1;\n";
				break;
			}
		}

		styleBuffer += "}\n";
	}

	return styleBuffer;
}

String& Page::getWebCode()
{
    sysl->TRACE(std::string("Page::getWebCode()"));

    if (!status || webBuffer.length() > 0)
        return webBuffer;

    // Here we know, that we've never build the code to draw this page.
    // Therefore we'll do it now.
    // First we scan for all buttons and create them.
    for (size_t i = 0; i < page.buttons.size(); i++)
    {
        PushButton *pbt = new PushButton(page.buttons[i], paletteFile);
        buttons.push_back(*pbt);
        delete pbt;
    }

    return webBuffer;
}

void Page::clear()
{
	page.buttons.clear();
	page.group.clear();
	page.height = 0;
	page.hideEffect = 0;
	page.hideTime = 0;
	page.left = 0;
	page.name.clear();
	page.pageID = 0;
	page.showEffect = 0;
	page.showTime = 0;
	page.sr.clear();
	page.top = 0;
	page.type = PNONE;
	page.width = 0;
}
