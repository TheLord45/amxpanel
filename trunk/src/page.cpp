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
#include <cstring>
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

Page::Page()
{
	sysl->TRACE(Syslog::ENTRY, std::string("Page::Page()"));
	status = false;
	paletteFile = "pal_001.xma";
	fontClass = 0;
	paletteClass = 0;
	iconClass = 0;
	buttonsDone = false;
	styleDone = false;
	webDone = false;
	Project = 0;
}

amx::Page::Page(const strings::String& file)
{
	sysl->TRACE(Syslog::ENTRY, std::string("Page::Page(const strings::String& file)"));
	status = false;
	pageFile = file;
	paletteFile = "pal_001.xma";
	fontClass = 0;
	paletteClass = 0;
	iconClass = 0;
	buttonsDone = false;
	styleDone = false;
	webDone = false;
	Project = 0;
}

bool amx::Page::parsePage()
{
	sysl->TRACE(String("Page::parsePage()"));

	if (status)
		return true;

	bool inButton = false;
	String lastName;
	int depth, oldDepth = -1;

	if (paletteFile.empty())
		paletteFile = "pal_001.xma";

	clear();
	String uri;
	uri.append(Configuration->getHTTProot());
	uri.append("/");
	uri.append(pageFile);
	sysl->TRACE(String("Page::parsePage: Reading file: ")+uri);

	try
	{
		ifstream xml(uri.data(), ifstream::binary);

		if (!xml)
		{
			sysl->errlog(String("Page::parsePage: Error opening the file ")+uri);
			return false;
		}

		std::string buffer;

		try
		{
			xml.seekg(0, xml.end);
			int length = xml.tellg();
			xml.seekg(0, xml.beg);
			buffer.resize(length, ' ');
			char *begin = &*buffer.begin();
			xml.read(begin, length);
		}
		catch (exception& e)
		{
			sysl->errlog(String("Page::parsePage: Error reading a file: ")+e.what());
			xml.close();
			return false;
		}

		xml.close();
		sysl->TRACE(String("Page::parsePage: length=")+buffer.length());
		// Convert from ISO-8859-15 to UTF-8.
//		String cbuf = isoToUTF(buffer);
		String cbuf = NameFormat::cp1250ToUTF8(buffer);
		buffer.clear();

//		xmlpp::TextReader reader(uri.toString());
		xmlpp::TextReader reader((const unsigned char *)cbuf.data(), cbuf.length());

		while (reader.read())
		{
			String name = string(reader.get_name());
			depth = reader.get_depth();

			if (depth < oldDepth)
			{
				oldDepth = depth;

				if (name.caseCompare("button") == 0)
					inButton = false;

				continue;
			}

			if (name.compare("#text") == 0)
				name = lastName;

			if (reader.has_attributes())
			{
				for (int i = 0; i < reader.get_attribute_count(); i++)
					sysl->TRACE(String("Page::parsePage: name=")+name+", depth="+reader.get_depth()+", attr="+reader.get_attribute(i));
			}
			else if (reader.has_value())
				sysl->TRACE(String("Page::parsePage: name=")+name+", depth="+reader.get_depth()+", value="+reader.get_value());
			else
				sysl->TRACE(String("Page::parsePage: name=")+name+", depth="+reader.get_depth());

			if (name.caseCompare("page") == 0 && reader.has_attributes())
			{
				String attr = string(reader.get_attribute(0));

				if (attr.caseCompare("page") == 0)
					page.type = PAGE;
				else if (attr.caseCompare("subpage") == 0)
					page.type = SUBPAGE;
				else
					page.type = PNONE;

				sysl->TRACE(String("Page::parsePage: page:")+page.type);
			}
			else if (name.caseCompare("pageID") == 0 && reader.has_value())
				page.pageID = atoi(reader.get_value().c_str());
			else if (name.caseCompare("name") == 0 && reader.has_value())
				page.name = reader.get_value();
			else if (name.caseCompare("left") == 0 && reader.has_value())
				page.left = atoi(reader.get_value().c_str());
			else if (name.caseCompare("top") == 0 && reader.has_value())
				page.top = atoi(reader.get_value().c_str());
			else if (name.caseCompare("width") == 0 && reader.has_value())
				page.width = atoi(reader.get_value().c_str());
			else if (name.caseCompare("height") == 0 && reader.has_value())
				page.height = atoi(reader.get_value().c_str());
			else if (name.caseCompare("group") == 0 && reader.has_value())
				page.group = reader.get_value();
			else if (name.caseCompare("showEffect") == 0 && reader.has_value())
				page.showEffect = (SHOWEFFECT)atoi(reader.get_value().c_str());
			else if (name.caseCompare("showTime") == 0 && reader.has_value())
				page.showTime = atoi(reader.get_value().c_str());
			else if (name.caseCompare("hideEffect") == 0 && reader.has_value())
				page.hideEffect = (SHOWEFFECT)atoi(reader.get_value().c_str());
			else if (name.caseCompare("hideTime") == 0 && reader.has_value())
				page.hideTime = atoi(reader.get_value().c_str());
			else if (name.caseCompare("button") == 0 && reader.has_attributes())
			{
				BUTTON_T button;
				button.clear();
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

				button.fb = FB_NONE;
				page.buttons.push_back(button);
				inButton = true;
				sysl->TRACE(String("Page::parsePage: Added for page ")+page.name+" button of type "+button.type);
			}
			else if (name.caseCompare("button") == 0)
				inButton = false;

			if (reader.get_depth() == 4 && inButton)
			{
				if (name.caseCompare("bi") == 0 && reader.has_value())
					page.buttons.back().bi = atoi(reader.get_value().c_str());
				else if (name.caseCompare("na") == 0 && reader.has_value())
					page.buttons.back().na = reader.get_value();
				else if (name.caseCompare("lt") == 0 && reader.has_value())
					page.buttons.back().lt = atoi(reader.get_value().c_str());
				else if (name.caseCompare("tp") == 0 && reader.has_value())
					page.buttons.back().tp = atoi(reader.get_value().c_str());
				else if (name.caseCompare("wt") == 0 && reader.has_value())
					page.buttons.back().wt = atoi(reader.get_value().c_str());
				else if (name.caseCompare("ht") == 0 && reader.has_value())
					page.buttons.back().ht = atoi(reader.get_value().c_str());
				else if (name.caseCompare("zo") == 0 && reader.has_value())
					page.buttons.back().zo = atoi(reader.get_value().c_str());
				else if (name.caseCompare("hs") == 0 && reader.has_value())
					page.buttons.back().hs = reader.get_value();
				else if (name.caseCompare("bs") == 0 && reader.has_value())
					page.buttons.back().bs = reader.get_value();
				else if (name.caseCompare("fb") == 0 && reader.has_value())
				{
					String value(reader.get_value());
					value.trim();

					if (value.caseCompare("channel") == 0)
						page.buttons.back().fb = FB_CHANNEL;
					else if (value.caseCompare("inverted channel") == 0)
						page.buttons.back().fb = FB_INV_CHANNEL;
					else if (value.caseCompare("always on") == 0)
						page.buttons.back().fb = FB_ALWAYS_ON;
					else if (value.caseCompare("momentary") == 0)
						page.buttons.back().fb = FB_MOMENTARY;
					else if (value.caseCompare("blink") == 0)
						page.buttons.back().fb = FB_BLINK;
					else
						page.buttons.back().fb = FB_NONE;
				}
				else if (name.caseCompare("ap") == 0 && reader.has_value())
					page.buttons.back().ap = atoi(reader.get_value().c_str());
				else if (name.caseCompare("ad") == 0 && reader.has_value())
					page.buttons.back().ad = atoi(reader.get_value().c_str());
				else if (name.caseCompare("cp") == 0 && reader.has_value())
					page.buttons.back().cp = atoi(reader.get_value().c_str());
				else if (name.caseCompare("ch") == 0 && reader.has_value())
					page.buttons.back().ch = atoi(reader.get_value().c_str());
				else if (name.caseCompare("lp") == 0 && reader.has_value())
					page.buttons.back().lp = atoi(reader.get_value().c_str());
				else if (name.caseCompare("lv") == 0 && reader.has_value())
					page.buttons.back().lv = atoi(reader.get_value().c_str());
				else if (name.caseCompare("va") == 0 && reader.has_value())
					page.buttons.back().va = atoi(reader.get_value().c_str());
				else if (name.caseCompare("rv") == 0 && reader.has_value())
					page.buttons.back().rv = atoi(reader.get_value().c_str());
				else if (name.caseCompare("rl") == 0 && reader.has_value())
					page.buttons.back().rl = atoi(reader.get_value().c_str());
				else if (name.caseCompare("rh") == 0 && reader.has_value())
					page.buttons.back().rh = atoi(reader.get_value().c_str());
				else if (name.caseCompare("dr") == 0 && reader.has_value())
					page.buttons.back().dr = reader.get_value().c_str();
				else if (name.caseCompare("pf") == 0 && reader.has_value())
				{
					page.buttons.back().pfName = reader.get_value().c_str();
					sysl->TRACE(String("Page::parsePage: found push page: ")+page.buttons.back().pfName);
				}
			}
			else if (inButton && reader.get_depth() == 3)	// Attributes
			{
				if (name.caseCompare("pf") == 0 && reader.has_attributes())
				{
					page.buttons.back().pfType = reader.get_attribute(0).c_str();	// FIXME: Find all commands and make an enum.
					sysl->TRACE(String("Page::parsePage: found push command: ")+page.buttons.back().pfType);
					// Known commands:
					// sShow      show popup
					// sHide      hide popup
					// scGroup    hide Group?
				}
			}

			if (inButton && name.caseCompare("sr") == 0 && reader.has_attributes())
			{
				SR_T sr;
				sr.clear();
				sr.number = atoi(reader.get_attribute(0).c_str());
				page.buttons.back().sr.push_back(sr);
				sysl->TRACE(String("Page::Page: Added for button ")+page.buttons.back().na+" sr with ID "+sr.number);
			}

			if (reader.get_depth() == 5 && inButton)
			{
				if (name.caseCompare("do") == 0 && reader.has_value())
					page.buttons.back().sr.back()._do = reader.get_value();
				else if (name.caseCompare("bs") == 0 && reader.has_value())
					page.buttons.back().sr.back().bs = reader.get_value();
				else if (name.caseCompare("mi") == 0 && reader.has_value())
				{
					String mi = reader.get_value().c_str();
					page.buttons.back().sr.back().mi = mi;

					if (!mi.empty())
					{
						int width, height;

						if (PushButton::getImageDimensions(Configuration->getHTTProot()+"/images/"+mi, &width, &height))
						{
							page.buttons.back().sr.back().mi_width = width;
							page.buttons.back().sr.back().mi_height = height;
						}
						else
						{
							page.buttons.back().sr.back().mi_width = 0;
							page.buttons.back().sr.back().mi_height = 0;
						}
					}
					else
					{
						page.buttons.back().sr.back().mi_width = 0;
						page.buttons.back().sr.back().mi_height = 0;
					}
				}
				else if (name.caseCompare("cb") == 0 && reader.has_value())
					page.buttons.back().sr.back().cb = reader.get_value();
				else if (name.caseCompare("cf") == 0 && reader.has_value())
					page.buttons.back().sr.back().cf = reader.get_value();
				else if (name.caseCompare("ct") == 0 && reader.has_value())
					page.buttons.back().sr.back().ct = reader.get_value();
				else if (name.caseCompare("ec") == 0 && reader.has_value())
					page.buttons.back().sr.back().ec = reader.get_value();
				else if (name.caseCompare("bm") == 0 && reader.has_value())
				{
					String bm = reader.get_value().c_str();
					page.buttons.back().sr.back().bm = bm;

					if (reader.has_attributes())
					{
						if (reader.get_attribute(0).compare("0") == 0)
							page.buttons.back().sr.back().dynamic = false;
						else
							page.buttons.back().sr.back().dynamic = true;
					}

					if (!bm.empty())
					{
						int width, height;

						if (PushButton::getImageDimensions(Configuration->getHTTProot()+"/images/"+bm, &width, &height))
						{
							page.buttons.back().sr.back().bm_width = width;
							page.buttons.back().sr.back().bm_height = height;
						}
						else
						{
							page.buttons.back().sr.back().bm_width = 0;
							page.buttons.back().sr.back().bm_height = 0;
						}
					}
					else
					{
						page.buttons.back().sr.back().bm_width = 0;
						page.buttons.back().sr.back().bm_height = 0;
					}
				}
				else if (name.caseCompare("ii") == 0 && reader.has_value())
					page.buttons.back().sr.back().ii = atoi(reader.get_value().c_str());
				else if (name.caseCompare("sb") == 0 && reader.has_value())
					page.buttons.back().sr.back().sb = atoi(reader.get_value().c_str());
				else if (name.caseCompare("ji") == 0 && reader.has_value())
					page.buttons.back().sr.back().ji = atoi(reader.get_value().c_str());
				else if (name.caseCompare("jb") == 0 && reader.has_value())
					page.buttons.back().sr.back().jb = atoi(reader.get_value().c_str());
				else if (name.caseCompare("ix") == 0 && reader.has_value())
					page.buttons.back().sr.back().ix = atoi(reader.get_value().c_str());
				else if (name.caseCompare("iy") == 0 && reader.has_value())
					page.buttons.back().sr.back().iy = atoi(reader.get_value().c_str());
				else if (name.caseCompare("jt") == 0 && reader.has_value())
					page.buttons.back().sr.back().jt = iToTo(atoi(reader.get_value().c_str()));
				else if (name.caseCompare("tx") == 0 && reader.has_value())
					page.buttons.back().sr.back().tx = atoi(reader.get_value().c_str());
				else if (name.caseCompare("ty") == 0 && reader.has_value())
					page.buttons.back().sr.back().ty = atoi(reader.get_value().c_str());
				else if (name.caseCompare("fi") == 0 && reader.has_value())
					page.buttons.back().sr.back().fi = atoi(reader.get_value().c_str());
				else if (name.caseCompare("te") == 0 && reader.has_value())
					page.buttons.back().sr.back().te = reader.get_value();
				else if (name.caseCompare("et") == 0 && reader.has_value())
					page.buttons.back().sr.back().et = atoi(reader.get_value().c_str());
				else if (name.caseCompare("ww") == 0 && reader.has_value())
					page.buttons.back().sr.back().ww = atoi(reader.get_value().c_str());
			}

			if (!inButton && name.caseCompare("sr") == 0 && reader.has_attributes())
			{
				SR_T sr;
				sr.number = atoi(reader.get_attribute(0).c_str());
				page.sr.push_back(sr);
				sysl->TRACE(String("Page::Page: Added for page ")+page.name+" sr with ID "+sr.number);
			}

			if (reader.get_depth() == 4 && !inButton)
			{
				if (name.caseCompare("bs") == 0 && reader.has_value())
					page.sr.back().bs = reader.get_value();
				else if (name.caseCompare("cb") == 0 && reader.has_value())
					page.sr.back().cb = reader.get_value();
				else if (name.caseCompare("cf") == 0 && reader.has_value())
					page.sr.back().cf = reader.get_value();
				else if (name.caseCompare("ct") == 0 && reader.has_value())
					page.sr.back().ct = reader.get_value();
				else if (name.caseCompare("ec") == 0 && reader.has_value())
					page.sr.back().ec = reader.get_value();
				else if (name.caseCompare("mi") == 0 && reader.has_value())
                {
					String mi = reader.get_value().c_str();
					page.sr.back().mi = mi;

					if (!mi.empty())
					{
						int width, height;

						if (PushButton::getImageDimensions(Configuration->getHTTProot()+"/images/"+mi, &width, &height))
						{
							page.sr.back().mi_width = width;
							page.sr.back().mi_height = height;
						}
						else
						{
							page.sr.back().mi_width = 0;
							page.sr.back().mi_height = 0;
						}
					}
					else
					{
						page.sr.back().mi_width = 0;
						page.sr.back().mi_height = 0;
					}
                }
				else if (name.caseCompare("bm") == 0 && reader.has_value())
				{
					String bm = reader.get_value().c_str();
					page.sr.back().bm = bm;

					if (reader.has_attributes())
					{
						if (reader.get_attribute(0).compare("0") == 0)
							page.sr.back().dynamic = false;
						else
							page.sr.back().dynamic = true;
					}

					if (!bm.empty())
					{
						int width, height;

						if (PushButton::getImageDimensions(Configuration->getHTTProot()+"/images/"+bm, &width, &height))
						{
							page.sr.back().bm_width = width;
							page.sr.back().bm_height = height;
						}
						else
						{
							page.sr.back().bm_width = 0;
							page.sr.back().bm_height = 0;
						}
					}
					else
					{
						page.sr.back().bm_width = 0;
						page.sr.back().bm_height = 0;
					}
				}
				else if (name.caseCompare("ii") == 0 && reader.has_value())
					page.sr.back().ii = atoi(reader.get_value().c_str());
				else if (name.caseCompare("ji") == 0 && reader.has_value())
					page.sr.back().ji = atoi(reader.get_value().c_str());
				else if (name.caseCompare("ix") == 0 && reader.has_value())
					page.sr.back().ix = atoi(reader.get_value().c_str());
				else if (name.caseCompare("iy") == 0 && reader.has_value())
					page.sr.back().iy = atoi(reader.get_value().c_str());
				else if (name.caseCompare("jt") == 0 && reader.has_value())
					page.sr.back().jt = iToTo(atoi(reader.get_value().c_str()));
				else if (name.caseCompare("tx") == 0 && reader.has_value())
					page.sr.back().tx = atoi(reader.get_value().c_str());
				else if (name.caseCompare("ty") == 0 && reader.has_value())
					page.sr.back().ty = atoi(reader.get_value().c_str());
				else if (name.caseCompare("fi") == 0 && reader.has_value())
					page.sr.back().fi = atoi(reader.get_value().c_str());
			}

			lastName = name;
			oldDepth = depth;
		}

		reader.close();
	}
	catch (xmlpp::internal_error& e)
	{
		sysl->errlog(string("Page::Page: XML parser error: ") + e.what());
		status = false;
		return status;
	}
	catch (exception& e)
	{
		sysl->errlog(string("Page::Page: XML fatal error: ") + e.what());
		status = false;
		return status;
	}

	status = true;
	return status;
}

