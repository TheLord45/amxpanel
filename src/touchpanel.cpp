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

#include <cstdlib>
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
#include "touchpanel.h"
#include "panelstruct.h"
#include "nameformat.h"

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
	registrated = false;
	busy = false;
	webConnected = false;
	regCallback(bind(&TouchPanel::webMsg, this, placeholders::_1));
	regCallbackStop(bind(&TouchPanel::stopClient, this));
	regCallbackConnected(bind(&TouchPanel::setWebConnect, this, placeholders::_1));
	readPages();
	amxnet = 0;
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
	sysl->TRACE(String("TouchPanel::startClient()"));
	webConnected = false;

	try
	{
		asio::io_context io_context;
		asio::ip::tcp::resolver r(io_context);
		AMXNet c(io_context);
		amxnet = &c;
		c.setCallback(bind(&TouchPanel::setCommand, this, placeholders::_1));
		c.setCallbackConn(bind(&TouchPanel::getWebConnect, this));

		while (1)
		{
			if (webConnected && registrated)
			{
				sysl->TRACE(String("TouchPanel::startClient: Starting connection to controller ..."));
				c.start(r.resolve(Configuration->getAMXController().toString(), String(Configuration->getAMXPort()).toString()));
				io_context.run();
			}
			else
				sleep(1);
		}
	}
	catch (std::exception& e)
	{
		sysl->TRACE(String("TouchPanel::startClient: Exception: ")+e.what());
		return true;
	}

	return false;;
}

/*
 * Diese Methode wird aus der Klasse AMXNet heraus aufgerufen. Dazu wird die
 * Methode an die Klasse übergeben. Sie fungiert dann als Callback-Funktion und
 * wird immer dann aufgerufen, wenn vom Controller eine Mitteilung gekommen ist.
 */
void TouchPanel::setCommand(const ANET_COMMAND& cmd)
{
	sysl->TRACE(String("TouchPanel::setCommand(const ANET_COMMAND& cmd)"));

	if (!registrated)
		return;

	commands.push_back(cmd);

	if (busy)
		return;

	busy = true;
	String com;

	while (commands.size() > 0)
	{
		ANET_COMMAND& bef = commands.at(0);
		commands.erase(commands.begin());

		if (bef.device1 != Configuration->getAMXChannel())
			continue;

		switch (bef.MC)
		{
			case 0x0006:
			case 0x0018:	// feedback channel on
				com = bef.data.chan_state.port;
				com.append("|ON-");
				com.append(bef.data.chan_state.channel);
				send(com);
			break;

			case 0x0007:
			case 0x0019:	// feedback channel off
				com = bef.data.chan_state.port;
				com.append("|OFF-");
				com.append(bef.data.chan_state.channel);
				send(com);
			break;

			case 0x000a:	// level value change
				com = bef.data.message_value.port;
				com += "|LEVEL-";
				com += bef.data.message_value.value;
				com += ",";

				switch (bef.data.message_value.type)
				{
					case 0x10: com += bef.data.message_value.content.byte; break;
					case 0x11: com += bef.data.message_value.content.ch; break;
					case 0x20: com += bef.data.message_value.content.integer; break;
					case 0x21: com += bef.data.message_value.content.sinteger; break;
					case 0x40: com += bef.data.message_value.content.dword; break;
					case 0x41: com += bef.data.message_value.content.sdword; break;
					case 0x4f: com += bef.data.message_value.content.fvalue; break;
					case 0x8f: com += bef.data.message_value.content.dvalue; break;
				}

				send(com);
			break;

			case 0x000c:	// Command string
				ANET_MSG_STRING msg = bef.data.message_string;

				if (msg.length < strlen((char *)&msg.content))
				{
					amxBuffer.append((char *)&msg.content);
					break;
				}
				else if (amxBuffer.length() > 0)
				{
					amxBuffer.append((char *)&msg.content);
					size_t len = (amxBuffer.length() >= sizeof(msg.content)) ? 1499 : amxBuffer.length();
					strncpy((char *)&msg.content, amxBuffer.data(), len);
					msg.content[len] = 0;
				}

				com = msg.port;
				com.append("|");
				com.append(NameFormat::cp1250ToUTF8((char *)&msg.content));
				send(com);
			break;
		}
	}

	busy = false;
}

