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

#include <utility>
#include <memory>
#include <unistd.h>
#ifdef __APPLE__
#include <boost/asio/ip/tcp.hpp>
#else
#include <asio/ip/tcp.hpp>
#endif
#include "config.h"
#include "syslog.h"
#include "fontlist.h"
#include "amxnet.h"
#include "touchpanel.h"

#ifdef __APPLE__
using namespace boost;
#endif

using namespace amx;
using namespace strings;
using namespace std;

extern Config *Configuration;
extern Syslog *sysl;

TouchPanel::TouchPanel()
{
	sysl->TRACE(Syslog::ENTRY, String("TouchPanel::TouchPanel()"));
	openPage = 0;
	busy = false;
	readPages();
}

TouchPanel::~TouchPanel()
{
	sysl->TRACE(Syslog::EXIT, String("TouchPanel::TouchPanel()"));
}

bool TouchPanel::startClient()
{
	try
	{
		asio::io_context io_context;
		asio::ip::tcp::resolver r(io_context);
		AMXNet c(io_context);
		c.setCallback(bind(&TouchPanel::setCommand, this, placeholders::_1));

		c.start(r.resolve(Configuration->getAMXController().toString(), String(Configuration->getPort()).toString()));

		io_context.run();
	}
	catch (std::exception& e)
	{
		sysl->TRACE(String("TouchPanel::startClient: Exception: ")+e.what());
		return true;
	}

	return false;;
}

String TouchPanel::getPage(int id)
{
	sysl->TRACE(String("TouchPanel::getPage(int id)"));

	for (size_t i = 0; i < pages.size(); i++)
	{
		if (pages[i].isOk() && pages[i].getPageID() == id)
			return pages[i].getWebCode();
	}

	return "";
}

String TouchPanel::getPageStyle(int id)
{
	sysl->TRACE(String("TouchPanel::getPageStyle(int id)"));

	for (size_t i = 0; i < pages.size(); i++)
	{
		if (pages[i].isOk() && pages[i].getPageID() == id)
			return pages[i].getStyleCode();
	}

	return "";
}

String TouchPanel::getPage(const String& name)
{
	sysl->TRACE(String("TouchPanel::getPage(const String& name)"));

	for (size_t i = 0; i < pages.size(); i++)
	{
		if (pages[i].isOk() && pages[i].getPageName().compare(name) == 0)
			return pages[i].getWebCode();
	}

	return "";
}

String TouchPanel::getPageStyle(const String& name)
{
	sysl->TRACE(String("TouchPanel::getPageStyle(const String& name)"));

	for (size_t i = 0; i < pages.size(); i++)
	{
		if (pages[i].isOk() && pages[i].getPageName().compare(name) == 0)
			return pages[i].getStyleCode();
	}

	return "";
}

String TouchPanel::getStartPage()
{
	sysl->TRACE(String("TouchPanel::getStartPage()"));
	String pg, styles;

	// The head of the document
	pg = "<!DOCTYPE html>\n";
	pg += "<html>\n<head>\n<meta charset=\"UTF-8\">\n";
	pg += "<title>AMX Panel</title>\n";
	// The styles
	PROJECT_T prg = getProject();
	int aid = getActivePage();
	styles = getPageStyle(aid);

	for (size_t i = 0; i < stPopups.size(); i++)
	{
		for (size_t j = 0; j < stPopups[i].onPages.size(); j++)
		{
			if (stPopups[i].onPages[j] == aid && stPopups[i].active)
				styles += getPageStyle(stPopups[i].ID);
		}
	}

	// Scripts
	pg += "<script>\n";
	pg += "function switchDisplay(name1, name2, dStat, bid)\n{\n";
	pg += "\tvar bname;\n\tvar url;\n";
	pg += "\tif (dStat == 1)\n\t{\n";
	pg += "\t\tdocument.getElementById(name1).style.display = \"none\";\n";
	pg += "\t\tdocument.getElementById(name2).style.display = \"inline\";\n";
	pg += "\t\tbname = \"button_\"+bid;\n";
	pg += "\t\turl = makeURL(document.getElementById(bname).href);\n";
	pg += "\t\tdocument.getElementById(bname).href = url+\"1\";\n";
	pg += "\t}\n\telse\n\t{\n";
	pg += "\t\tdocument.getElementById(name1).style.display = \"inline\";\n";
	pg += "\t\tdocument.getElementById(name2).style.display = \"none\";\n";
	pg += "\t\tbname = \"button_\"+bid;\n";
	pg += "\t\turl = makeURL(document.getElementById(bname).href);\n";
	pg += "\t\tdocument.getElementById(bname).href = url+\"0\";\n";
	pg += "\t}\n";
	pg += "function makeURL(url)\n{\n";
	pg += "\tvar pos;\n\tvar u;\n\n";
	pg += "\tpos = url.lastIndexOf('=');\n\n";
	pg += "\tif (pos != -1)\n";
	pg += "\t\tu = url.substring(0, pos+1);\n\n";
	pg += "\treturn u;\n}\n}\n";
	pg += "</script>\n";
	pg += "</head>\n";
	// The page body
	pg += "<body>\n";
	pg += getPage(aid);

	for (size_t i = 0; i < stPopups.size(); i++)
	{
		for (size_t j = 0; j < stPopups[i].onPages.size(); j++)
		{
			if (stPopups[i].onPages[j] == aid && stPopups[i].active)
				pg += getPage(stPopups[i].ID);
		}
	}

	pg += "</body>\n</html>\n";
	// FIXME!
	return pg;
}