amx::Page::~Page()
{
	sysl->TRACE(Syslog::EXIT, std::string("Page::Page(...)"));

//	if (paletteClass)
//		delete paletteClass;
}

void Page::serializeToFile()
{
	sysl->TRACE(String("Page::serializeToFile()"));

	fstream pgFile;
	String fname = Configuration->getHTTProot()+"/scripts/Page"+page.pageID+".js";

	try
	{
		pgFile.open(fname.toString(), ios::in | ios::out | ios::trunc | ios::binary);

		if (!pgFile.is_open())
		{
			sysl->errlog(String("Page::serializeToFile: Error opening file ")+fname);
			return;
		}
	}
	catch (const std::fstream::failure e)
	{
		sysl->errlog(std::string("Page::serializeToFile: I/O Error: ")+e.what());
		return;
	}

	pgFile << "var structPage" << page.pageID << " = {" << std::endl;
	pgFile << "\t\"name\":\"" << page.name << "\",\"ID\":" << page.pageID << ",\"type\":" << page.type << "," << std::endl;
	pgFile << "\t\"left\":" << page.left << ",\"top\":" << page.top << ",\"width\":" << page.width << ",\"height\":" << page.height << "," << std::endl;
	pgFile << "\t\"group\":\"" << page.group << "\",\"showEffect\":" << page.showEffect << ",\"showTime\":" << page.showTime << "," << std::endl;
	pgFile << "\t\"hideEffect\":" << page.hideEffect << ",\"hideTime\":" << page.hideTime << ",\"buttons\":[";

	for (size_t i = 0; i < page.buttons.size(); i++)
	{
		if (i > 0)
			pgFile << ",";

		pgFile << "\n\t\t{\"bname\":\"" << page.buttons[i].na << "\",\"bID\":" << page.buttons[i].bi << ",\"btype\":" << page.buttons[i].type << "," << std::endl;
		pgFile << "\t\t \"lt\":" << page.buttons[i].lt << ",\"tp\":" << page.buttons[i].tp << ",\"wt\":" << page.buttons[i].wt << ",\"ht\":" << page.buttons[i].ht << "," << std::endl;
		pgFile << "\t\t \"zo\":" << page.buttons[i].zo << ",\"hs\":\"" << page.buttons[i].hs << "\",\"bs\":\"" << page.buttons[i].bs << "\"," << std::endl;
		pgFile << "\t\t \"fb\":" << page.buttons[i].fb << ",\"ap\":" << page.buttons[i].ap << ",\"ad\":" << page.buttons[i].ad << ",\"ch\":" << page.buttons[i].ch << "," << std::endl;
		pgFile << "\t\t \"cp\":" << page.buttons[i].cp << ",\"lp\":" << page.buttons[i].lp << ",\"lv\":" << page.buttons[i].lv << ",\"dr\":\"" << page.buttons[i].dr << "\"," << std::endl;
		pgFile << "\t\t \"va\":" << page.buttons[i].va << ",\"rv\":" << page.buttons[i].rv << ",\"rl\":" << page.buttons[i].rl << ",\"rh\":" << page.buttons[i].rh << "," << std::endl;
		pgFile << "\t\t \"pfType\":\"" << page.buttons[i].pfType << "\",\"pfName\":\"" << page.buttons[i].pfName << "\",\"sr\":[";

		for (size_t j = 0; j < page.buttons[i].sr.size(); j++)
		{
			if (j > 0)
				pgFile << ",";

			pgFile << "\n\t\t\t{\"number\":" << page.buttons[i].sr[j].number << ",\"do\":\"" << page.buttons[i].sr[j]._do << "\",\"bs\":\"" << page.buttons[i].sr[j].bs << "\"," << std::endl;
			pgFile << "\t\t\t \"mi\":\"" << page.buttons[i].sr[j].mi << "\",\"cb\":\"" << page.buttons[i].sr[j].cb << "\",\"cf\":\"" << page.buttons[i].sr[j].cf << "\"," << std::endl;
			pgFile << "\t\t\t \"ct\":\"" << page.buttons[i].sr[j].ct << "\",\"ec\":\"" << page.buttons[i].sr[j].ec << "\",\"bm\":\"" << page.buttons[i].sr[j].bm << "\",";
			pgFile << "\"mi_width\":" << page.buttons[i].sr[j].mi_width << ",\"mi_height\":" << page.buttons[i].sr[j].mi_height << ",\"bm_width\":" << page.buttons[i].sr[j].bm_width << ",";
			pgFile << "\"bm_height\":" << page.buttons[i].sr[j].bm_height << "," << std::endl;
			pgFile << "\t\t\t \"dynamic\":" << ((page.buttons[i].sr[j].dynamic)?"true":"false") << ",\"sb\":" << page.buttons[i].sr[j].sb << ",\"ii\":" << page.buttons[i].sr[j].ii << "," << std::endl;
			pgFile << "\t\t\t \"ji\":" << page.buttons[i].sr[j].ji << ",\"jb\":" << page.buttons[i].sr[j].jb << ",\"ix\":" << page.buttons[i].sr[j].ix << "," << std::endl;
			pgFile << "\t\t\t \"iy\":" << page.buttons[i].sr[j].iy << ",\"fi\":" << page.buttons[i].sr[j].fi << ",\"te\":\"" << NameFormat::textToWeb(page.buttons[i].sr[j].te) << "\"," << std::endl;
			pgFile << "\t\t\t \"jt\":" << page.buttons[i].sr[j].jt << ",\"tx\":" << page.buttons[i].sr[j].tx << ",\"ty\":" << page.buttons[i].sr[j].ty << "," << std::endl;
			pgFile << "\t\t\t \"ww\":" << page.buttons[i].sr[j].ww << ",\"et\":" << page.buttons[i].sr[j].et << "}";
		}

		pgFile << "]\n\t\t}";
	}

	pgFile << "],\"sr\":[";

	for (size_t j = 0; j < page.sr.size(); j++)
	{
		if (j > 0)
			pgFile << ",";

		pgFile << "\n\t\t{\"number\":" << page.sr[j].number << ",\"do\":\"" << page.sr[j]._do << "\",\"bs\":\"" << page.sr[j].bs << "\"," << std::endl;
		pgFile << "\t\t \"mi\":\"" << page.sr[j].mi << "\",\"cb\":\"" << page.sr[j].cb << "\",\"cf\":\"" << page.sr[j].cf << "\"," << std::endl;
        pgFile << "\t\t \"mi_width\":" << page.sr[j].mi_width << ",\"mi_height\":" << page.sr[j].mi_height << ",\"bm_width\":" << page.sr[j].bm_width << ",";
		pgFile << "\"bm_height\":" << page.sr[j].bm_height << "," << std::endl;
		pgFile << "\t\t \"ct\":\"" << page.sr[j].ct << "\",\"ec\":\"" << page.sr[j].ec << "\",\"bm\":\"" << page.sr[j].bm << "\"," << std::endl;
		pgFile << "\t\t \"dynamic\":" << ((page.sr[j].dynamic)?"true":"false") << ",\"sb\":" << page.sr[j].sb << ",\"ii\":" << page.sr[j].ii << "," << std::endl;
		pgFile << "\t\t \"ji\":" << page.sr[j].ji << ",\"jb\":" << page.sr[j].jb << ",\"ix\":" << page.sr[j].ix << "," << std::endl;
		pgFile << "\t\t \"iy\":" << page.sr[j].iy << ",\"fi\":" << page.sr[j].fi << ",\"te\":\"" << NameFormat::textToWeb(page.sr[j].te) << "\"," << std::endl;
		pgFile << "\t\t \"jt\":" << page.sr[j].jt << ",\"tx\":" << page.sr[j].tx << ",\"ty\":" << page.sr[j].ty << ",";
		pgFile << "\"ww\":" << page.sr[j].ww << ",\"et\":" << page.sr[j].et << "}";
	}

	pgFile << "]\n\t};" << std::endl << std::endl;
	pgFile.close();
}