/*
 * This function is called from class WebSocket every time a message from the
 * client web browser is received. The messages are processed and then the
 * result is send to the controller, if there is something to send.
 */
void TouchPanel::webMsg(std::string& msg)
{
	sysl->TRACE(String("TouchPanel::webMsg(std::string& msg) [")+msg+"]");

	if (registrated && msg.find("PUSH:") != std::string::npos)
	{
		std::vector<String> parts = String(msg).split(":");
		ANET_SEND as;
		as.port = atoi(parts[1].data());
		as.channel = atoi(parts[2].data());
		int value = atoi(parts[3].data());

		if (value)
			as.MC = 0x0084;
		else
			as.MC = 0x0085;

		sysl->TRACE(String("TouchPanel::webMsg: port: ")+as.port+", channel: "+as.channel+", value: "+value+", MC: 0x"+NameFormat::toHex(as.MC, 4));

		if (amxnet != 0)
			amxnet->sendCommand(as);
		else
			sysl->warnlog(String("TouchPanel::webMsg: Class to talk with an AMX controller was not initialized!"));
	}
	else if (registrated && msg.find("LEVEL:") != std::string::npos)
	{
		std::vector<String> parts = String(msg).split(":");
		ANET_SEND as;
		as.port = atoi(parts[1].data());
		as.channel = atoi(parts[2].data());
		as.level = as.channel;
		as.value = atoi(parts[3].data());
		as.MC = 0x008a;
		sysl->TRACE(String("TouchPanel::webMsg: port: ")+as.port+", channel: "+as.channel+", value: "+as.value+", MC: 0x"+NameFormat::toHex(as.MC, 4));

		if (amxnet != 0)
			amxnet->sendCommand(as);
		else
			sysl->warnlog(String("TouchPanel::webMsg: Class to talk with an AMX controller was not initialized!"));
	}
	else if (msg.find("REGISTER:") != std::string::npos)
	{
		std::vector<String> parts = String(msg).split(":");

		if (parts.size() != 2)
			return;

		String regID = parts[1].substring((size_t)0, parts[1].length()-1);
		sysl->warnlog(String("TouchPanel::webMsg: Try to registrate with ID: %1 ...").arg(regID));
		std::vector<String> ht = Configuration->getHashTable(Configuration->getHashTablePath());

		if (isPresent(ht, regID))
		{
			registrated = true;
			sysl->warnlog(String("TouchPanel::webMsg: Registration with ID: %1 was successfull.").arg(regID));
			String com = "0|#REG-OK";
			send(com);
			return;
		}
		else
		{
			sysl->warnlog(String("TouchPanel::webMsg: Access for ID: %1 is denied!").arg(regID));
			registrated = false;
			String com = "0|#REG-NAK";
			send(com);
			com = "0|#ERR-Access denied!";
			send(com);
		}
	}
}

void TouchPanel::stopClient()
{
	sysl->TRACE(String("TouchPanel::stopClient()"));

	if (amxnet != 0)
		amxnet->stop();
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

void TouchPanel::readPages()
{
	sysl->TRACE(String("TouchPanel::readPages()"));

	if (gotPages || isParsed())
		return;

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

			pageList.push_back(p.getPageData());

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

				if (!sBargraphs.empty() && p.haveBargraphs())
					sBargraphs += ",\n";

				if (p.haveBargraphs())
					sBargraphs += p.getBargraphs();

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
				p.serializeToFile();
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

				if (!sBargraphs.empty() && p.haveBargraphs())
					sBargraphs += ",\n";

				if (p.haveBargraphs())
					sBargraphs += p.getBargraphs();

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
				p.serializeToFile();
			}
		}

		scBtArray += "]";
	}
	catch (std::exception& e)
	{
		sysl->errlog(String("TouchPanel::readPages: ")+e.what());
		exit(1);
	}
}

