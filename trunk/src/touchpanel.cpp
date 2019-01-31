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
	regCallback(bind(&TouchPanel::webMsg, this, placeholders::_1));
	readPages();
	// Start thread for websocket
/*	try
	{
		thread thr = thread([=] { run(); });
		thr.detach();
	}
	catch (std::exception &e)
	{
		sysl->errlog(std::string("TouchPanel::TouchPanel: Error creating a thread: ")+e.what());
	} */
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

	for (size_t i = 0; i < stPages.size(); i++)
	{
		if (!stPages[i].webcode.empty() && stPages[i].ID == id)
			return stPages[i].webcode;
	}

	for (size_t i = 0; i < stPopups.size(); i++)
	{
		if (!stPopups[i].webcode.empty() && stPopups[i].ID == id)
			return stPopups[i].webcode;
	}

	return "";
}

String TouchPanel::getPageStyle(int id)
{
	sysl->TRACE(String("TouchPanel::getPageStyle(int id)"));

	for (size_t i = 0; i < stPages.size(); i++)
	{
		if (!stPages[i].styles.empty() && stPages[i].ID == id)
			return stPages[i].styles;
	}

	for (size_t i = 0; i < stPopups.size(); i++)
	{
		if (!stPopups[i].styles.empty() && stPopups[i].ID == id)
			return stPopups[i].styles;
	}

	return "";
}

String TouchPanel::getPage(const String& name)
{
	sysl->TRACE(String("TouchPanel::getPage(const String& name)"));

	for (size_t i = 0; i < stPages.size(); i++)
	{
		if (!stPages[i].webcode.empty() && stPages[i].name.compare(name) == 0)
			return stPages[i].webcode;
	}

	for (size_t i = 0; i < stPopups.size(); i++)
	{
		if (!stPopups[i].webcode.empty() && stPopups[i].name.compare(name) == 0)
			return stPopups[i].webcode;
	}

	return "";
}

String TouchPanel::getPageStyle(const String& name)
{
	sysl->TRACE(String("TouchPanel::getPageStyle(const String& name)"));

	for (size_t i = 0; i < stPages.size(); i++)
	{
		if (!stPages[i].styles.empty() && stPages[i].name.compare(name) == 0)
			return stPages[i].styles;
	}

	for (size_t i = 0; i < stPopups.size(); i++)
	{
		if (!stPopups[i].styles.empty() && stPopups[i].name.compare(name) == 0)
			return stPopups[i].styles;
	}

	return "";
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
				String com(bef.port1);
				com.append("|");
				com.append((char *)&msg.content);
				send(com);

				/* FIXME: EinfÃÂ¼gen von Code fÃÂ¼r eine Schattenverwaltung der Pages.
				 *        Alle Befehle welche eine Seite in irgend einer Form
				 *        manipulieren, mÃÂ¼ssen im Code gespeichert werden.
				 *        Verbindet sich ein EndgerÃÂ¤t, muss seine OberflÃÂ¤che
				 *        korrekt gesetzt werden.
				 * 
				 * Alternative:
				 * Die verbindung zum Controller wird unterbrochen, sobald der
				 * Client nicht mehr verfÃÂ¼gbar ist und erst wieder aufgebaut,
				 * wenn der Client sich erneut verbindet. Dadurch wird der
				 * Seitenaufbau vom Controller gesteuert.
				 */
/*				if (com.contains("PAGE-"))
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
				} */
			}
		break;
	}

	// FIXME: Add a function to inform the client about a new page.
	busy = false;
}

/*
 * This function is called from class WebSocket every time a message from the
 * client web browser is received. The messages are processed and then the
 * result is send to the controller, if there is something to send.
 */