void amx::Page::generateButtons()
{
	sysl->TRACE(String("Page::generateButtons()"));

	if (buttonsDone)
		return;

	sysl->TRACE(String("Page::generateButtons: for page: ")+page.name);

	try
	{
		if (!paletteClass)
		{
			buttonsDone = false;
			sysl->errlog(String("Page::generateButtons: Missing palette initialization!"));
			return;
		}

		for (size_t i = 0; i < page.buttons.size(); i++)
		{
			if (page.buttons[i].ad > 0 || page.buttons[i].ch > 0)
			{
				if (!btArray.empty())
					btArray += ",";

				btArray += String("\n\t\t{\"pnum\":")+page.pageID+",\"bi\":"+page.buttons[i].bi+",";
				btArray += String("\"instances\":")+page.buttons[i].sr.size()+",";
				btArray += String("\"ap\":")+page.buttons[i].ap+",\"ac\":"+page.buttons[i].ad;
				btArray += String(",\"cp\":")+page.buttons[i].cp+",\"ch\":"+page.buttons[i].ch+"}";
			}

			PushButton pbt(page.buttons[i], paletteClass->getPalette());
			pbt.setFontClass(fontClass);
			pbt.setPageList(pgList);
			pbt.setIconClass(iconClass);
			pbt.setPageID(page.pageID);
			styleBuffer += pbt.getStyle();
			String buf = pbt.getWebCode();
			btWebBuffer.push_back(buf);
			scriptCode += pbt.getScriptCode();
			scrStart += pbt.getScriptCodeStart();

			if (!sBargraphs.empty() && pbt.haveBargraph())
				sBargraphs += ",\n";

			if (pbt.haveBargraph())
				sBargraphs += pbt.getBargraphs();
		}
	}
	catch (exception& e)
	{
		sysl->errlog(String("Page::generateButtons: Error: ")+e.what());
	}

	buttonsDone = true;
}

