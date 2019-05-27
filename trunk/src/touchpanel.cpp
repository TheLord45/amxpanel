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
#include <chrono>
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
//	registrated = false;
	busy = false;
	webConnected = false;
	regCallback(bind(&TouchPanel::webMsg, this, placeholders::_1, placeholders::_2));
	regCallbackStop(bind(&TouchPanel::stopClient, this));
	regCallbackConnected(bind(&TouchPanel::setWebConnect, this, placeholders::_1, placeholders::_2));
	regCallbackRegister(bind(&TouchPanel::regWebConnect, this, placeholders::_1, placeholders::_2));
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

bool TouchPanel::haveFreeSlot()
{
	sysl->TRACE(String("TouchPanel::haveFreeSlot()"));

	if (registration.size() == 0)
		return true;

	bool loop = false;
	std::vector<int>& Slots = Configuration->getAMXChannels();

	for (size_t i = 0; i < Slots.size(); i++)
	{
		loop = false;

		for (size_t j = 0; j < registration.size(); j++)
		{
			if (!registration[j].status && registration[j].channel == Slots[i])
			{
				sysl->DebugMsg(String("TouchPanel::haveFreeSlot: Reuse free slot ")+Slots[i]);
				return true;
			}
			else if (registration[j].status && registration[j].channel == Slots[i])
			{
				loop = true;
				break;
			}
		}

		if (!loop)
		{
			sysl->DebugMsg(String("TouchPanel::haveFreeSlot: Found free slot ")+Slots[0]);
			return true;
		}
	}

	sysl->DebugMsg(String("TouchPanel::haveFreeSlot: No free slot found!"));
	return false;
}

int TouchPanel::getFreeSlot()
{
	sysl->TRACE(String("TouchPanel::getFreeSlot()"));
	sysl->DebugMsg(String("TouchPanel::getFreeSlot: registration size=%1").arg(registration.size()));

	PANELS_T::iterator itr;

	for (itr = registration.begin(); itr != registration.end(); ++itr)
	{
		if (!itr->second.status && itr->first >= 10000 && itr->first < 11000)
			return itr->second.channel;
	}

	std::vector<int>& slots = Configuration->getAMXChannels();
	bool found = false;

	for (size_t i = 0; i < slots.size(); i++)
	{
		found = false;

		for (itr = registration.begin(); itr != registration.end(); ++itr)
		{
			if (itr->second.status && itr->first == slots[i])
			{
				found = true;
				break;
			}
		}

		if (!found)
			return slots[i];
	}

	return 0;
}

bool TouchPanel::replaceSlot(PANELS_T::iterator key, REGISTRATION_T& reg)
{
	sysl->TRACE(String("TouchPanel::replaceSlot(PANELS_T::iterator key, REGISTRATION_T& reg)"));

	if (registration.size() == 0 || key == registration.end())
		return false;

	int id = key->first;
	std::pair <PANELS_T::iterator, bool> ptr;

	if (id != reg.channel && id == 0 && reg.channel >= 10000 && reg.channel <= 11000)
	{
		id = reg.channel;
		registration.erase(key);
		ptr = registration.insert(PAIR(id, reg));

		if (!ptr.second)
			sysl->warnlog(String("TouchPanel::replaceSlot: Key %1 was not inserted again!").arg(id));
	}
	else
		registration[key->first] = reg;

	showContent(reg.pan);
	return true;
}