void TouchPanel::webMsg(std::string& msg)
{
	// FIXME: Add code here!
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

	if (gotPages)
		return;

	// delete all chameleon images
	String cmd = String("rm ")+Configuration->getHTTProot()+"/chameleon/ChamImage_*.png";
	system(cmd.data());

	try
	{
		vector<String> pgs = getPageFileNames();
		scBtArray = "\"buttons\":[";
		bool first = false;

		for (size_t i = 0; i < pgs.size(); i++)
		{
			sysl->TRACE(String("TouchPanel::readPages: Parsing page ")+pgs[i]);
			Page p(pgs[i]);
			p.setPalette(getPalettes());
			p.setParentSize(getProject().panelSetup.screenWidth, getProject().panelSetup.screenHeight);
			p.setFontClass(getFontList());
			p.setProject(&getProject());
			p.setIconClass(getIconClass());

			if (!p.parsePage())
			{
				sysl->warnlog(String("TouchPanel::readPages: Page ")+p.getPageName()+" had an error! Page will be ignored.");
				continue;
			}

			if (p.getType() == PAGE)
			{
				ST_PAGE pg;
				pg.ID = p.getPageID();
				pg.name = p.getPageName();
				pg.file = p.getFileName();
				pg.styles = p.getStyleCode();
				pg.webcode = p.getWebCode();
				scrBuffer += p.getScriptCode();
				scrStart += p.getScriptStart();

				if (p.haveBtArray())
				{
					if (first)
						scBtArray += ",";
					else
						first = true;

					scBtArray += p.getBtArray();
				}

				if (pg.name.compare(getProject().panelSetup.powerUpPage) == 0)
					pg.active = true;
				else
					pg.active = false;

				stPages.push_back(pg);
			}
			else
			{
				ST_POPUP pop;
				pop.ID = p.getPageID();
				pop.name = p.getPageName();
				pop.file = p.getFileName();
				pop.group = p.getGroupName();
				pop.styles = p.getStyleCode();
				pop.webcode = p.getWebCode();
				pop.active = false;
				scrBuffer += p.getScriptCode();
				scrStart += p.getScriptStart();

				if (p.haveBtArray())
				{
					if (first)
						scBtArray += ",";
					else
						first = true;

				    scBtArray += p.getBtArray();
                }

				for (size_t j = 0; j < getProject().panelSetup.powerUpPopup.size(); j++)
				{
					if (pop.name.compare(getProject().panelSetup.powerUpPopup[j]) == 0)
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
		}

		scBtArray += "]";
	}
	catch (exception& e)
	{
		sysl->errlog(String("TouchPanel::readPages: ")+e.what());
		exit(1);
	}
}

bool TouchPanel::parsePages()
{
	sysl->TRACE(std::string("TouchPanel::parsePages()"));
	fstream pgFile, cssFile;
	std::string fname = Configuration->getHTTProot().toString()+"/index.html";
	std::string cssname = Configuration->getHTTProot().toString()+"/amxpanel.css";

	try
	{
		pgFile.open(fname, ios::in | ios::out | ios::trunc | ios::binary);

		if (!pgFile.is_open())
		{
			sysl->errlog(std::string("TouchPanel::parsePages: Error opening file ")+fname);
			return false;
		}

		cssFile.open(cssname, ios::in | ios::out | ios::trunc | ios::binary);

		if (!cssFile.is_open())
		{
			sysl->errlog(std::string("TouchPanel::parsePages: Error opening file ")+cssname);
			pgFile.close();
			return false;
		}
	}
	catch (const std::fstream::failure e)
	{
		sysl->errlog(std::string("TouchPanel::parsePages: I/O Error: ")+e.what());
		return false;
	}

	// Font faces
	cssFile << getFontList()->getFontStyles();
	// The styles
	// Write the main pages
	for (size_t i = 0; i < stPages.size(); i++)
		cssFile << getPageStyle(stPages[i].ID);
	// write the styles of all popups
	for (size_t i = 0; i < stPopups.size(); i++)
		cssFile << getPageStyle(stPopups[i].ID);

	cssFile.close();

	// Page header
	pgFile << "<!DOCTYPE html>\n";
	pgFile << "<html>\n<head>\n<meta charset=\"UTF-8\">\n";
	pgFile << "<title>AMX Panel</title>\n";
	pgFile << "<link rel=\"stylesheet\" type=\"text/css\" href=\"amxpanel.css\">\n";
	// Scripts
	pgFile << "<script>\n";
	pgFile << "\"use strict\";\n";
	pgFile << "var pageName = \"\";\n";
	pgFile << "var wsocket;\n";
    writePages(pgFile);
	writePopups(pgFile);
	writeGroups(pgFile);
	writeBtArray(pgFile);
    writeIconTable(pgFile);
	pgFile << "var Pages;\n";
	pgFile << "var Popups;\n";
	pgFile << "var popupGroups;\n";
	pgFile << "var buttonArray;\n\n";
	pgFile << "Pages = JSON.parse(basePages);\n";
	pgFile << "Popups = JSON.parse(pageNames);\n";
	pgFile << "popupGroups = JSON.parse(pageGroups);\n";
	pgFile << "</script>\n";
	pgFile << "<script type=\"text/javascript\" src=\"amxpanel.js\"></script>\n";
	// Add some special script functions
	pgFile << "<script>\n";
	pgFile << scrBuffer << "\n";
	// This is the "main" program
	PROJECT_T prg = getProject();
	pgFile << "function main()\n{\n";
	pgFile << "\tshowPage('"<< prg.panelSetup.powerUpPage << "');\n";

	for (size_t i = 0; i < prg.panelSetup.powerUpPopup.size(); i++)
		pgFile << "\tshowPopup('" << prg.panelSetup.powerUpPopup[i] << "');\n";

	pgFile << String("\t")+scrStart+"\n";
	pgFile << "}\n";
	pgFile << "</script>\n";
	pgFile << "</head>\n";
	// The page body
	pgFile << "<body onload=\"main(); connect();\">\n";
//	pgFile << getPage(aid);

	for (size_t i = 0; i < stPages.size(); i++)
	{
		pgFile << stPages[i].webcode;
		writeAllPopups(pgFile);
		pgFile << "</div>\n";
	}

	pgFile << "</body>\n</html>\n";
	return true;
}

void TouchPanel::writeGroups (fstream& pgFile)
{
	sysl->TRACE(std::string("TouchPanel::writeGroups (fstream& pgFile)"));
	std::vector<strings::String> grName;
	pgFile << "var pageGroups = '{";

	// Find all unique group names
	for (size_t i = 0; i < stPopups.size(); i++)
	{
		bool have = false;

		for (size_t j = 0; j < grName.size(); j++)
		{
			if (grName[j].compare(stPopups[i].group) == 0)
			{
				have = true;
				break;
			}
		}

		if (have)
			continue;

		grName.push_back(stPopups[i].group);
	}

	// Go through group names and order pages together
	for (size_t i = 0; i < grName.size(); i++)
	{
		if (i > 0)
			pgFile << ",";

		pgFile << "\"" << grName[i] << "\":[";
		bool komma = false;

		for (size_t j = 0; j < stPopups.size(); j++)
		{
			if (grName[i].compare(stPopups[j].group) == 0)
			{
				if (komma)
					pgFile << ",";

				pgFile << "\"" << stPopups[j].name << "\"";
				komma = true;
			}
		}

		pgFile << "]";
	}

	pgFile << "}';\n\n";
}

void TouchPanel::writePages(std::fstream& pgFile)
{
	sysl->TRACE(String("TouchPanel::writePages(std::fstream& pgFile)"));
	bool first = true;
	pgFile << "var basePages = '{\"pages\":[";

	for (size_t i = 0; i < stPages.size(); i++)
	{
		if (!first)
			pgFile << ",";

		pgFile << "{\"name\":\"" << stPages[i].name << "\",\"ID\":" << stPages[i].ID << ",\"active\":false}";
		first = false;
	}

	pgFile << "]}';\n";
}

void TouchPanel::writePopups (fstream& pgFile)
{
	sysl->TRACE(std::string("TouchPanel::writePopups (fstream& pgFile)"));
	bool first = true;
	pgFile << "var pageNames = '{\"pages\":[";

	for (size_t i = 0; i < stPopups.size(); i++)
	{
		if (!first)
			pgFile << ",";

		pgFile << "{\"name\":\"" << stPopups[i].name << "\",\"ID\":" << stPopups[i].ID << ",\"group\":\"" << stPopups[i].group << "\",\"active\":false,\"lnpage\":\"\"}";
		first = false;
	}

	pgFile << "]}';\n";
}

void TouchPanel::writeAllPopups (fstream& pgFile)
{
	sysl->TRACE(std::string("TouchPanel::writeAllPopups (fstream& pgFile)"));

	for (size_t i = 0; i < stPopups.size(); i++)
		pgFile << stPopups[i].webcode;
}

void TouchPanel::writeStyles(std::fstream& pgFile)
{
	sysl->TRACE(String("TouchPanel::writeStyles(std::fstream& pgFile)"));
	vector<String> pgs = getPageFileNames();

	for (size_t i = 0; i < pgs.size(); i++)
	{
		Page pg(pgs[i]);
		pg.setPalette(getPalettes());
		pg.setParentSize(getProject().panelSetup.screenWidth, getProject().panelSetup.screenHeight);
		pg.setFontClass(getFontList());

		if (!pg.isOk())
		{
			sysl->warnlog(String("TouchPanel::writeStyles: Page ")+pg.getPageName()+" had an error! No styles will be written.");
			continue;
		}

		pgFile << pg.getStyleCode();
	}
}

void TouchPanel::writeBtArray(fstream& pgFile)
{
	sysl->TRACE(std::string("TouchPanel::writeBtArray(fstream& pgFile)"));

	pgFile << "var buttonArray = {" << scBtArray << "};\n";
}

void TouchPanel::writeIconTable(std::fstream& pgFile)
{
    sysl->TRACE(std::string("TouchPanel::writeIconTable(std::fstream& pgFile)"));

    Icon *ic = getIconClass();
    size_t ni = ic->numIcons();
    pgFile << "var iconArray =\n\t{\"icons\":[\n";
    
    for (size_t i = 0; i < ni; i++)
    {
        if (i > 0)
            pgFile << ",\n";

        pgFile << "\t\t{\"id\":" << ic->getID(i) << ",\"file\":\"" << ic->getFileName(i) << "\"}";
    }

    pgFile << "\n\t]};\n\n";
}