bool TouchPanel::parsePages()
{
	sysl->TRACE(std::string("TouchPanel::parsePages()"));

	fstream pgFile, cssFile, jsFile, cacheFile;
	// Did we've already parsed?
	if (isParsed())
		return true;

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

	cssFile << "* {\n\tbox-sizing: border-box;\n}\n";
	// Font faces
	cssFile << getFontList()->getFontStyles();
	// The styles
	// Write the main pages
//	for (size_t i = 0; i < stPages.size(); i++)
//		cssFile << getPageStyle(stPages[i].ID);
	// write the styles of all popups
//	for (size_t i = 0; i < stPopups.size(); i++)
//		cssFile << getPageStyle(stPopups[i].ID);

	cssFile.close();

	try
	{
		std::string maniName = Configuration->getHTTProot().toString()+"/manifest.json";
		jsFile.open(maniName, ios::in | ios::out | ios::trunc | ios::binary);

		if (!jsFile.is_open())
		{
			sysl->errlog(std::string("TouchPanel::parsePages: Error opening file ")+maniName);
			return false;
		}

		jsFile << "{" << std::endl << "\t\"short_name\": \"AMXP\"," << std::endl;
		jsFile << "\t\"name\": \"AMX Panel\"," << std::endl;
		jsFile << "\t\"icons\": [" << std::endl << "\t\t{" << std::endl;
		jsFile << "\t\t\t\"src\": \"images/icon.png\"," << std::endl;
		jsFile << "\t\t\t\"type\": \"image/png\"," << std::endl;
		jsFile << "\t\t\t\"sizes\": \"256x256\"" << std::endl << "\t\t}" << std::endl;
		jsFile << "\t]," << std::endl;

		if (Configuration->getWSStatus())
			jsFile << "\t\"start_url\": \"https://";
		else
			jsFile << "\t\"start_url\": \"http://";

		jsFile << Configuration->getWebSocketServer() << "/" << Configuration->getWebLocation().toString() << "/index.html\"," << std::endl;
		jsFile << "\t\"background_color\": \"#5a005a\"," << std::endl;
		jsFile << "\t\"display\": \"standalone\"," << std::endl;

		if (Configuration->getWSStatus())
			jsFile << "\t\"scope\": \"https://";
		else
			jsFile << "\t\"scope\": \"http://";

		jsFile << Configuration->getWebSocketServer() << "/" << Configuration->getWebLocation().toString() << "/\"," << std::endl;
		jsFile << "\t\"theme_color\": \"#5a005a\"," << std::endl;
		jsFile << "\t\"orientation\": \"landscape\"" << std::endl << "}" << std::endl;
		jsFile.close();
	}
	catch (const std::fstream::failure e)
	{
		sysl->errlog(std::string("TouchPanel::parsePages: I/O Error: ")+e.what());
		return false;
	}
/****
	std::string cacheName = Configuration->getHTTProot().toString()+"/cache.appcache";

	try
	{
		cacheFile.open(cacheName, ios::in | ios::out | ios::trunc | ios::binary);

		if (!cacheFile.is_open())
		{
			sysl->errlog(std::string("TouchPanel::parsePages: Error opening file ")+cacheName);
			return false;
		}
	}
	catch (const std::fstream::failure e)
	{
		sysl->errlog(std::string("TouchPanel::parsePages: I/O Error: ")+e.what());
		return false;
	}

	cacheFile << "CACHE MANIFEST" << std::endl;
****/
	getFontList()->serializeToJson();

	// Page header
	pgFile << "<!DOCTYPE html>\n";
//	pgFile << "<html manifest=\"cache.appcache\">\n<head>\n<meta charset=\"UTF-8\">\n";
	pgFile << "<html>\n<head>\n<meta charset=\"UTF-8\">\n";
	pgFile << "<title>AMX Panel</title>\n";
	pgFile << "<meta name=\"viewport\" content=\"width=device-width, height=device-height, initial-scale=0.7, minimum-scale=0.7, maximum-scale=1.0, user-scalable=yes\"/>\n";
	pgFile << "<meta name=\"mobile-web-app-capable\" content=\"yes\" />" << std::endl;
	pgFile << "<meta name=\"apple-mobile-web-app-capable\" content=\"yes\" />\n";
	pgFile << "<meta name=\"apple-mobile-web-app-status-bar-style\" content=\"black\" />" << std::endl;
	pgFile << "<link rel=\"manifest\" href=\"manifest.json\">" << std::endl;
	pgFile << "<link rel=\"icon\" sizes=\"256x256\" href=\"images/icon.png\">" << std::endl;
	pgFile << "<link rel=\"stylesheet\" type=\"text/css\" href=\"amxpanel.css\">\n";
	// Scripts
	pgFile << "<script type=\"text/javascript\" src=\"scripts/imprint.min.js\"></script>\n";
	pgFile << "<script>\n";
	pgFile << "\"use strict\";\n";
	pgFile << "var fingerprint = \"\";\n";
	pgFile << "var pageName = \"\";\n";
	pgFile << "var wsocket;\n";
	pgFile << "var wsStatus = 0;" << std::endl << std::endl;
	pgFile << "var browserTests = [\n";
	pgFile << "\t\"audio\",\n\t\"availableScreenResolution\",\n\t\"canvas\",\n";
	pgFile << "\t\"colorDepth\",\n\t\"cookies\",\n\t\"cpuClass\",\n\t\"deviceDpi\",\n";
	pgFile << "\t\"doNotTrack\",\n\t\"indexedDb\",\n\t\"installedFonts\",\n";
	pgFile << "\t\"installedLanguages\",\n";
	pgFile << "\t\"language\",\n\t\"localIp\",\n\t\"localStorage\",\n\t\"pixelRatio\",\n";
	pgFile << "\t\"platform\",\n\t\"plugins\",\n\t\"processorCores\",\n\t\"screenResolution\",\n";
	pgFile << "\t\"sessionStorage\",\n\t\"timezoneOffset\",\n\t\"touchSupport\",\n";
	pgFile << "\t\"userAgent\",\n\t\"webGl\"\n];\n\n";

	try
	{
		std::string jsname = Configuration->getHTTProot().toString()+"/scripts/pages.js";
		jsFile.open(jsname, ios::in | ios::out | ios::trunc | ios::binary);

		if (!jsFile.is_open())
		{
			sysl->errlog(std::string("TouchPanel::parsePages: Error opening file ")+jsname);
			pgFile.close();
			return false;
		}

		jsFile << "var Pages;\n\n";
		writePages(jsFile);
		jsFile.close();
	}
	catch (const std::fstream::failure e)
	{
		sysl->errlog(std::string("TouchPanel::parsePages: I/O Error: ")+e.what());
		pgFile.close();
//		cacheFile.close();
		return false;
	}

	try
	{
		std::string jsname = Configuration->getHTTProot().toString()+"/scripts/popups.js";
		jsFile.open(jsname, ios::in | ios::out | ios::trunc | ios::binary);

		if (!jsFile.is_open())
		{
			sysl->errlog(std::string("TouchPanel::parsePages: Error opening file ")+jsname);
			pgFile.close();
			return false;
		}

		jsFile << "var Popups;\n\n";
		writePopups(jsFile);
		jsFile.close();
	}
	catch (const std::fstream::failure e)
	{
		sysl->errlog(std::string("TouchPanel::parsePages: I/O Error: ")+e.what());
		pgFile.close();
//		cacheFile.close();
		return false;
	}

	try
	{
		std::string jsname = Configuration->getHTTProot().toString()+"/scripts/groups.js";
		jsFile.open(jsname, ios::in | ios::out | ios::trunc | ios::binary);

		if (!jsFile.is_open())
		{
			sysl->errlog(std::string("TouchPanel::parsePages: Error opening file ")+jsname);
			pgFile.close();
			return false;
		}

		jsFile << "var popupGroups;\n\n";
		writeGroups(jsFile);
		jsFile.close();
	}
	catch (const std::fstream::failure e)
	{
		sysl->errlog(std::string("TouchPanel::parsePages: I/O Error: ")+e.what());
		pgFile.close();
//		cacheFile.close();
		return false;
	}

	try
	{
		std::string jsname = Configuration->getHTTProot().toString()+"/scripts/btarray.js";
		jsFile.open(jsname, ios::in | ios::out | ios::trunc | ios::binary);

		if (!jsFile.is_open())
		{
			sysl->errlog(std::string("TouchPanel::parsePages: Error opening file ")+jsname);
			pgFile.close();
			return false;
		}

		jsFile << "var buttonArray;\n\n";
		writeBtArray(jsFile);
		jsFile.close();
	}
	catch (const std::fstream::failure e)
	{
		sysl->errlog(std::string("TouchPanel::parsePages: I/O Error: ")+e.what());
		pgFile.close();
//		cacheFile.close();
		return false;
	}

	try
	{
		std::string jsname = Configuration->getHTTProot().toString()+"/scripts/icons.js";
		jsFile.open(jsname, ios::in | ios::out | ios::trunc | ios::binary);

		if (!jsFile.is_open())
		{
			sysl->errlog(std::string("TouchPanel::parsePages: Error opening file ")+jsname);
			pgFile.close();
			return false;
		}

		writeIconTable(jsFile);
		jsFile.close();
	}
	catch (const std::fstream::failure e)
	{
		sysl->errlog(std::string("TouchPanel::parsePages: I/O Error: ")+e.what());
		pgFile.close();
//		cacheFile.close();
		return false;
	}

	try
	{
		std::string jsname = Configuration->getHTTProot().toString()+"/scripts/bargraphs.js";
		jsFile.open(jsname, ios::in | ios::out | ios::trunc | ios::binary);

		if (!jsFile.is_open())
		{
			sysl->errlog(std::string("TouchPanel::parsePages: Error opening file ")+jsname);
			pgFile.close();
			return false;
		}

		writeBargraphs(jsFile);
		jsFile.close();
	}
	catch (const std::fstream::failure e)
	{
		sysl->errlog(std::string("TouchPanel::parsePages: I/O Error: ")+e.what());
		pgFile.close();
//		cacheFile.close();
		return false;
	}

	try
	{
		std::string jsname = Configuration->getHTTProot().toString()+"/scripts/palette.js";
		jsFile.open(jsname, ios::in | ios::out | ios::trunc | ios::binary);

		if (!jsFile.is_open())
		{
			sysl->errlog(std::string("TouchPanel::parsePages: Error opening file ")+jsname);
			pgFile.close();
			return false;
		}

		jsFile << getPalettes()->getJson();
		jsFile.close();
	}
	catch (const std::fstream::failure e)
	{
		sysl->errlog(std::string("TouchPanel::parsePages: I/O Error: ")+e.what());
		pgFile.close();
//		cacheFile.close();
		return false;
	}

	pgFile << "</script>" << std::endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/pages.js\"></script>" << std::endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/popups.js\"></script>" << std::endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/groups.js\"></script>" << std::endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/btarray.js\"></script>" << std::endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/icons.js\"></script>" << std::endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/bargraphs.js\"></script>" << std::endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/palette.js\"></script>" << std::endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/fonts.js\"></script>" << std::endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/chameleon.js\"></script>" << std::endl << std::endl;
/****
	cacheFile << "scripts/pages.js" << std::endl;
	cacheFile << "scripts/popups.js" << std::endl;
	cacheFile << "scripts/groups.js" << std::endl;
	cacheFile << "scripts/btarray.js" << std::endl;
	cacheFile << "scripts/icons.js" << std::endl;
	cacheFile << "scripts/bargraphs.js" << std::endl;
	cacheFile << "scripts/palette.js" << std::endl;
	cacheFile << "scripts/fonts.js" << std::endl;
	cacheFile << "scripts/chameleon.js" << std::endl;
****/
	for (size_t i = 0; i < stPages.size(); i++)
	{
		pgFile << "<script type=\"text/javascript\" src=\"scripts/Page" << stPages[i].ID << ".js\"></script>" << std::endl;
//		cacheFile << "scripts/Page" << stPages[i].ID << ".js" << std::endl;
	}

	pgFile << std::endl;

	for (size_t i = 0; i < stPopups.size(); i++)
	{
		pgFile << "<script type=\"text/javascript\" src=\"scripts/Page" << stPopups[i].ID << ".js\"></script>" << std::endl;
//		cacheFile << "scripts/Page" << stPopups[i].ID << ".js" << std::endl;
	}

	pgFile << std::endl << "<script type=\"text/javascript\" src=\"scripts/page.js\"></script>" << std::endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/amxpanel.js\"></script>" << std::endl;
/****
	cacheFile << "scripts/page.js" << std::endl;
	cacheFile << "scripts/amxpanel.js" << std::endl;
	cacheFile << "amxpanel.css" << std::endl;
	cacheFile << "index.html" << std::endl;
	cacheFile << "manifest.json" << std::endl;

	std::vector<String> bitmaps;

	for (size_t i = 0; i < pageList.size(); i++)
	{
		for (size_t j = 0; j < pageList[i].sr.size(); j++)
		{
			if (pageList[i].sr[j].bm.length() > 0 && pageList[i].sr[j].sb == 0)
			{
				if (!isPresent(bitmaps, pageList[i].sr[j].bm))
					bitmaps.push_back(NameFormat::toURL(pageList[i].sr[j].bm));
			}

			if (pageList[i].sr[j].mi.length() > 0)
			{
				if (!isPresent(bitmaps, pageList[i].sr[j].mi))
					bitmaps.push_back(NameFormat::toURL(pageList[i].sr[j].mi));
			}
		}

		for (size_t z = 0; z < pageList[i].buttons.size(); z++)
		{
			for (size_t j = 0; j < pageList[i].buttons[z].sr.size(); j++)
			{
				if (pageList[i].buttons[z].sr[j].bm.length() > 0 && pageList[i].buttons[z].sr[j].sb == 0)
				{
					if (!isPresent(bitmaps, pageList[i].buttons[z].sr[j].bm))
						bitmaps.push_back(NameFormat::toURL(pageList[i].buttons[z].sr[j].bm));
				}

				if (pageList[i].buttons[z].sr[j].mi.length() > 0)
				{
					if (!isPresent(bitmaps, pageList[i].buttons[z].sr[j].mi))
						bitmaps.push_back(NameFormat::toURL(pageList[i].buttons[z].sr[j].mi));
				}
			}
		}
	}

	for (size_t i = 0; i < bitmaps.size(); i++)
		cacheFile << "images/" << bitmaps[i] << std::endl;

	cacheFile << "NETWORK:" << std::endl << "*" << std::endl;
	cacheFile.close();
****/
	// Add some special script functions
	pgFile << "<script>\n";
	pgFile << scrBuffer << "\n";
	// This is the WebSocket connection function
	pgFile << "function connect()\n{\n";
	pgFile << "\ttry\n\t{\n";

	if (Configuration->getWSStatus())
		pgFile << "\t\twsocket = new WebSocket(\"wss://" << Configuration->getWebSocketServer() << ":" << Configuration->getSidePort() << "/\");\n";
	else
		pgFile << "\t\twsocket = new WebSocket(\"ws://" << Configuration->getWebSocketServer() << ":" << Configuration->getSidePort() << "/\");\n";

	pgFile << "\t\twsocket.onopen = function() {" << std::endl;
    pgFile << "\t\t\tgetRegistrationID();\n\t\t\tsetOnlineStatus(1);\n";
	pgFile << "\t\t\tif (typeof registrationID == \"string\" && registrationID.length > 0)\n";
	pgFile << "\t\t\t\twsocket.send('REGISTER:'+registrationID+';');\n";
	pgFile << "\t\t\telse\n\t\t\t\terrlog(\"connect: Missing registration ID!\");\n\t\t}\n";
	pgFile << "\t\twsocket.onerror = function(error) { errlog('WebSocket error: '+error); setOnlineStatus(0); }\n";
	pgFile << "\t\twsocket.onmessage = function(e) { parseMessage(e.data); }\n";
	pgFile << "\t\twsocket.onclose = function() { TRACE('WebSocket is closed!'); setOnlineStatus(0); }\n\t}\n\tcatch (exception)\n";
	pgFile << "\t{\n\t\tsetOnlineStatus(0);\n\t\tconsole.error(\"Error initializing: \"+exception);\n\t}\n}\n\n";
	// Create a fingerprint
	pgFile << "function makeFingerprint()\n{\n";
	pgFile << "\tconsole.time('getImprint');\n";
	pgFile << "\t\timprint.test(browserTests).then(function(result) {\n";
	pgFile << "\t\t\tconsole.timeEnd('getImprint');\n\t\t\tfingerprint = result;\n";
	pgFile << "\t\t\tgetRegistrationID();\n\t});\n}\n\n";
	// This is the "main" program
	PROJECT_T prg = getProject();
	pgFile << "function main()\n{\n";
	pgFile << "\tvar elem = document.documentElement;\n\n\tif (elem.requestFullscreen)\n";
    pgFile << "\t\telem.requestFullscreen();\n";
	pgFile << "\telse if (elem.mozRequestFullScreen)\t/* Firefox */\n";
    pgFile << "\t\telem.mozRequestFullScreen();\n";
	pgFile << "\telse if (elem.webkitRequestFullscreen)\t/* Chrome, Safari and Opera */\n";
    pgFile << "\t\telem.webkitRequestFullscreen();\n\n";
	pgFile << "\twindow.addEventListener('online',  onOnline);\n";
	pgFile << "\twindow.addEventListener('offline', onlineStatus);\n";
	pgFile << "\tshowPage('"<< prg.panelSetup.powerUpPage << "');\n";

	for (size_t i = 0; i < prg.panelSetup.powerUpPopup.size(); i++)
		pgFile << "\tshowPopup('" << prg.panelSetup.powerUpPopup[i] << "');\n";

	pgFile << String("\t")+scrStart+"\n";
	pgFile << "}\n";
	pgFile << "</script>\n";
	pgFile << "</head>\n";
	// The page body
	pgFile << "<body onload=\"makeFingerprint(); main(); connect(); onlineStatus();\">" << std::endl;
	pgFile << "   <div id=\"main\"></div>" << std::endl;
/*
	for (size_t i = 0; i < stPages.size(); i++)
	{
		pgFile << stPages[i].webcode;
		writeAllPopups(pgFile);
	}
*/
	pgFile << "</body>\n</html>\n";
	pgFile.close();
	// Mark as parsed
	try
	{
		std::string nm = Configuration->getHTTProot().toString()+"/.parsed";
		pgFile.open(nm, ios::in | ios::out | ios::trunc | ios::binary);
		DateTime dt;
		pgFile << dt.toString();
		pgFile.close();
	}
	catch(const std::fstream::failure e)
	{
		sysl->errlog(std::string("TouchPanel::parsePages: I/O Error: ")+e.what());
		return false;
	}

	return true;
}