int amx::Page::findPage (const String& name)
{
	sysl->TRACE(String("Page::findPage (const String& name)"));

	for (size_t i = 0; i < pgList.size(); i++)
	{
		if (pgList[i].name.compare(name) == 0)
			return pgList[i].pageID;
	}

	return 0;
}

String& amx::Page::getStyleCode()
{
	sysl->TRACE(String("Page::getStyleCode()"));

	if (!status || styleDone || !paletteClass)
		return styleBuffer;

	String pgName = String("Page_")+page.pageID;
	styleBuffer = String(".")+pgName+" {\n";
	styleBuffer += String("  left: ")+String(page.left)+"px;\n";
	styleBuffer += String("  top: ")+String(page.top)+"px;\n";
	styleBuffer += String("  width: ")+String(page.width)+"px;\n";
	styleBuffer += String("  height: ")+String(page.height)+"px;\n";

	bool hasChameleon = (!page.sr[0].mi.empty() && !page.sr[0].bm.empty() && page.sr[0].bs.empty());
	sysl->TRACE(String("Page::getStyleCode: hasChameleon=")+hasChameleon+", mi="+page.sr[0].mi+", bm="+page.sr[0].bm+", bs="+page.sr[0].bs);

	if (page.sr.size() > 0 && page.sr[0].bm.length() > 0)
	{
		if (!hasChameleon)
		{
			styleBuffer += String("  background-color: ")+paletteClass->colorToString(paletteClass->getColor(page.sr[0].cf))+";\n";
			styleBuffer += String("  background-image: url(images/")+page.sr[0].bm+");\n";
		}
/*		else
		{
			String fname = PushButton::createChameleonImage(Configuration->getHTTProot()+"/images/"+page.sr[0].mi, Configuration->getHTTProot()+"/images/"+page.sr[0].bm, paletteClass->getColor(page.sr[0].cf), paletteClass->getColor(page.sr[0].cb));

			if (!fname.empty())
				styleBuffer += String("  background-image: url(")+fname+");\n";
			else
			{
				styleBuffer += String("  background-image: url(images/")+NameFormat::toURL(page.sr[0].mi)+"), ";
				styleBuffer += String("url(images/")+NameFormat::toURL(page.sr[0].bm)+");\n";
				styleBuffer += "  background-blend-mode: screen;\n";
			}
		}
*/
		styleBuffer += String("  color: ")+paletteClass->colorToString(paletteClass->getColor(page.sr[0].ct))+";\n";
		styleBuffer += "  background-repeat: no-repeat;\n";
	}

	if (page.type == SUBPAGE)
	{
		if (Project)
		{
			bool have = false;

			for (size_t x = 0; x < Project->panelSetup.powerUpPopup.size(); x++)
			{
				if (page.name.compare(Project->panelSetup.powerUpPopup[x]) == 0)
				{
					styleBuffer += "  display: inline-block;\n";	// Show the popup
					have = true;
					break;
				}
			}

//			if (!have)
//				styleBuffer += "  display: none;\n";	// Hide the popup
		}
//		else
//			styleBuffer += "  display: none;\n";		// Hide the popup

		styleBuffer += "  position: absolute;\n";		// Fixed position, don't move
//		styleBuffer += "  z-index: 1;\n";				// Display on top

		if (page.showEffect && page.showTime)
		{
			styleBuffer += String("  animation-name: ani_")+pgName+";\n";
			styleBuffer += String("  animation-duration: ")+String((double)page.showTime / 10.0)+"s;\n";
			styleBuffer += "}\n";
			styleBuffer += String("@keyframes ani_")+pgName+" {\n";

			switch (page.showEffect)
			{
				case SE_SLIDE_TOP:			// top
				case SE_SLIDE_TOP_FADE:
					styleBuffer += String("  from { top: -")+String(page.top)+"px; opacity: 0; }\n";
					styleBuffer += String("  to { top: 0; opacity: 1; }\n");
				break;
				case SE_SLIDE_LEFT:			// left
				case SE_SLIDE_LEFT_FADE:
					styleBuffer += String("  from { left: -")+String(page.left)+"px; opacity: 0; }\n";
					styleBuffer += String("  to { left: 0; opacity: 1; }\n");
				break;
				case SE_SLIDE_RIGHT:		// rght
				case SE_SLIDE_RIGHT_FADE:
					styleBuffer += String("  from { right: -")+String(page.left+page.width)+"px; opacity: 0; }\n";
					styleBuffer += String("  to { right: 0; opacity: 1; }\n");
				break;
				case SE_SLIDE_BOTTOM:		// bottom
				case SE_SLIDE_BOTTOM_FADE:
					styleBuffer += String("  from { top: ")+String(totalHeight)+"px; opacity: 0; }\n";
					styleBuffer += String("  to { top: ")+String(totalHeight-page.height)+"; opacity: 1; }\n";
				break;
				case SE_FADE:
					styleBuffer += String("  from { bottom: ")+String(totalHeight)+"px; opacity: 0; }\n";
					styleBuffer += String("  to { bottom: ")+String(totalHeight-page.height)+"; opacity: 1; }\n";
				break;
			}
		}

		styleBuffer += "}\n";
	}
	else if (page.type == PAGE)
	{
		if (Project)
		{
			if (page.name.compare(Project->panelSetup.powerUpPage) == 0)
				styleBuffer += "  display: block;\n";		// Show the page
//			else
//				styleBuffer += "  display: none;\n";		// Hide the page
		}
//		else
//			styleBuffer += "  display: none;\n";			// Hide the page

		styleBuffer += "  position: absolute;\n";			// Fixed position, don't move
		styleBuffer += "  overflow: hidden;\n";				// Enable scroll if needed
		styleBuffer += "}\n";
	}

	generateButtons();
	styleDone = true;
	return styleBuffer;
}