bool TouchPanel::registerSlot (int channel, String& regID, long pan)
{
	sysl->TRACE(String("TouchPanel::registerSlot (int channel, String& regID, long pan)"));
	sysl->DebugMsg(String("TouchPanel::registerSlot: Registering channel %1 with registration ID %2.").arg(channel).arg(regID));

	PANELS_T::iterator itr;

	for (itr = registration.begin(); itr != registration.end(); ++itr)
	{
		if (itr->second.pan == pan)
		{
			if (itr->first == channel && itr->second.channel == channel && itr->second.regID.compare(regID) == 0)
				return false;

			REGISTRATION_T reg = itr->second;

			if (itr->first != channel)
				reg.channel = channel;

			reg.regID = regID;
			replaceSlot(itr, reg);
			return true;
		}
	}

	REGISTRATION_T reg;

	reg.channel = channel;
	reg.regID = regID;
	reg.status = true;
	reg.pan = pan;
	reg.amxnet = 0;
	std::pair <PANELS_T::iterator, bool> ptr;
	ptr = registration.insert(PAIR(channel, reg));

	if (!ptr.second)
		sysl->warnlog(String("TouchPanel::registerSlot: Key %1 was not inserted again!").arg(channel));

	showContent(pan);
	return true;
}

bool TouchPanel::releaseSlot (int channel)
{
	sysl->TRACE(String("TouchPanel::releaseSlot (int channel)"));

	PANELS_T::iterator itr;

	for (itr = registration.begin(); itr != registration.end(); ++itr)
	{
		if (itr->first == channel)
		{
			REGISTRATION_T reg = itr->second;
			reg.status = false;
			replaceSlot(itr, reg);
			sysl->DebugMsg(String("TouchPanel::releaseSlot: Unregistered channel %1 with registration ID %2.").arg(channel).arg(reg.regID));
			return true;
		}
	}

	return false;
}

bool TouchPanel::releaseSlot (String& regID)
{
	sysl->TRACE(String("TouchPanel::releaseSlot (String& regID)"));

	PANELS_T::iterator itr;

	for (itr = registration.begin(); itr != registration.end(); ++itr)
	{
		if (itr->second.regID.compare(regID) == 0)
		{
			REGISTRATION_T reg = itr->second;
			reg.status = false;
			sysl->DebugMsg(String("TouchPanel::releaseSlot: Unregistered channel %1 with registration ID %2.").arg(itr->first).arg(regID));
			replaceSlot(itr, reg);
			return true;
		}
	}

	return false;
}

bool TouchPanel::isRegistered(String& regID)
{
	sysl->TRACE(String("TouchPanel::isRegistered(String& regID)"));

	PANELS_T::iterator itr;

	for (itr = registration.begin(); itr != registration.end(); ++itr)
	{
		if (itr->second.regID.compare(regID) == 0 && itr->second.status)
		{
			sysl->DebugMsg(String("TouchPanel::isRegistered: %1 is registered.").arg(regID));
			return true;
		}
	}

	sysl->DebugMsg(String("TouchPanel::isRegistered: %1 is NOT registered.").arg(regID));
	return false;
}

bool TouchPanel::isRegistered(int channel)
{
	sysl->TRACE(String("TouchPanel::isRegistered(int channel)"));

	PANELS_T::iterator itr;

	for (itr = registration.begin(); itr != registration.end(); ++itr)
	{
		if (itr->first == channel && itr->second.status)
		{
			sysl->DebugMsg(String("TouchPanel::isRegistered: %1 is registered.").arg(channel));
			return true;
		}
	}

	sysl->DebugMsg(String("TouchPanel::isRegistered: %1 is NOT registered.").arg(channel));
	return false;
}

AMXNet *TouchPanel::getConnection(int id)
{
	sysl->TRACE(String("TouchPanel::getConnection(int id)"));

	AMXNet *amxnet = 0;
	PANELS_T::iterator key;

	if ((key = registration.find(id)) != registration.end())
	{
		amxnet = key->second.amxnet;
	}
	
	if (amxnet == 0)
		sysl->errlog(String("TouchPanel::webMsg: Network connection not found for panel %1!").arg(id));
	
	return amxnet;
}

bool TouchPanel::delConnection(int id)
{
	sysl->TRACE(String("TouchPanel::delConnection(int id)"));

	if (id < 10000 || id > 11000)
		return false;

	PANELS_T::iterator itr;

	if ((itr = registration.find(id)) != registration.end())
	{
		if (itr->second.amxnet != 0)
			delete itr->second.amxnet;

		registration.erase(itr);
		return true;
	}

	return false;
}