bool TouchPanel::isPresent(const std::vector<String>& vs, const String& str)
{
	for (size_t i = 0; i < vs.size(); i++)
	{
		if (str.compare(vs[i]) == 0)
			return true;
	}

	return false;
}

void TouchPanel::writeGroups (fstream& pgFile)
{
	sysl->TRACE(std::string("TouchPanel::writeGroups (fstream& pgFile)"));
	std::vector<strings::String> grName;
	pgFile << "var popupGroups = {";

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

		pgFile << "\n\t\t\"" << grName[i] << "\":[";
		bool komma = false;

		for (size_t j = 0; j < stPopups.size(); j++)
		{
			if (grName[i].compare(stPopups[j].group) == 0)
			{
				if (komma)
					pgFile << ",";

				pgFile << "\n\t\t\t\"" << stPopups[j].name << "\"";
				komma = true;
			}
		}

		pgFile << "\n\t\t]";
	}

	pgFile << "\n\t};\n\n";
}

void TouchPanel::writePages(std::fstream& pgFile)
{
	sysl->TRACE(String("TouchPanel::writePages(std::fstream& pgFile)"));
	bool first = true;
	pgFile << "var Pages = {\"pages\":[";

	for (size_t i = 0; i < stPages.size(); i++)
	{
		if (!first)
			pgFile << ",";

		pgFile << "\n\t\t{\"name\":\"" << stPages[i].name << "\",\"ID\":" << stPages[i].ID << ",\"active\":false}";
		first = false;
	}

	pgFile << "\n\t]};\n";
}