String& amx::Page::getWebCode()
{
	sysl->TRACE(std::string("Page::getWebCode()"));

	if (!status || webDone)
		return webBuffer;

	// Here we know, that we've never build the code to draw this page.
	// Therefore we'll do it now.
	// First we scan for all buttons and create them.
	generateButtons();

	String pgName = String("Page_")+page.pageID;
	webBuffer += String("<div id=\"")+pgName+"\" class=\""+pgName+"\">\n";

	for (size_t i = 0; i < btWebBuffer.size(); i++)
		webBuffer += btWebBuffer[i];

//	if (page.type != SUBPAGE)
		webBuffer += "</div>\n";

	webDone = true;
	return webBuffer;
}

void amx::Page::clear()
{
	sysl->TRACE(String("Page::clear()"));

	page.buttons.clear();
	page.group.clear();
	page.height = 0;
	page.hideEffect = SE_NONE;
	page.hideTime = 0;
	page.left = 0;
	page.name.clear();
	page.pageID = 0;
	page.showEffect = SE_NONE;
	page.showTime = 0;
	page.sr.clear();
	page.top = 0;
	page.type = PNONE;
	page.width = 0;
	status = false;
}

String amx::Page::isoToUTF(const String& str)
{
	String strOut;

	for (size_t i = 0; i < str.length(); i++)
	{
		uint8_t ch = str.at(i);

		if (ch < 0x80)
			strOut.push_back(ch);
		else
		{
			strOut.push_back(0xc0 | ch >> 6);
			strOut.push_back(0x80 | (ch & 0x3f));
		}
	}

	return strOut;
}

TEXT_ORIENTATION amx::Page::iToTo(int t)
{
	switch(t)
	{
		case 0: return ORI_ABSOLUT;
		case 1: return ORI_TOP_LEFT;
		case 2: return ORI_TOP_MIDDLE;
		case 3: return ORI_TOP_RIGHT;
		case 4: return ORI_CENTER_LEFT;
		case 5: return ORI_CENTER_MIDDLE;
		case 6: return ORI_CENTER_RIGHT;
		case 7: return ORI_BOTTOM_LEFT;
		case 8: return ORI_BOTTOM_MIDDLE;
		case 9: return ORI_BOTTOM_RIGHT;
		default: return ORI_CENTER_MIDDLE;
	}
}