void TouchPanel::setCommand(const ANET_COMMAND& cmd)
{
	commands.push_back(cmd);

	if (busy)
		return;

	busy = true;

	while (commands.size() > 0)
	{
		ANET_COMMAND bef = commands.at(0);
		commands.erase(commands.begin());

		switch (bef.MC)
		{
			case 0x000c:
				ANET_MSG_STRING msg = bef.data.message_string;
				String com((char *)&msg.content);

				if (com.contains("PAGE-"))
				{
					String name = com.substring(5);
					int id = findPage(name);
					int aid = 0;

					if ((aid = getActivePage()) != id)
					{
						for (size_t i = 0; i < stPages.size(); i++)
						{
							if (stPages[i].ID == aid)
								stPages[i].active = false;
							else if (stPages[i].ID == id)
								stPages[i].active = true;
						}
					}
				}
				else if (com.caseCompare("@PPA") == 0)
				{
					// FIXME: Close all popups on current page
				}
				else if (com.caseCompare("@PPX") == 0)
				{
					// FIXME: Close all popups on all pages
				}
				else if (com.contains("@PPF-") == 0)
				{
					String name = com.substring(5);
					// FIXME: Close a partucular popup
				}
				else if (com.contains("@PPN-") == 0)
				{
					String name = com.substring(5);
					// FIXME: Open a partucular popup
				}
			}
		break;
	}

	// FIXME: Add a function to inform the client about a new page.
	busy = false;
}

String TouchPanel::requestPage(const http::server::Request& req)
{
	String uri = req.uri;
	// FIXME: Find the channel, port and other parameters to send to the
	// controller.
	// Return the actual page to the browser.
	return getStartPage();
}

int TouchPanel::findPage(const String& name)
{
	PROJECT_T pro = getProject();

	for (size_t i = 0; i < pro.pageLists.size(); i++)
	{
		PAGE_LIST_T pl = pro.pageLists[i];

		for (size_t j = 0; j < pl.pageList.size(); j++)
		{
			PAGE_ENTRY_T pe = pl.pageList[i];

			if (pe.name.compare(name) == 0)
				return pe.pageID;
		}
	}

	return 0;
}

int TouchPanel::getActivePage()
{
	for (size_t i = 0; i < stPages.size(); i++)
	{
		if (stPages[i].active)
			return stPages[i].ID;
	}

	return 0;
}

void TouchPanel::readPages()
{
	sysl->TRACE(String("TouchPanel::readPages()"));
	vector<String> pgs = getPageFileNames();

	for (size_t i = 0; i < pgs.size(); i++)
	{
		Page *p = new Page(pgs[i]);
		p->setPaletteFile(getProject().supportFileList.colorFile);
		p->setParentSize(getProject().panelSetup.screenWidth, getProject().panelSetup.screenHeight);
		p->setFontClass(getFontList());
		pages.push_back(*p);

		if (p->getType() == PAGE)
		{
			ST_PAGE pg;
			pg.ID = p->getPageID();

			if (p->getPageName().compare(getProject().panelSetup.powerUpPage) == 0)
				pg.active = true;
			else
				pg.active = false;

			stPages.push_back(pg);
		}
		else
		{
			ST_POPUP pop;
			pop.ID = p->getPageID();
			pop.group = p->getGroupName();
			pop.active = false;

			for (size_t j = 0; j < getProject().panelSetup.powerUpPopup.size(); j++)
			{
				if (p->getPageName().compare(getProject().panelSetup.powerUpPopup[i]) == 0)
				{
					pop.active = true;
					int aid = findPage(getProject().panelSetup.powerUpPage);

					if (aid > 0)
						pop.onPages.push_back(aid);

					break;
				}
			}

			stPopups.push_back(pop);
		}

		delete p;
	}
}