void TouchPanel::writePopups (fstream& pgFile)
{
	sysl->TRACE(std::string("TouchPanel::writePopups (fstream& pgFile)"));
	bool first = true;
	pgFile << "var Popups = {\"pages\":[";

	for (size_t i = 0; i < stPopups.size(); i++)
	{
		if (!first)
			pgFile << ",";

		pgFile << "\n\t\t{\"name\":\"" << stPopups[i].name << "\",\"ID\":" << stPopups[i].ID << ",\"group\":\"" << stPopups[i].group << "\",\"active\":false,\"lnpage\":\"\"}";
		first = false;
	}

	pgFile << "\n\t]};\n";
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

	pgFile << "var buttonArray = {" << scBtArray << "\n\t};\n";
}

void TouchPanel::writeIconTable(std::fstream& pgFile)
{
    sysl->TRACE(std::string("TouchPanel::writeIconTable(std::fstream& pgFile)"));

    Icon *ic = getIconClass();
    size_t ni = ic->numIcons();
    pgFile << "var iconArray = {\"icons\":[\n";

    for (size_t i = 0; i < ni; i++)
    {
        if (i > 0)
            pgFile << ",\n";

        pgFile << "\t\t{\"id\":" << ic->getID(i) << ",\"file\":\"" << ic->getFileName(i) << "\",";
        pgFile << "\"width\":" << ic->getWidth(i) << ",\"height\":" << ic->getHeight(i) << "}";
    }

    pgFile << "\n\t]};\n\n";
}

void TouchPanel::writeBargraphs(std::fstream& pgFile)
{
	sysl->TRACE(String("TouchPanel::writeBargraphs(std::fstream& pgFile)"));
	pgFile << "var bargraphs = {\"bargraphs\":[\n" << sBargraphs << "\n]};\n";
}

bool amx::TouchPanel::isParsed()
{
	fstream pgFile;
	// Did we've already parsed?
	try
	{
		std::string nm = Configuration->getHTTProot().toString()+"/.parsed";
		pgFile.open(nm, ios::in);

		if (pgFile.is_open())
		{
			sysl->log(Syslog::INFO, String("TouchPanel::parsePages: Pages are already parsed. Skipping!"));
			pgFile.close();
			return true;
		}

		pgFile.close();
	}
	catch(const std::fstream::failure e)
	{
		sysl->errlog(std::string("TouchPanel::parsePages: I/O Error: ")+e.what());
	}

	return false;
}