void TouchPanel::regWebConnect(long pan, int id)
{
	sysl->TRACE(String("TouchPanel::regWebConnect(websocketpp::connection_hdl hdl, int id)"));

	PANELS_T::iterator itr;
	sysl->DebugMsg(String("TouchPanel::regWebConnect: Registering id %1 with pan %2").arg(id).arg(pan));

	for (itr = registration.begin(); itr != registration.end(); ++itr)
	{
		if (id >= 10000 && id <= 11000 && itr->second.pan == pan)
		{
			REGISTRATION_T reg = itr->second;
			reg.channel = id;
			reg.pan = pan;
			replaceSlot(itr, reg);
			return;
		}
		else if (id == -1 && itr->second.pan == pan)
		{
			if (itr->second.amxnet != 0)
				itr->second.amxnet->stop();
			else
				sysl->warnlog(String("TouchPanel::regWebConnect: No pointer to class AMXNet for ID %1 with pan %2!").arg(itr->first).arg(itr->second.pan));

			releaseSlot(itr->first);
			delConnection(itr->first);
			return;
		}
	}

	if (id == -1)
		return;

	// Add the data
	REGISTRATION_T reg;
	reg.channel = id;
	reg.amxnet = 0;
	reg.pan = pan;
	reg.status = false;

	if ((itr = registration.find(id)) != registration.end())
	{
		if (itr->second.amxnet != 0)
		{
			itr->second.amxnet->stop();
			delete itr->second.amxnet;
		}

		itr->second = reg;
	}
	else
	{
		std::pair <PANELS_T::iterator, bool> ptr;
		ptr = registration.insert(PAIR(id, reg));

		if (!ptr.second)
			sysl->warnlog(String("TouchPanel::regWebConnect: Key %1 was not inserted again!").arg(id));
	}

	showContent(pan);
}

bool TouchPanel::newConnection(int id)
{
	sysl->TRACE(String("TouchPanel::newConnection(int id) [id=%1]").arg(id));

	if (id < 10000 || id > 11000)
	{
		sysl->warnlog(String("TouchPanel::newConnection: Refused to register a panel with id %1").arg(id));
		return false;
	}

	try
	{
		AMXNet *pANet = new AMXNet();
		pANet->setPanelID(id);
		pANet->setCallback(bind(&TouchPanel::setCommand, this, placeholders::_1));
		pANet->setCallbackConn(bind(&TouchPanel::getWebConnect, this, placeholders::_1));

		PANELS_T::iterator key;

		if ((key = registration.find(id)) != registration.end())
		{
			REGISTRATION_T reg = key->second;
			reg.amxnet = pANet;
			replaceSlot(key, reg);
		}
		else
		{
			REGISTRATION_T reg;
			reg.channel = id;
			reg.amxnet = pANet;
			reg.status = false;
			std::pair <PANELS_T::iterator, bool> ptr;
			ptr = registration.insert(PAIR(id, reg));
			sysl->warnlog(String("TouchPanel::newConnection: Registered panel ID %1 without a registration key and with no websocket handle!").arg(id));

			if (!ptr.second)
				sysl->warnlog(String("TouchPanel::newConnection: Key %1 was not inserted again!").arg(id));
		}

		thread thr = thread([=] { pANet->Run(); });
		thr.detach();
	}
	catch (std::exception& e)
	{
		sysl->TRACE(String("TouchPanel::newConnection: Exception: ")+e.what());
		PANELS_T::iterator key;

		if ((key = registration.find(id)) != registration.end())
		{
			if (key->second.amxnet != 0)
				delete key->second.amxnet;

			registration.erase(key);
		}

		return false;
	}

	return true;
}

