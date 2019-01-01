/*
 * Copyright (C) 2018, 2019 by Andreas Theofilu <andreas@theosys.at>
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
#include <thread>
#include <exception>
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
#include "panelstruct.h"

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
	// Start thread for websocket
	try
	{
		thread thr = thread([=] { run(); });
		thr.detach();
	}
	catch (std::exception &e)
	{
		sysl->errlog(std::string("TouchPanel::TouchPanel: Error creating a thread: ")+e.what());
	}
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

		if (bef.device1 != Configuration->getAMXChannel())
			continue;

		switch (bef.MC)
		{
			case 0x000c:
				ANET_MSG_STRING msg = bef.data.message_string;
				String com((char *)&msg.content);

				if (com.contains("PAGE-"))
				{
					String name = com.substring(com.findFirstOf('-')+1);
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

bool TouchPanel::parsePages()
{
	sysl->TRACE(std::string("TouchPanel::parsePages()"));
	fstream pgFile;
	std::string fname = Configuration->getHTTProot().toString()+"/index.html";

	try
	{
		pgFile.open(fname, ios::in | ios::out | ios::trunc | ios::binary);

		if (!pgFile.is_open())
		{
			sysl->errlog(std::string("TouchPanel::parsePages: Error opening file ")+fname);
			return false;
		}
	}
	catch (const std::fstream::failure e)
	{
		sysl->errlog(std::string("TouchPanel::parsePages: I/O Error: ")+e.what());
		return false;
	}

	// Page header
	pgFile << "<!DOCTYPE html>\n";
	pgFile << "<html>\n<head>\n<meta charset=\"UTF-8\">\n";
	pgFile << "<title>AMX Panel</title>\n";

	// Font faces
	pgFile << getFontList()->getFontStyles();
	// The styles
	// write the styles of all popups
	for (size_t i = 0; i < stPopups.size(); i++)
		pgFile << getPageStyle(stPopups[i].ID);
	// Write the main pages
	for (size_t i = 0; i < stPages.size(); i++)
		pgFile << getPageStyle(stPages[i].ID);

	// Scripts
	pgFile << "<script>\n";
	pgFile << "\"use strict\";\n";
	pgFile << "var pageName = \"\";\n";
	pgFile << "var wsocket;\n";
	writePopups(pgFile);
	writeGroups(pgFile);
	pgFile << "var Popups;\n";
	pgFile << "var popupGroups;\n\n";
	pgFile << "Popups = JSON.parse(pageNames);\n";
	pgFile << "popupGroups = JSON.parse(pageGroups);\n\n";
	// findPageNumber
	pgFile << "function findPageNumber(name)\n{\n";
	pgFile << "\tvar i;\n\n";
	pgFile << "for (i in Popups.pages)\n\t{\n";
	pgFile << "\t\tif (Popups.pages[i].name == name)\n";
	pgFile << "\t\t\treturn Popups.pages[i].ID;\n";
	pgFile << "\t}\n\n\treturn -1;\n}\n";
	// findPageGroup
	pgFile << "function findPageGroup(name)\n{\n";
	pgFile << "\tvar i;\n\n";
	pgFile << "\tfor (i in Popups.pages)\n\t{\n";
	pgFile << "\t\tif (Popups.pages[i].name == name)\n";
	pgFile << "\t\t\treturn Popups.pages[i].group;\n\t}\n\n";
	pgFile << "\treturn "";\n}\n";
	// hideGroup
	pgFile << "function hideGroup(name)\n{\n";
	pgFile << "\tvar nm;\n\tvar group;\n\tvar i;\n";
	pgFile << "\tgroup = popupGroups[name];\n\n";
	pgFile << "\tfor (i in group)\n\t{\n";
	pgFile << "\t\tvar pg;\n";
	pgFile << "\t\tpg = findPageNumber(group[i]);\n";
	pgFile << "\t\tnm = 'Page_'+pg;\n";
	pgFile << "\t\tdocument.getElementById(nm).style.display = 'none';\n";
	pgFile << "\t}\n}\n";
	// showPopup
	pgFile << "function showPopup(name)\n{\n";
	pgFile << "\tvar pname;\n\tvar pID;\n\tvar group;\n\n";
	pgFile << "\tpID = findPageNumber(name);\n";
	pgFile << "\tgroup = findPageGroup(name);\n";
	pgFile << "\tpname = \"Page_\"+pID;\n";
	pgFile << "\thideGroup(group);\n";
	pgFile << "\tdocument.getElementById(pname).style.display = 'inline';\n}\n";
	// hidePopup
	pgFile << "function hidePopup(name)\n{\n";
	pgFile << "\tvar pname;\n\tvar pID;\n\n";
	pgFile << "\tpID = findPageNumber(name);\n";
	pgFile << "\tpname = \"Page_\"+pID;\n";
	pgFile << "\tdocument.getElementById(pname).style.display = 'none';\n}\n";
	// switchDisplay
	pgFile << "function switchDisplay(name1, name2, dStat, bid)\n{\n";
	pgFile << "\tvar bname;\n\tvar url;\n";
	pgFile << "\tif (dStat == 1)\n\t{\n";
	pgFile << "\t\tdocument.getElementById(name1).style.display = \"none\";\n";
	pgFile << "\t\tdocument.getElementById(name2).style.display = \"inline\";\n";
	pgFile << "\t\tbname = pageName+\":button_\"+bid;\n";
	pgFile << "\t\twriteText(\"PUSH:\"+bname+\":1;\");\n";
	pgFile << "\t}\n\telse\n\t{\n";
	pgFile << "\t\tdocument.getElementById(name1).style.display = \"inline\";\n";
	pgFile << "\t\tdocument.getElementById(name2).style.display = \"none\";\n";
	pgFile << "\t\tbname = pageName+\":button_\"+bid;\n";
	pgFile << "\t\twriteText(\"PUSH:\"+bname+\":0;\");\n";
	pgFile << "\t}\n";
	// connect()
	pgFile << "function connect()\n{\n";
	pgFile << "\ttry\n\t{\n";
	pgFile << "\t\twsocket = new WebSocket(\"wss://" << Configuration->getWebSocketServer() << ":" << Configuration->getSidePort() << "/\");\n";
	pgFile << "\t\twsocket.onopen = function() { wsocket.send('READY;'); }\n";
	pgFile << "\t\twsocket.onerror = function(error) { console.log(`WebSocket error: ${error}`); }\n";
	pgFile << "\t\twsocket.onmessage = function(e) { parseMessage(e.data); }\n";
	pgFile << "\t\twsocket.onclose = function() { console.log('WebSocket is closed!'); }\n";
	pgFile << "\t}\n\tcatch (exception)\n";
	pgFile << "\t{\n\tconsole.error(\"Error initializing: \"+exception);\n\t}\n}\n\n";
	// writeText()
	pgFile << "function writeText(msg)\n{\n";
	pgFile << "\tif (wsocket.readyState != WebSocket.OPEN)\n\t{\n";
	pgFile << "\t\talert(\"Socket not ready!\");\n\t\treturn;\n\t}\n";
	pgFile << "\twsocket.send(msg);\n}\n";
	// Check for time scripts
	pgFile << "function checkTime(i) {\n";
	pgFile << "\tif (i < 10) {i = \"0\" + i};\n";
	pgFile << "\treturn i;\n";
	pgFile << "}\n";
	// This is the "main" program
	PROJECT_T prg = getProject();
	int aid = findPage(prg.panelSetup.powerUpPage);
	pgFile << "showPage('Page_"<< aid << "');\n";

	for (size_t i = 0; i < prg.panelSetup.powerUpPopup.size(); i++)
		pgFile << "showPopup('" << prg.panelSetup.powerUpPopup[i] << "');\n";

	pgFile << "</script>\n";
	pgFile << "</head>\n";
	// The page body
	pgFile << "<body onload=\"connect();\">\n";
	pgFile << getPage(aid);

	for (size_t i = 0; i < pages.size(); i++)
	{
		if (pages[i].getType() != PAGE)
			continue;

		pgFile << pages[i].getWebCode();
		writeAllPopups(pgFile);
		pgFile << "</div>\n";
	}

	pgFile << "</body>\n</html>\n";
}

void TouchPanel::writeGroups (fstream& pgFile)
{
	sysl->TRACE("TouchPanel::writeGroups (fstream& pgFile)");
	std::vector<strings::String> grName;
	strings::String actGroup;
	bool repeat = true;
	bool found = false;
	pgFile << "var pageGroups = '{";

	while (repeat)
	{
		bool komma = false;

		if (found)
			komma = true;

		found = false;

		for (size_t i = 0; i < pages.size(); i++)
		{
			String gn = pages[i].getGroupName();

			if (pages[i].getType() != SUBPAGE || gn.empty())
				continue;

			// Make sure we don't have the page already put to any group
			bool have = false;

			for (size_t j = 0; j < grName.size(); j++)
			{
				if (grName[j].compare(gn) == 0)
				{
					have = true;
					break;
				}
			}

			if (have && actGroup.compare(gn) != 0)
			{
				have = false;
				continue;
			}

			if (!have)
			{
				grName.push_back(gn);
				actGroup = gn;

				if (komma)
					pgFile << ",";

				pgFile << "\"" << actGroup << "\":[";
			}
			else
				pgFile << ",";

			pgFile << "\"" << pages[i].getPageName() << "\"";
			found = true;
		}

		if (found)
			pgFile << "]";
		else
			repeat = false;
	}

	pgFile << "}';\n\n";
}

void TouchPanel::writePopups (fstream& pgFile)
{
	sysl->TRACE("TouchPanel::writePopups (fstream& pgFile)");
	bool first = true;
	pgFile << "var pageNames = \"'{\"pages\":[";

	for (size_t i = 0; i < pages.size(); i++)
	{
		if (pages[i].getType() == PAGE)
			continue;

		if (!first)
			pgFile << ",";

		pgFile << "{\"name\":\"" << pages[i].getPageName() << "\",\"ID\":" << pages[i].getPageID() << ",\"group\":\"" << pages[i].getGroupName() << "\"}";
		first = false;
	}

	pgFile << "]}';\n";
}

void TouchPanel::writeAllPopups (fstream& pgFile)
{
	sysl->TRACE("TouchPanel::writeAllPopups (fstream& pgFile)");

	for (size_t i = 0; i < pages.size(); i++)
	{
		if (pages[i].getType != SUBPAGE)
			continue;

		pgFile << pages[i].getWebCode();
	}
}