bool TouchPanel::getWebConnect(AMXNet* pANet)
{
	sysl->TRACE(String("TouchPanel::getWebConnect(AMXNet* pANet)"));

	PANELS_T::iterator itr;

	for (itr = registration.begin(); itr != registration.end(); ++itr)
	{
		if (itr->second.amxnet == pANet)
			return itr->second.status;
	}

	return false;
}

bool TouchPanel::send(int id, String& msg)
{
	sysl->TRACE(String("TouchPanel::send(int id, String& msg) [id=%1, msg=%2]").arg(id).arg(msg));

	PANELS_T::iterator itr;

	if ((itr = registration.find(id)) != registration.end())
		return WebSocket::send(msg, itr->second.pan);

	return false;
}

/*
 * Diese Methode wird aus der Klasse AMXNet heraus aufgerufen. Dazu wird die
 * Methode an die Klasse übergeben. Sie fungiert dann als Callback-Funktion und
 * wird immer dann aufgerufen, wenn vom Controller eine Mitteilung gekommen ist.
 */
void TouchPanel::setCommand(const ANET_COMMAND& cmd)
{
	sysl->TRACE(String("TouchPanel::setCommand(const ANET_COMMAND& cmd)"));

	if (!isRegistered(cmd.device1))
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

		if (!isRegistered(bef.device1))
			continue;

		switch (bef.MC)
		{
			case 0x0006:
			case 0x0018:	// feedback channel on
				com = String("%1:%2").arg(bef.device1).arg(bef.data.chan_state.port);
				com.append("|ON-");
				com.append(bef.data.chan_state.channel);
				send(bef.device1, com);
			break;

			case 0x0007:
			case 0x0019:	// feedback channel off
				com = String("%1:%2").arg(bef.device1).arg(bef.data.chan_state.port);
				com.append("|OFF-");
				com.append(bef.data.chan_state.channel);
				send(bef.device1, com);
			break;

			case 0x000a:	// level value change
				com = String("%1:%2").arg(bef.device1).arg(bef.data.message_value.port);
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

				send(bef.device1, com);
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

				com = String("%1:%2").arg(bef.device1).arg(msg.port);
				com.append("|");
				com.append(NameFormat::cp1250ToUTF8((char *)&msg.content));
				send(bef.device1, com);
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
void TouchPanel::webMsg(std::string& msg, long pan)
{
	sysl->TRACE(String("TouchPanel::webMsg(std::string& msg, websocketpp::connection_hdl hdl) [")+msg+"]");

	std::vector<String> parts = String(msg).split(":");
	ANET_SEND as;
	
	if (msg.find("REGISTER:") == std::string::npos)
		as.device = atoi(parts[1].data());
	else
		as.device = 0;

	if (isRegistered(as.device) && msg.find("PUSH:") != std::string::npos)
	{
		AMXNet *amxnet;

		if ((amxnet = getConnection(as.device)) == 0)
		{
			sysl->errlog(String("TouchPanel::webMsg: Network connection not found for panel %1!").arg(as.device));
			return;
		}

		as.port = atoi(parts[2].data());
		as.channel = atoi(parts[3].data());
		int value = atoi(parts[4].data());

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
	else if (isRegistered(as.device) && msg.find("LEVEL:") != std::string::npos)
	{
		AMXNet *amxnet;

		if ((amxnet = getConnection(as.device)) == 0)
		{
			sysl->errlog(String("TouchPanel::webMsg: Network connection not found for panel %1!").arg(as.device));
			return;
		}
		
		as.port = atoi(parts[2].data());
		as.channel = atoi(parts[3].data());
		as.level = as.channel;
		as.value = atoi(parts[4].data());
		as.MC = 0x008a;
		sysl->TRACE(String("TouchPanel::webMsg: port: ")+as.port+", channel: "+as.channel+", value: "+as.value+", MC: 0x"+NameFormat::toHex(as.MC, 4));

		if (amxnet != 0)
			amxnet->sendCommand(as);
		else
			sysl->warnlog(String("TouchPanel::webMsg: Class to talk with an AMX controller was not initialized!"));
	}
	else if (isRegistered(as.device) && msg.find("PING:") != std::string::npos)	// PING:<device>:<counter>:<time>
	{
		if (parts.size() >= 4)
		{
			String answer = String("%1:0|#PONG-%1,%2,%3").arg(parts[1]).arg(parts[2]).arg(parts[3]);
			send(as.device, answer);
		}
	}
	else if (msg.find("REGISTER:") != std::string::npos)
	{
		if (parts.size() != 2)
			return;

		String regID = parts[1].substring((size_t)0, parts[1].length()-1);
		sysl->warnlog(String("TouchPanel::webMsg: Try to registrate with ID: %1 ...").arg(regID));
		std::vector<String> ht = Configuration->getHashTable(Configuration->getHashTablePath());
		String ip = getIP(pan);

		if (isPresent(ht, regID) || (ip.length() > 0 && Configuration->isAllowedNet(ip)))
		{
			if (!haveFreeSlot() && !isRegistered(regID))
			{
				sysl->errlog(String("TouchPanel::webMsg: No free slots available!"));
				String com = "0|#REG-NAK";
				send(as.device, com);
				com = "0:0|#ERR-No free slots available!";
				send(as.device, com);
				return;
			}
			else if (isRegistered(regID))
				return;

			int slot = getFreeSlot();

			if (slot == 0)
			{
				sysl->errlog(String("TouchPanel::webMsg: No more free slots available!"));
				String s = "0:0|#REG-NAK";
				send(as.device, s);
				s = "0:0|#ERR-No more free slots!";
				send(as.device, s);
				return;
			}

			registerSlot(slot, regID, pan);

			if (!newConnection(slot))
			{
				sysl->errlog(std::string("TouchPanel::webMsg: Error connecting to controller!"));
				String s = "0:0|#REG-NAK";
				send(as.device, s);
				s = "0:0|#ERR-Connection error!";
				send(as.device, s);
				return;
			}

			sysl->warnlog(String("TouchPanel::webMsg: Registration with ID: %1 was successfull.").arg(regID));
			String com = String("0:0|#REG-OK,%1,%2").arg(slot).arg(regID);
			send(slot, com);
			return;
		}
		else
		{
			sysl->warnlog(String("TouchPanel::webMsg: Access for ID: %1 is denied!").arg(regID));
			String com = "0:0|#REG-NAK";
			send(as.device, com);
			com = "0:0|#ERR-Access denied!";
			send(as.device, com);
		}
	}
}

void TouchPanel::stopClient()
{
	sysl->TRACE(String("TouchPanel::stopClient()"));

	PANELS_T::iterator itr;

	for (itr = registration.begin(); itr != registration.end(); ++itr)
	{
		itr->second.amxnet->stop();
		delete itr->second.amxnet;
		registration.erase(itr);
	}
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

		jsFile << "{" << std::endl << "\t\"short_name\": \"AMXPanel\"," << std::endl;
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

	getFontList()->serializeToJson();

	// Service worker code
	try
	{
		std::string cacheName = Configuration->getHTTProot().toString()+"/scripts/sw.js";
		cacheFile.open(cacheName, ios::in | ios::out | ios::trunc | ios::binary);

		if (!cacheFile.is_open())
		{
			sysl->errlog(std::string("TouchPanel::parsePages: Error opening file ")+cacheName);
			return false;
		}

		cacheFile << std::endl << "// This is the Service Worker needed to run as a stand allone app." << std::endl;
		cacheFile << "if('serviceWorker' in navigator)\n{" << std::endl;
		cacheFile << "\twindow.addEventListener('load', function() {" << std::endl;
		cacheFile << "\t\tnavigator.serviceWorker.register('" << Configuration->getWebLocation() << "/scripts/sw.js').then(function(registration) {" << std::endl;
		cacheFile << "\t\t\tdebug(\"Service Worker registration successful width scope: \"+registration.scope);" << std::endl;
		cacheFile << "\t\t}, function(err) {\n\t\t\terrlog(\"Registration failed:\"+err);" << std::endl;
		cacheFile << "\t\t})\n\t})\n}" << std::endl << std::endl;
		cacheFile << "var cache_name = 'amxpanel-" << VERSION << "'" << std::endl << std::endl;
		cacheFile << "var urls_to_cache = [" << std::endl;
		cacheFile << "\t'" << Configuration->getWebLocation() << "'," << std::endl;
		cacheFile << "\t'" << Configuration->getWebLocation() << "/scripts/'," << std::endl;
		cacheFile << "\t'" << Configuration->getWebLocation() << "/images/'" << std::endl;
		cacheFile << "]" << std::endl << std::endl;
		cacheFile << "self.addEventListener('install', function(e) {" << std::endl;
		cacheFile << "\te.waitUntil(caches.open(cache_name).then(function(cache) {" << std::endl;
		cacheFile << "\t\treturn cache.addAll(urls_to_cache)\n\t}) )\n})" << std::endl << std::endl;
		cacheFile << "self.addEventListener('fetch', function(e) {" << std::endl;
		cacheFile << "\te.respondWith(caches.match(e.request).then(function(response) {" << std::endl;
		cacheFile << "\t\tif(response)\n\t\t\treturn response\n\t\telse\n\t\t\treturn fetch(e.request)" << std::endl;
		cacheFile << "\t}) )\n})" << std::endl << std::endl;
		cacheFile.close();
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
	pgFile << "<meta name=\"viewport\" content=\"width=device-width, height=device-height, initial-scale=0.7, minimum-scale=0.7, maximum-scale=1.0, user-scalable=yes\"/>\n";
	pgFile << "<meta name=\"mobile-web-app-capable\" content=\"yes\" />" << std::endl;
	pgFile << "<meta name=\"apple-mobile-web-app-capable\" content=\"yes\" />\n";
	pgFile << "<meta name=\"apple-mobile-web-app-status-bar-style\" content=\"black\" />" << std::endl;
	pgFile << "<link rel=\"manifest\" href=\"manifest.json\">" << std::endl;
	pgFile << "<link rel=\"icon\" sizes=\"256x256\" href=\"images/icon.png\">" << std::endl;
	pgFile << "<link rel=\"stylesheet\" type=\"text/css\" href=\"amxpanel.css\">" << std::endl;
	pgFile << "<link rel=\"stylesheet\" type=\"text/css\" href=\"scripts/keyboard/css/index.css\">" << std::endl;
	// Scripts
	pgFile << "<script type=\"text/javascript\" src=\"scripts/sw.js\"></script>" << std::endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/imprint.min.js\"></script>" << std::endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/store.modern.min.js\"></script>" << std::endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/keyboard/index.js\"></script>" << std::endl;
	pgFile << "<script>\n";
	pgFile << "\"use strict\";\n";
	pgFile << "var fingerprint = \"\";\n";
	pgFile << "var pageName = \"\";\n";
	pgFile << "var wsocket = null;\n";
	pgFile << "var ws_online = 0;		// 0 = offline, 1 = online, 2 = connecting" << std::endl;
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
		return false;
	}

	try
	{
		std::string jsname = Configuration->getHTTProot().toString()+"/scripts/resource.js";
		jsFile.open(jsname, ios::in | ios::out | ios::trunc | ios::binary);

		if (!jsFile.is_open())
		{
			sysl->errlog(std::string("TouchPanel::parsePages: Error opening file ")+jsname);
			pgFile.close();
			return false;
		}

		PROJECT_T prj = getProject();
		std::vector<RESOURCE_LIST_T>& resList = prj.resourceLists;
		jsFile << "var ressources = { \"ressources\":[" << std::endl;

		for (size_t i = 0; i < resList.size(); i++)
		{
			if (i > 0)
				jsFile << "," << std::endl;

			std::vector<RESOURCE_T>& res = resList[i].ressource;
			jsFile << "\t{\"type\":\"" << resList[i].type << "\",\"ressource\":[" << std::endl;

			for (size_t j = 0; j < res.size(); j++)
			{
				if (j > 0)
					jsFile << "," << std::endl;

				jsFile << "\t\t{\"name\":\"" << res[j].name << "\",\"protocol\":\"" << res[j].protocol << "\"," << std::endl;
				jsFile << "\t\t \"user\":\"" << res[j].user << "\",\"password\":\"" << res[j].password << "\"," << std::endl;
				jsFile << "\t\t \"encrypted\":" << ((res[j].encrypted)?"true":"false") << ",\"host\":\"" << res[j].host << "\"," << std::endl;
				jsFile << "\t\t \"path\":\"" << res[j].path << "\",\"file\":\"" << res[j].file << "\",\"refresh\":" << res[j].refresh << "}";
			}

			jsFile << std::endl << "\t]}";
		}

		jsFile << std::endl << "]};" << std::endl;
		jsFile.close();
	}
	catch (const std::fstream::failure e)
	{
		sysl->errlog(std::string("TouchPanel::parsePages: I/O Error: ")+e.what());
		pgFile.close();
		return false;
	}

	pgFile << "</script>" << std::endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/browser.js\"></script>" << std::endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/pages.js\"></script>" << std::endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/popups.js\"></script>" << std::endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/groups.js\"></script>" << std::endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/btarray.js\"></script>" << std::endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/icons.js\"></script>" << std::endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/bargraphs.js\"></script>" << std::endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/palette.js\"></script>" << std::endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/fonts.js\"></script>" << std::endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/chameleon.js\"></script>" << std::endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/resource.js\"></script>" << std::endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/movie.js\"></script>" << std::endl << std::endl;

	for (size_t i = 0; i < stPages.size(); i++)
	{
		pgFile << "<script type=\"text/javascript\" src=\"scripts/Page" << stPages[i].ID << ".js\"></script>" << std::endl;
	}

	pgFile << std::endl;

	for (size_t i = 0; i < stPopups.size(); i++)
	{
		pgFile << "<script type=\"text/javascript\" src=\"scripts/Page" << stPopups[i].ID << ".js\"></script>" << std::endl;
	}

	pgFile << std::endl << "<script type=\"text/javascript\" src=\"scripts/page.js\"></script>" << std::endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/amxpanel.js\"></script>" << std::endl;
	// Add some special script functions
	pgFile << "<script>\n";
	pgFile << scrBuffer << std::endl;
	// This is the WebSocket connection function
	pgFile << "function connect()\n{\n";
	pgFile << "\tif (wsocket !== null && (wsocket.readyState == WebSocket.OPEN || wsocket.readyState == WebSocket.CLOSING) && ws_online > 0)" << std::endl;
	pgFile << "\t\treturn;" << std::endl << std::endl;
	pgFile << "\ttry\n\t{\n\t\tws_online = 2;" << std::endl;

	if (Configuration->getWSStatus())
		pgFile << "\t\twsocket = new WebSocket(\"wss://" << Configuration->getWebSocketServer() << ":" << Configuration->getSidePort() << "/\");\n";
	else
		pgFile << "\t\twsocket = new WebSocket(\"ws://" << Configuration->getWebSocketServer() << ":" << Configuration->getSidePort() << "/\");\n";

	pgFile << "\t\twsocket.onopen = function() {" << std::endl;
    pgFile << "\t\t\tgetRegistrationID();\n\t\t\tws_online = 1;\t\t// online\n\t\t\tsetOnlineStatus(1);\n" << std::endl;
	pgFile << "\t\t\tif (!regStatus)\n\t\t\t{" << std::endl;
	pgFile << "\t\t\t\tif (typeof registrationID == \"string\" && registrationID.length > 0)\n";
	pgFile << "\t\t\t\t\twsocket.send('REGISTER:'+registrationID+';');\n";
	pgFile << "\t\t\t\telse\n\t\t\t\t\terrlog(\"connect: Missing registration ID!\");\n\t\t\t}\n\t\t}" << std::endl;
	pgFile << "\t\twsocket.onerror = function(error) { errlog('WebSocket error: '+error); setOnlineStatus(9); }\n";
	pgFile << "\t\twsocket.onmessage = function(e) { parseMessage(e.data); }\n";
	pgFile << "\t\twsocket.onclose = function() {\n\t\t\tTRACE('WebSocket is closed!');" << std::endl;
	pgFile << "\t\t\tws_online = 0;\t\t// offline\n\t\t\tregStatus = false;\n\t\t\tsetOnlineStatus(0);\n\t\t}\n\t}\n\tcatch (exception)\n";
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
	pgFile << "\tif (isIOS() || (isFirefox() && isAndroid()))\n\t{" << std::endl;
	pgFile << "\t\tEVENT_DOWN = \"touchstart\";\n\t\tEVENT_UP = \"touchend\";\n\t\tEVENT_MOVE = \"touchmove\";" << std::endl;
	pgFile << "\t\tTRACE(\"main: Events were set to TOUCH...\");\n\t}" << std::endl;
	pgFile << "\telse if (isFirefox() || isSafari() || isMacOS())" << std::endl;
	pgFile << "\t{\n\t\tEVENT_DOWN = \"mousedown\";\n\t\tEVENT_UP = \"mouseup\";\n\t\tEVENT_MOVE = \"mousemove\";" << std::endl;
	pgFile << "\t\tTRACE(\"main: Events were set to MOUSE...\");\n\t}\n" << std::endl;
	pgFile << "\thandleStandby();" << std::endl;
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
	sysl->TRACE(String("TouchPanel::isPresent(const std::vector<String>& vs, const String& str)"));

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

uint64_t TouchPanel::getMS()
{
	return std::chrono::duration_cast< std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch()).count();
}

void TouchPanel::setWebConnect(bool s, long pan)
{
	sysl->TRACE(String("TouchPanel::setWebConnect(bool s, websocketpp::connection_hdl hdl)"));

	PANELS_T::iterator itr;

	for (itr = registration.begin(); itr != registration.end(); ++itr)
	{
		if (itr->second.pan == pan)
		{
			REGISTRATION_T reg = itr->second;
			reg.status = s;
			replaceSlot(itr, reg);
			sysl->DebugMsg(String("TouchPanel::setWebConnect: Status set to %1").arg((s)?"TRUE":"FALSE"));
			break;
		}
	}
}

void TouchPanel::showContent(long pan)
{
	sysl->TRACE(String("TouchPanel::showContent(long pan)"));

	bool found = false;
	PANELS_T::iterator itr;
	sysl->log_serial(Syslog::IDEBUG, String("  DBG"));
	sysl->log_serial(Syslog::IDEBUG, String("  DBG     \"size\" : ")+registration.size());

	for (itr = registration.begin(); itr != registration.end(); ++itr)
	{
		if (itr->second.pan == pan)
		{
			sysl->log_serial(Syslog::IDEBUG, String("  DBG     \"first\": %1").arg(itr->first));
			sysl->log_serial(Syslog::IDEBUG, String("  DBG     channel: %1").arg(itr->second.channel));
			sysl->log_serial(Syslog::IDEBUG, String("  DBG     pan    : %1").arg(itr->second.pan));
			sysl->log_serial(Syslog::IDEBUG, String("  DBG     regID  : %1").arg(itr->second.regID));
			sysl->log_serial(Syslog::IDEBUG, String("  DBG     status : %1").arg(itr->second.status));
			sysl->log_serial(Syslog::IDEBUG, String("  DBG     *amxnet: %1").arg((itr->second.amxnet == 0)?"NULL":"<pointer>"));
			sysl->DebugMsg(String("  DBG"));
			found = true;
		}
	}

	if (!found)
		sysl->DebugMsg(std::string("TouchPanel::showContent: Content not found!"));
}
