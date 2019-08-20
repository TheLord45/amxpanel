/*
 * Copyright (C) 2018, 2019 by Andreas Theofilu <andreas@theosys.at>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
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
#include "trace.h"
#include "str.h"

#ifdef __APPLE__
using namespace boost;
#endif

using namespace amx;
using namespace std;

extern Config *Configuration;
extern Syslog *sysl;

TouchPanel::TouchPanel()
		: mut(),
		  cond()
{
	sysl->TRACE(Syslog::ENTRY, "TouchPanel::TouchPanel()");
	busy = false;
	serNum = 74201;
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
	catch (exception &e)
	{
		sysl->errlog(string("TouchPanel::TouchPanel: Error creating a thread: ")+e.what());
	}
}

TouchPanel::~TouchPanel()
{
	sysl->TRACE(Syslog::EXIT, "TouchPanel::TouchPanel()");
}

bool TouchPanel::haveFreeSlot()
{
	DECL_TRACER("TouchPanel::haveFreeSlot()");

	vector<int>& Slots = Configuration->getAMXChannels();
	bool loop = false;

	if (registration.size() == 0 && Slots.size() > 0)
		return true;

	for (size_t i = 0; i < Slots.size(); i++)
	{
		loop = false;

		for (size_t j = 0; j < registration.size(); j++)
		{
			if (!registration[j].status && registration[j].channel == Slots[i])
				return true;
			else if (registration[j].status && registration[j].channel == Slots[i])
			{
				loop = true;
				break;
			}
		}

		if (!loop)
			return true;
	}

	sysl->warnlog("TouchPanel::haveFreeSlot: No free slot found!");
	return false;
}

int TouchPanel::getFreeSlot()
{
	DECL_TRACER("TouchPanel::getFreeSlot()");

	bool loop = false;
	vector<int>& Slots = Configuration->getAMXChannels();

	for (size_t i = 0; i < Slots.size(); i++)
	{
		loop = false;

		for (size_t j = 0; j < registration.size(); j++)
		{
			if (!registration[j].status && registration[j].channel == Slots[i])
				return Slots[i];
			else if (registration[j].status && registration[j].channel == Slots[i])
			{
				loop = true;
				break;
			}
		}

		if (!loop)
			return Slots[i];
	}

	sysl->warnlog("TouchPanel::getFreeSlot: No free slot found!");
	return 0;
}

bool TouchPanel::replaceSlot(PANELS_T::iterator key, REGISTRATION_T& reg)
{
	DECL_TRACER("TouchPanel::replaceSlot(PANELS_T::iterator key, REGISTRATION_T& reg)");

	if (registration.size() == 0 || key == registration.end())
		return false;

	int id = key->first;
	pair <PANELS_T::iterator, bool> ptr;

	if (id != reg.channel && id == 0 && reg.channel >= 10000 && reg.channel <= 11000)
	{
		id = reg.channel;
		registration.erase(key);
		ptr = registration.insert(PAIR(id, reg));

		if (!ptr.second)
			sysl->warnlog(string("TouchPanel::replaceSlot: Key ")+to_string(id)+" was not inserted again!");
	}
	else
		registration[key->first] = reg;

	showContent(reg.pan);
	return true;
}

bool TouchPanel::registerSlot (int channel, string& regID, long pan)
{
	DECL_TRACER("TouchPanel::registerSlot (int channel, string& regID, long pan)");

	PANELS_T::iterator itr = registration.begin();
	size_t i = 0;

	while (i < registration.size())
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
			sysl->DebugMsg("TouchPanel::registerSlot: Registered channel "+to_string(channel)+" with registration ID "+regID+".");
			return true;
		}

		i++;
		++itr;
	}

	REGISTRATION_T reg;

	reg.channel = channel;
	reg.regID = regID;
	reg.status = true;
	reg.pan = pan;
	reg.amxnet = 0;
	pair <PANELS_T::iterator, bool> ptr;
	ptr = registration.insert(PAIR(channel, reg));

	if (!ptr.second)
		sysl->warnlog("TouchPanel::registerSlot: Key "+to_string(channel)+" was not inserted again!");
	else
		sysl->DebugMsg("TouchPanel::registerSlot: Registering channel "+to_string(channel)+" with registration ID "+regID+".");

	showContent(pan);
	return true;
}

bool TouchPanel::releaseSlot (int channel)
{
	DECL_TRACER(string("TouchPanel::releaseSlot (int channel)"));

	PANELS_T::iterator itr;

	if ((itr = registration.find(channel)) != registration.end())
	{
		itr->second.status = false;
		sysl->DebugMsg(string("TouchPanel::registerSlot: Unregistered channel ")+to_string(channel)+" with registration ID "+itr->second.regID+".");
		return true;
	}

	return false;
}

bool TouchPanel::releaseSlot (const string& regID)
{
	DECL_TRACER("TouchPanel::releaseSlot (string& regID)");

	PANELS_T::iterator itr;
	size_t i = 0;

	for (itr = registration.begin(); itr != registration.end(); ++itr)
	{
		if (itr->second.regID.compare(regID) == 0)
		{
			itr->second.status = false;
			sysl->DebugMsg("TouchPanel::releaseSlot: Unregistered channel "+to_string(itr->first)+" with registration ID "+regID+".");
			return true;
		}

		i++;

		if (i >= registration.size())
			break;
	}

	return false;
}

bool TouchPanel::isRegistered(const string& regID)
{
	DECL_TRACER("TouchPanel::isRegistered(string& regID)");

	for (size_t i = 0; i < registration.size(); i++)
	{
		if (registration[i].regID.compare(regID) == 0 && registration[i].status)
			return true;
	}

	return false;
}

bool TouchPanel::isRegistered(int channel)
{
	DECL_TRACER("TouchPanel::isRegistered(int channel)");

	if (registration.size() == 0)
		return false;

	PANELS_T::iterator itr;

	for (itr = registration.begin(); itr != registration.end(); ++itr)
	{
		if (itr->first == channel && itr->second.status)
			return true;
	}

	return false;
}

AMXNet *TouchPanel::getConnection(int id)
{
	DECL_TRACER("TouchPanel::getConnection(int id)");

	AMXNet *amxnet = 0;
	PANELS_T::iterator key;

	if ((key = registration.find(id)) != registration.end())
		amxnet = key->second.amxnet;

	if (amxnet == 0)
		sysl->errlog("TouchPanel::webMsg: Network connection not found for panel "+to_string(id)+"!");

	return amxnet;
}

string& TouchPanel::getAMXBuffer(int id)
{
	DECL_TRACER("TouchPanel::getAMXBuffer(int id)");

	PANELS_T::iterator key;

	if ((key = registration.find(id)) != registration.end())
		return key->second.amxBuffer;

	return none;
}

void TouchPanel::setAMXBuffer(int id, const string& buf)
{
	DECL_TRACER("TouchPanel::setAMXBuffer(int id, const string& buf)");

	PANELS_T::iterator key;

	if ((key = registration.find(id)) != registration.end())
		key->second.amxBuffer = buf;
}

bool TouchPanel::delConnection(int id)
{
	DECL_TRACER("TouchPanel::delConnection(int id)");

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

/*
 * This method is called from WebSocket::tcp_post_init(). It is registered as
 * a callback function and will be called whenever a browser makes a new
 * contact over WEB socket.
 */
void TouchPanel::regWebConnect(long pan, int id)
{
	std::lock_guard<std::mutex> lock(mut);
	DECL_TRACER("TouchPanel::regWebConnect(websocketpp::connection_hdl hdl, int id)");

	PANELS_T::iterator itr;
	sysl->DebugMsg("TouchPanel::regWebConnect: Registering id "+to_string(id)+" with pan "+to_string(pan));

	if (id != 0)
	{
		for (itr = registration.begin(); itr != registration.end(); ++itr)
		{
			if (id >= 10000 && id <= 11000 && itr->second.pan == pan)
			{
				itr->second.channel = id;
				itr->second.pan = pan;
				showContent(pan);
				return;
			}
			else if (id == -1 && itr->second.pan == pan)
			{
				if (itr->second.amxnet != 0)
					itr->second.amxnet->stop();
				else
					sysl->warnlog("TouchPanel::regWebConnect: No pointer to class AMXNet for ID "+to_string(itr->first)+" with pan "+to_string(itr->second.pan)+"!");

				releaseSlot(itr->first);
				delConnection(itr->first);
				cond.notify_one();
				return;
			}
		}
	}

	if (id == -1)
	{
		cond.notify_one();
		return;
	}

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
		pair <PANELS_T::iterator, bool> ptr;
		ptr = registration.insert(PAIR(id, reg));

		if (!ptr.second)
			sysl->warnlog("TouchPanel::regWebConnect: Key "+to_string(id)+" was not inserted again!");
	}

	showContent(pan);
	cond.notify_one();
}

bool TouchPanel::newConnection(int id)
{
	DECL_TRACER("TouchPanel::newConnection(int id) [id="+to_string(id)+"]");

	if (id < 10000 || id > 11000)
	{
		sysl->warnlog("TouchPanel::newConnection: Refused to register a panel with id "+to_string(id));
		return false;
	}

	try
	{
		AMXNet *pANet = new AMXNet(getSerialNum());
		pANet->setPanelID(id);
		pANet->setCallback(bind(&TouchPanel::setCommand, this, placeholders::_1));

		PANELS_T::iterator key;

		if ((key = registration.find(id)) != registration.end())
		{
			if (key->second.amxnet == 0)
				key->second.amxnet = pANet;
			else
				delete pANet;
		}
		else
		{
			REGISTRATION_T reg;
			reg.channel = id;
			reg.amxnet = pANet;
			reg.status = false;
			pair <PANELS_T::iterator, bool> ptr;
			ptr = registration.insert(PAIR(id, reg));
			sysl->warnlog("TouchPanel::newConnection: Registered panel ID "+to_string(id)+" without a registration key and with no websocket handle!");

			if (!ptr.second)
				sysl->warnlog("TouchPanel::newConnection: Key "+to_string(id)+" was not inserted again!");
		}

		thread thr = thread([=] { pANet->Run(); });
		thr.detach();
	}
	catch (exception& e)
	{
		sysl->TRACE(string("TouchPanel::newConnection: Exception: ")+e.what());
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

bool TouchPanel::send(int id, string& msg)
{
	DECL_TRACER(string("TouchPanel::send(int id, string& msg) [id=")+to_string(id)+", msg="+msg+"]");

	PANELS_T::iterator itr;

	if ((itr = registration.find(id)) != registration.end())
	{
		string m(msg);
		return WebSocket::send(m, itr->second.pan);
	}

	return false;
}

/*
 * Diese Methode wird aus der Klasse AMXNet heraus aufgerufen. Dazu wird die
 * Methode an die Klasse Ã¼bergeben. Sie fungiert dann als Callback-Funktion und
 * wird immer dann aufgerufen, wenn vom Controller eine Mitteilung gekommen ist.
 */
void TouchPanel::setCommand(const ANET_COMMAND& cmd)
{
	std::lock_guard<std::mutex> lock(mut);
	DECL_TRACER("TouchPanel::setCommand(const ANET_COMMAND& cmd)");

	if (!isRegistered(cmd.device1))
		return;

	commands.push_back(cmd);

	if (busy)
		return;

	busy = true;
	string com;

	while (commands.size() > 0)
	{
		ANET_COMMAND& bef = commands.at(0);
		commands.erase(commands.begin());
		string amxBuffer = getAMXBuffer(bef.device1);

		switch (bef.MC)
		{
			case 0x0006:
			case 0x0018:	// feedback channel on
				com.assign(to_string(bef.device1));
				com.append(":");
				com.append(to_string(bef.data.chan_state.port));
				com.append("|ON-");
				com.append(to_string(bef.data.chan_state.channel));
				send(bef.device1, com);
			break;

			case 0x0007:
			case 0x0019:	// feedback channel off
				com.assign(to_string(bef.device1));
				com.append(":");
				com.append(to_string(bef.data.chan_state.port));
				com.append("|OFF-");
				com.append(to_string(bef.data.chan_state.channel));
				send(bef.device1, com);
			break;

			case 0x000a:	// level value change
				com.assign(to_string(bef.device1));
				com.append(":");
				com.append(to_string(bef.data.message_value.port));
				com += "|LEVEL-";
				com += to_string(bef.data.message_value.value);
				com += ",";

				switch (bef.data.message_value.type)
				{
					case 0x10: com += to_string(bef.data.message_value.content.byte); break;
					case 0x11: com += to_string(bef.data.message_value.content.ch); break;
					case 0x20: com += to_string(bef.data.message_value.content.integer); break;
					case 0x21: com += to_string(bef.data.message_value.content.sinteger); break;
					case 0x40: com += to_string(bef.data.message_value.content.dword); break;
					case 0x41: com += to_string(bef.data.message_value.content.sdword); break;
					case 0x4f: com += to_string(bef.data.message_value.content.fvalue); break;
					case 0x8f: com += to_string(bef.data.message_value.content.dvalue); break;
				}

				send(bef.device1, com);
			break;

			case 0x000c:	// Command string
				ANET_MSG_STRING msg = bef.data.message_string;

				if (msg.length < strlen((char *)&msg.content))
				{
					amxBuffer.append((char *)&msg.content);
					setAMXBuffer(bef.device1, amxBuffer);
					break;
				}
				else if (amxBuffer.length() > 0)
				{
					amxBuffer.append((char *)&msg.content);
					setAMXBuffer(bef.device1, amxBuffer);
					size_t len = (amxBuffer.length() >= sizeof(msg.content)) ? 1499 : amxBuffer.length();
					strncpy((char *)&msg.content, amxBuffer.c_str(), len);
					msg.content[len] = 0;
				}

				com.assign(to_string(bef.device1));
				com.append(":");
				com.append(to_string(msg.port));
				com.append("|");
				com.append(NameFormat::cp1250ToUTF8((char *)&msg.content));
				send(bef.device1, com);
			break;
		}
	}

	busy = false;
	cond.notify_one();
}

/*
 * This function is called from class WebSocket every time a message from the
 * client web browser is received. The messages are processed and then the
 * result is send to the controller, if there is something to send.
 */
void TouchPanel::webMsg(string& msg, long pan)
{
	std::lock_guard<std::mutex> lock(mut);
	DECL_TRACER("TouchPanel::webMsg(string& msg, websocketpp::connection_hdl hdl) ["+msg+"]");

	vector<string> parts = Str::split(msg, ":");
	ANET_SEND as;

	if (msg.find("REGISTER:") == string::npos)
		as.device = atoi(parts[1].c_str());
	else
		as.device = 0;

	if (msg.find("REGISTER:") != string::npos)
	{
		if (parts.size() != 2)
		{
			cond.notify_one();
			return;
		}

		string regID = parts[1].substr(0, parts[1].length()-1);
		sysl->warnlog("TouchPanel::webMsg: Try to registrate with ID: "+regID+" ...");
		vector<string> ht;

		try
		{
			ht = Configuration->getHashTable(Configuration->getHashTablePath());
		}
		catch (exception& e)
		{
			sysl->warnlog("TouchPanel::webMsg: No hashtable found!");
			ht.clear();
		}

		string ip = getIP(pan);

		if ((ip.length() > 0 && Configuration->isAllowedNet(ip)) || isPresent(ht, regID))
		{
			if (!haveFreeSlot() && !isRegistered(regID))
			{
				sysl->errlog("TouchPanel::webMsg: No free slots available!");
				string com = "0|#REG-NAK";
				send(as.device, com);
				com = "0:0|#ERR-No free slots available!";
				send(as.device, com);
				cond.notify_one();
				return;
			}
			else if (isRegistered(regID))
			{
				sysl->warnlog("TouchPanel::webMsg: Panel with registration ID "+regID+" is already registrated!");
				cond.notify_one();
				return;
			}

			int slot = getFreeSlot();

			if (slot == 0)
			{
				sysl->errlog("TouchPanel::webMsg: No more free slots available!");
				string s = "0:0|#REG-NAK";
				send(as.device, s);
				s = "0:0|#ERR-No more free slots!";
				send(as.device, s);
				cond.notify_one();
				return;
			}

			registerSlot(slot, regID, pan);

			if (!newConnection(slot))
			{
				sysl->errlog(string("TouchPanel::webMsg: Error connecting to controller!"));
				string s = "0:0|#REG-NAK";
				send(as.device, s);
				s = "0:0|#ERR-Connection error!";
				send(as.device, s);
				cond.notify_one();
				return;
			}

			sysl->warnlog("TouchPanel::webMsg: Registration with ID: "+regID+" was successfull.");
			string com = "0:0|#REG-OK,"+to_string(slot)+","+regID;
			send(slot, com);
			cond.notify_one();
			return;
		}
		else
		{
			sysl->warnlog("TouchPanel::webMsg: Access for ID: "+regID+" is denied!");
			string com = "0:0|#REG-NAK";
			send(as.device, com);
			com = "0:0|#ERR-Access denied!";
			send(as.device, com);
			cond.notify_one();
		}
	}
	else if (isRegistered(as.device) && msg.find("PUSH:") != string::npos)
	{
		AMXNet *amxnet;

		if ((amxnet = getConnection(as.device)) == 0)
		{
			sysl->errlog(string("TouchPanel::webMsg: Network connection not found for panel ")+to_string(as.device)+"!");
			cond.notify_one();
			return;
		}

		as.port = atoi(parts[2].c_str());
		as.channel = atoi(parts[3].c_str());
		int value = atoi(parts[4].c_str());

		if (value)
			as.MC = 0x0084;
		else
			as.MC = 0x0085;

		sysl->TRACE(string("TouchPanel::webMsg: port: ")+to_string(as.port)+", channel: "+to_string(as.channel)+", value: "+to_string(value)+", MC: 0x"+NameFormat::toHex(as.MC, 4));

		if (amxnet != 0)
			amxnet->sendCommand(as);
		else
			sysl->warnlog(string("TouchPanel::webMsg: Class to talk with an AMX controller was not initialized!"));
	}
	else if (isRegistered(as.device) && msg.find("LEVEL:") != string::npos)
	{
		AMXNet *amxnet;

		if ((amxnet = getConnection(as.device)) == 0)
		{
			sysl->errlog(string("TouchPanel::webMsg: Network connection not found for panel ")+to_string(as.device)+"!");
			cond.notify_one();
			return;
		}

		as.port = atoi(parts[2].c_str());
		as.channel = atoi(parts[3].c_str());
		as.level = as.channel;
		as.value = atoi(parts[4].c_str());
		as.MC = 0x008a;
		sysl->TRACE(string("TouchPanel::webMsg: port: ")+to_string(as.port)+", channel: "+to_string(as.channel)+", value: "+to_string(as.value)+", MC: 0x"+NameFormat::toHex(as.MC, 4));

		if (amxnet != 0)
			amxnet->sendCommand(as);
		else
			sysl->warnlog(string("TouchPanel::webMsg: Class to talk with an AMX controller was not initialized!"));
	}
	else if (isRegistered(as.device) && msg.find("PING:") != string::npos)	// PING:<device>:<counter>:<time>
	{
		if (parts.size() >= 4)
		{
			string answer = parts[1]+":0|#PONG-"+parts[1]+","+parts[2]+","+parts[3];
			send(as.device, answer);
		}
	}
	else if (isRegistered(as.device) &&
			 (msg.find("KEY:") != string::npos ||		// KEY:<panelID>:<port>:<channel>:<string>;
			  msg.find("STRING:") != string::npos))	// STRING:<panelID>:<port>:<channel>:<string>;
	{
		AMXNet *amxnet;

		if ((amxnet = getConnection(as.device)) == 0)
		{
			sysl->errlog("TouchPanel::webMsg: Network connection not found for panel "+to_string(as.device)+"!");
			cond.notify_one();
			return;
		}

		as.port = atoi(parts[2].c_str());
		as.channel = atoi(parts[3].c_str());
		size_t i = 4;

		while (i < parts.size())
		{
			if (i > 4)
				as.msg.append(":");

			as.msg.append(parts[i]);
			i++;
		}

		size_t pos = as.msg.find_last_of(';');

		if (pos != string::npos)
			as.msg = as.msg.substr(0, pos);

		if (msg.find("KEY:") != string::npos)
			as.msg = NameFormat::UTF8ToCp1250(as.msg);

		as.MC = 0x008b;
		sysl->TRACE("TouchPanel::webMsg: port: "+to_string(as.port)+", channel: "+to_string(as.channel)+", msg: "+as.msg+", MC: 0x"+NameFormat::toHex(as.MC, 4));

		if (amxnet != 0)
			amxnet->sendCommand(as);
		else
			sysl->warnlog("TouchPanel::webMsg: Class to talk with an AMX controller was not initialized!");
	}

	cond.notify_one();
}

void TouchPanel::stopClient()
{
	std::lock_guard<std::mutex> lock(mut);
	DECL_TRACER("TouchPanel::stopClient()");

	PANELS_T::iterator itr;

	while (registration.size() > 0)
	{
		itr = registration.begin();
		REGISTRATION_T reg = itr->second;

		if (reg.amxnet != 0)
		{
			reg.amxnet->stop();
			delete reg.amxnet;
		}

		registration.erase(itr);
	}

	cond.notify_one();
}

int TouchPanel::findPage(const string& name)
{
	DECL_TRACER("TouchPanel::findPage(const string& name)");
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
	DECL_TRACER("TouchPanel::readPages()");

	if (gotPages || isParsed())
		return;

	try
	{
		vector<string> pgs = getPageFileNames();
		scBtArray = "\"buttons\":[";
		bool first = false;

		for (size_t i = 0; i < pgs.size(); i++)
		{
			sysl->TRACE("TouchPanel::readPages: Parsing page "+pgs[i]);
			Page p(pgs[i]);
			p.setPalette(getPalettes());
			p.setParentSize(getProject().panelSetup.screenWidth, getProject().panelSetup.screenHeight);
			p.setFontClass(getFontList());
			p.setProject(&getProject());
			p.setIconClass(getIconClass());

			if (!p.parsePage())
			{
				sysl->warnlog("TouchPanel::readPages: Page "+p.getPageName()+" had an error! Page will be ignored.");
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
				pop.modal = p.getModal();
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
	catch (exception& e)
	{
		sysl->errlog(string("TouchPanel::readPages: ")+e.what());
		exit(1);
	}
}

bool TouchPanel::parsePages()
{
	DECL_TRACER(string("TouchPanel::parsePages()"));

	fstream pgFile, cssFile, jsFile, cacheFile;
	// Did we've already parsed?
	if (isParsed())
		return true;

	string fname = Configuration->getHTTProot()+"/index.html";
	string cssname = Configuration->getHTTProot()+"/amxpanel.css";

	try
	{
		pgFile.open(fname, ios::in | ios::out | ios::trunc | ios::binary);

		if (!pgFile.is_open())
		{
			sysl->errlog(string("TouchPanel::parsePages: Error opening file ")+fname);
			return false;
		}

		cssFile.open(cssname, ios::in | ios::out | ios::trunc | ios::binary);

		if (!cssFile.is_open())
		{
			sysl->errlog(string("TouchPanel::parsePages: Error opening file ")+cssname);
			pgFile.close();
			return false;
		}
	}
	catch (const fstream::failure e)
	{
		sysl->errlog(string("TouchPanel::parsePages: I/O Error: ")+e.what());
		return false;
	}

	cssFile << "* {\n\tbox-sizing: border-box;\n}\n";
	// Font faces
	cssFile << getFontList()->getFontStyles();
	cssFile.close();

	try
	{
		string maniName = Configuration->getHTTProot()+"/manifest.json";
		jsFile.open(maniName, ios::in | ios::out | ios::trunc | ios::binary);

		if (!jsFile.is_open())
		{
			sysl->errlog(string("TouchPanel::parsePages: Error opening file ")+maniName);
			return false;
		}

		jsFile << "{" << endl << "\t\"short_name\": \"AMXPanel\"," << endl;
		jsFile << "\t\"name\": \"AMX Panel\"," << endl;
		jsFile << "\t\"icons\": [" << endl << "\t\t{" << endl;
		jsFile << "\t\t\t\"src\": \"images/icon.png\"," << endl;
		jsFile << "\t\t\t\"type\": \"image/png\"," << endl;
		jsFile << "\t\t\t\"sizes\": \"256x256\"" << endl << "\t\t}" << endl;
		jsFile << "\t]," << endl;

		if (Configuration->getWSStatus())
			jsFile << "\t\"start_url\": \"https://";
		else
			jsFile << "\t\"start_url\": \"http://";

		jsFile << Configuration->getWebSocketServer() << "/" << Configuration->getWebLocation() << "/index.html\"," << endl;
		jsFile << "\t\"background_color\": \"#5a005a\"," << endl;
		jsFile << "\t\"display\": \"standalone\"," << endl;

		if (Configuration->getWSStatus())
			jsFile << "\t\"scope\": \"https://";
		else
			jsFile << "\t\"scope\": \"http://";

		jsFile << Configuration->getWebSocketServer() << "/" << Configuration->getWebLocation() << "/\"," << endl;
		jsFile << "\t\"theme_color\": \"#5a005a\"," << endl;
		jsFile << "\t\"orientation\": \"landscape\"" << endl << "}" << endl;
		jsFile.close();
	}
	catch (const fstream::failure e)
	{
		sysl->errlog(string("TouchPanel::parsePages: I/O Error: ")+e.what());
		return false;
	}

	getFontList()->serializeToJson();

	// Service worker code
	try
	{
		string cacheName = Configuration->getHTTProot()+"/scripts/sw.js";
		cacheFile.open(cacheName, ios::in | ios::out | ios::trunc | ios::binary);

		if (!cacheFile.is_open())
		{
			sysl->errlog(string("TouchPanel::parsePages: Error opening file ")+cacheName);
			return false;
		}

		cacheFile << endl << "// This is the Service Worker needed to run as a stand allone app." << endl;
		cacheFile << "if('serviceWorker' in navigator)\n{" << endl;
		cacheFile << "\twindow.addEventListener('load', function() {" << endl;
		cacheFile << "\t\tnavigator.serviceWorker.register('" << Configuration->getWebLocation() << "/scripts/sw.js').then(function(registration) {" << endl;
		cacheFile << "\t\t\tdebug(\"Service Worker registration successful width scope: \"+registration.scope);" << endl;
		cacheFile << "\t\t}, function(err) {\n\t\t\terrlog(\"Registration failed:\"+err);" << endl;
		cacheFile << "\t\t})\n\t})\n}" << endl << endl;
		cacheFile << "var cache_name = 'amxpanel-" << VERSION << "'" << endl << endl;
		cacheFile << "var urls_to_cache = [" << endl;
		cacheFile << "\t'" << Configuration->getWebLocation() << "'," << endl;
		cacheFile << "\t'" << Configuration->getWebLocation() << "/scripts/'," << endl;
		cacheFile << "\t'" << Configuration->getWebLocation() << "/images/'" << endl;
		cacheFile << "]" << endl << endl;
		cacheFile << "self.addEventListener('install', function(e) {" << endl;
		cacheFile << "\te.waitUntil(caches.open(cache_name).then(function(cache) {" << endl;
		cacheFile << "\t\treturn cache.addAll(urls_to_cache)\n\t}) )\n})" << endl << endl;
		cacheFile << "self.addEventListener('fetch', function(e) {" << endl;
		cacheFile << "\te.respondWith(caches.match(e.request).then(function(response) {" << endl;
		cacheFile << "\t\tif(response)\n\t\t\treturn response\n\t\telse\n\t\t\treturn fetch(e.request)" << endl;
		cacheFile << "\t}) )\n})" << endl << endl;
		cacheFile.close();
	}
	catch (const fstream::failure e)
	{
		sysl->errlog(string("TouchPanel::parsePages: I/O Error: ")+e.what());
		return false;
	}

	// Page header
	pgFile << "<!DOCTYPE html>\n";
	pgFile << "<html>\n<head>\n<meta charset=\"UTF-8\">\n";
	pgFile << "<title>AMX Panel</title>\n";
	pgFile << "<meta name=\"viewport\" content=\"width=device-width, height=device-height, initial-scale=0.7, minimum-scale=0.7, maximum-scale=1.0, user-scalable=yes\"/>\n";
	pgFile << "<meta name=\"mobile-web-app-capable\" content=\"yes\" />" << endl;
	pgFile << "<meta name=\"apple-mobile-web-app-capable\" content=\"yes\" />\n";
	pgFile << "<meta name=\"apple-mobile-web-app-status-bar-style\" content=\"black\" />" << endl;
	pgFile << "<link rel=\"manifest\" href=\"manifest.json\">" << endl;
	pgFile << "<link rel=\"icon\" sizes=\"256x256\" href=\"images/icon.png\">" << endl;
	pgFile << "<link rel=\"stylesheet\" type=\"text/css\" href=\"amxpanel.css\">" << endl;
	// Scripts
	pgFile << "<script type=\"text/javascript\" src=\"scripts/sw.js\"></script>" << endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/store.modern.min.js\"></script>" << endl;
	pgFile << "<script>" << endl;
	pgFile << "\"use strict\";" << endl;
	pgFile << "var pageName = \"\";" << endl;
	pgFile << "var wsocket = null;" << endl;
	pgFile << "var ws_online = 0;		// 0 = offline, 1 = online, 2 = connecting" << endl;
	pgFile << "var wsStatus = 0;" << endl << endl;
	pgFile << "var browserTests = [" << endl;
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
		string jsname = Configuration->getHTTProot()+"/scripts/pages.js";
		jsFile.open(jsname, ios::in | ios::out | ios::trunc | ios::binary);

		if (!jsFile.is_open())
		{
			sysl->errlog(string("TouchPanel::parsePages: Error opening file ")+jsname);
			pgFile.close();
			return false;
		}

		jsFile << "var Pages;\n\n";
		writePages(jsFile);
		jsFile.close();
	}
	catch (const fstream::failure e)
	{
		sysl->errlog(string("TouchPanel::parsePages: I/O Error: ")+e.what());
		pgFile.close();
//		cacheFile.close();
		return false;
	}

	try
	{
		string jsname = Configuration->getHTTProot()+"/scripts/popups.js";
		jsFile.open(jsname, ios::in | ios::out | ios::trunc | ios::binary);

		if (!jsFile.is_open())
		{
			sysl->errlog(string("TouchPanel::parsePages: Error opening file ")+jsname);
			pgFile.close();
			return false;
		}

		jsFile << "var Popups;\n\n";
		writePopups(jsFile);
		jsFile.close();
	}
	catch (const fstream::failure e)
	{
		sysl->errlog(string("TouchPanel::parsePages: I/O Error: ")+e.what());
		pgFile.close();
//		cacheFile.close();
		return false;
	}

	try
	{
		string jsname = Configuration->getHTTProot()+"/scripts/groups.js";
		jsFile.open(jsname, ios::in | ios::out | ios::trunc | ios::binary);

		if (!jsFile.is_open())
		{
			sysl->errlog(string("TouchPanel::parsePages: Error opening file ")+jsname);
			pgFile.close();
			return false;
		}

		jsFile << "var popupGroups;\n\n";
		writeGroups(jsFile);
		jsFile.close();
	}
	catch (const fstream::failure e)
	{
		sysl->errlog(string("TouchPanel::parsePages: I/O Error: ")+e.what());
		pgFile.close();
		return false;
	}

	try
	{
		string jsname = Configuration->getHTTProot()+"/scripts/btarray.js";
		jsFile.open(jsname, ios::in | ios::out | ios::trunc | ios::binary);

		if (!jsFile.is_open())
		{
			sysl->errlog(string("TouchPanel::parsePages: Error opening file ")+jsname);
			pgFile.close();
			return false;
		}

		jsFile << "var buttonArray;\n\n";
		writeBtArray(jsFile);
		jsFile.close();
	}
	catch (const fstream::failure e)
	{
		sysl->errlog(string("TouchPanel::parsePages: I/O Error: ")+e.what());
		pgFile.close();
		return false;
	}

	try
	{
		string jsname = Configuration->getHTTProot()+"/scripts/icons.js";
		jsFile.open(jsname, ios::in | ios::out | ios::trunc | ios::binary);

		if (!jsFile.is_open())
		{
			sysl->errlog(string("TouchPanel::parsePages: Error opening file ")+jsname);
			pgFile.close();
			return false;
		}

		writeIconTable(jsFile);
		jsFile.close();
	}
	catch (const fstream::failure e)
	{
		sysl->errlog(string("TouchPanel::parsePages: I/O Error: ")+e.what());
		pgFile.close();
		return false;
	}

	try
	{
		string jsname = Configuration->getHTTProot()+"/scripts/bargraphs.js";
		jsFile.open(jsname, ios::in | ios::out | ios::trunc | ios::binary);

		if (!jsFile.is_open())
		{
			sysl->errlog(string("TouchPanel::parsePages: Error opening file ")+jsname);
			pgFile.close();
			return false;
		}

		writeBargraphs(jsFile);
		jsFile.close();
	}
	catch (const fstream::failure e)
	{
		sysl->errlog(string("TouchPanel::parsePages: I/O Error: ")+e.what());
		pgFile.close();
		return false;
	}

	try
	{
		string jsname = Configuration->getHTTProot()+"/scripts/palette.js";
		jsFile.open(jsname, ios::in | ios::out | ios::trunc | ios::binary);

		if (!jsFile.is_open())
		{
			sysl->errlog(string("TouchPanel::parsePages: Error opening file ")+jsname);
			pgFile.close();
			return false;
		}

		jsFile << getPalettes()->getJson();
		jsFile.close();
	}
	catch (const fstream::failure e)
	{
		sysl->errlog(string("TouchPanel::parsePages: I/O Error: ")+e.what());
		pgFile.close();
		return false;
	}

	try
	{
		string jsname = Configuration->getHTTProot()+"/scripts/resource.js";
		jsFile.open(jsname, ios::in | ios::out | ios::trunc | ios::binary);

		if (!jsFile.is_open())
		{
			sysl->errlog(string("TouchPanel::parsePages: Error opening file ")+jsname);
			pgFile.close();
			return false;
		}

		PROJECT_T prj = getProject();
		vector<RESOURCE_LIST_T>& resList = prj.resourceLists;
		jsFile << "var ressources = { \"ressources\":[" << endl;

		for (size_t i = 0; i < resList.size(); i++)
		{
			if (i > 0)
				jsFile << "," << endl;

			vector<RESOURCE_T>& res = resList[i].ressource;
			jsFile << "\t{\"type\":\"" << resList[i].type << "\",\"ressource\":[" << endl;

			for (size_t j = 0; j < res.size(); j++)
			{
				if (j > 0)
					jsFile << "," << endl;

				jsFile << "\t\t{\"name\":\"" << res[j].name << "\",\"protocol\":\"" << res[j].protocol << "\"," << endl;
				jsFile << "\t\t \"user\":\"" << res[j].user << "\",\"password\":\"" << res[j].password << "\"," << endl;
				jsFile << "\t\t \"encrypted\":" << ((res[j].encrypted)?"true":"false") << ",\"host\":\"" << res[j].host << "\"," << endl;
				jsFile << "\t\t \"path\":\"" << res[j].path << "\",\"file\":\"" << res[j].file << "\",\"refresh\":" << res[j].refresh << "}";
			}

			jsFile << endl << "\t]}";
		}

		jsFile << endl << "]};" << endl;
		jsFile.close();
	}
	catch (const fstream::failure e)
	{
		sysl->errlog(string("TouchPanel::parsePages: I/O Error: ")+e.what());
		pgFile.close();
		return false;
	}

	pgFile << "</script>" << endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/browser.js\"></script>" << endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/pages.js\"></script>" << endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/popups.js\"></script>" << endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/groups.js\"></script>" << endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/btarray.js\"></script>" << endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/icons.js\"></script>" << endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/bargraphs.js\"></script>" << endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/palette.js\"></script>" << endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/fonts.js\"></script>" << endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/chameleon.js\"></script>" << endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/resource.js\"></script>" << endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/movie.js\"></script>" << endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/keyboard.js\"></script>" << endl << endl;

	for (size_t i = 0; i < stPages.size(); i++)
	{
		pgFile << "<script type=\"text/javascript\" src=\"scripts/Page" << stPages[i].ID << ".js\"></script>" << endl;
	}

	pgFile << endl;

	for (size_t i = 0; i < stPopups.size(); i++)
	{
		pgFile << "<script type=\"text/javascript\" src=\"scripts/Page" << stPopups[i].ID << ".js\"></script>" << endl;
	}

	pgFile << endl << "<script type=\"text/javascript\" src=\"scripts/page.js\"></script>" << endl;
	pgFile << "<script type=\"text/javascript\" src=\"scripts/amxpanel.js\"></script>" << endl;
	// Add some special script functions
	pgFile << "<script>\n";
	pgFile << scrBuffer << endl;
	// This is the WebSocket connection function
	pgFile << "function connect()\n{\n";
	pgFile << "\tif (wsocket !== null && (wsocket.readyState == WebSocket.OPEN || wsocket.readyState == WebSocket.CLOSING) && ws_online > 0)" << endl;
	pgFile << "\t\treturn;" << endl << endl;
	pgFile << "\ttry\n\t{\n\t\tws_online = 2;" << endl;

	if (Configuration->getWSStatus())
		pgFile << "\t\twsocket = new WebSocket(\"wss://" << Configuration->getWebSocketServer() << ":" << Configuration->getSidePort() << "/\");\n";
	else
		pgFile << "\t\twsocket = new WebSocket(\"ws://" << Configuration->getWebSocketServer() << ":" << Configuration->getSidePort() << "/\");\n";

	pgFile << "\t\twsocket.onopen = function() {" << endl;
    pgFile << "\t\t\tgetRegistrationID();\n\t\t\tws_online = 1;\t\t// online\n\t\t\tsetOnlineStatus(1);\n" << endl;
	pgFile << "\t\t\tif (!regStatus)\n\t\t\t{" << endl;
	pgFile << "\t\t\t\tif (typeof registrationID == \"string\" && registrationID.length > 0)\n";
	pgFile << "\t\t\t\t\twsocket.send('REGISTER:'+registrationID+';');\n";
	pgFile << "\t\t\t\telse\n\t\t\t\t\terrlog(\"connect: Missing registration ID!\");\n\t\t\t}\n\t\t}" << endl;
	pgFile << "\t\twsocket.onerror = function(error) { errlog('WebSocket error: '+error); setOnlineStatus(9); }\n";
	pgFile << "\t\twsocket.onmessage = function(e) { parseMessage(e.data); }\n";
	pgFile << "\t\twsocket.onclose = function() {\n\t\t\tTRACE('WebSocket is closed!');" << endl;
	pgFile << "\t\t\tws_online = 0;\t\t// offline\n\t\t\tregStatus = false;\n\t\t\tsetOnlineStatus(0);\n\t\t}\n\t}\n\tcatch (exception)\n";
	pgFile << "\t{\n\t\tsetOnlineStatus(0);\n\t\tconsole.error(\"Error initializing: \"+exception);\n\t}\n}\n\n";
	// This is the "main" program
	PROJECT_T prg = getProject();
	pgFile << "function main()\n{\n";
	pgFile << "\tif (isIOS() || (isFirefox() && isAndroid()))\n\t{" << endl;
	pgFile << "\t\tEVENT_DOWN = \"touchstart\";\n\t\tEVENT_UP = \"touchend\";\n\t\tEVENT_MOVE = \"touchmove\";" << endl;
	pgFile << "\t\tTRACE(\"main: Events were set to TOUCH...\");\n\t}" << endl;
	pgFile << "\telse if (isFirefox() || isSafari() || isMacOS())" << endl;
	pgFile << "\t{\n\t\tEVENT_DOWN = \"mousedown\";\n\t\tEVENT_UP = \"mouseup\";\n\t\tEVENT_MOVE = \"mousemove\";" << endl;
	pgFile << "\t\tTRACE(\"main: Events were set to MOUSE...\");\n\t}\n" << endl;
	pgFile << "\thandleStandby();" << endl;
	pgFile << "\tvar elem = document.documentElement;\n\n\tif (elem.requestFullscreen)\n";
    pgFile << "\t\telem.requestFullscreen();\n";
	pgFile << "\telse if (elem.mozRequestFullScreen)\t/* Firefox */\n";
    pgFile << "\t\telem.mozRequestFullScreen();\n";
	pgFile << "\telse if (elem.webkitRequestFullscreen)\t/* Chrome, Safari and Opera */\n";
    pgFile << "\t\telem.webkitRequestFullscreen();\n\n";
	pgFile << "\twindow.addEventListener('online',  onOnline);\n";
	pgFile << "\twindow.addEventListener('offline', onOffline);\n";
	pgFile << "\tshowPage('"<< prg.panelSetup.powerUpPage << "');\n";

	for (size_t i = 0; i < prg.panelSetup.powerUpPopup.size(); i++)
		pgFile << "\tshowPopup('" << prg.panelSetup.powerUpPopup[i] << "');\n";

	pgFile << string("\t")+scrStart+"\n";
	pgFile << "}\n";
	pgFile << "</script>\n";
	pgFile << "</head>\n";
	// The page body
//	pgFile << "<body onload=\"main(); connect(); onlineStatus();\">" << endl;
	pgFile << "<body onload=\"main(); connect();\">" << endl;
	pgFile << "   <div id=\"main\"></div>" << endl;
	pgFile << "</body>\n</html>\n";
	pgFile.close();
	// Mark as parsed
	try
	{
		string nm = Configuration->getHTTProot()+"/.parsed";
		pgFile.open(nm, ios::in | ios::out | ios::trunc | ios::binary);
		DateTime dt;
		pgFile << dt.toString();
		pgFile.close();
	}
	catch(const fstream::failure e)
	{
		sysl->errlog(string("TouchPanel::parsePages: I/O Error: ")+e.what());
		return false;
	}

	return true;
}

bool TouchPanel::isPresent(const vector<string>& vs, const string& str)
{
	DECL_TRACER(string("TouchPanel::isPresent(const vector<string>& vs, const string& str)"));

	for (size_t i = 0; i < vs.size(); i++)
	{
		if (str.compare(vs[i]) == 0)
			return true;
	}

	return false;
}

void TouchPanel::writeGroups (fstream& pgFile)
{
	DECL_TRACER(string("TouchPanel::writeGroups (fstream& pgFile)"));
	vector<string> grName;
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

void TouchPanel::writePages(fstream& pgFile)
{
	DECL_TRACER(string("TouchPanel::writePages(fstream& pgFile)"));
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
	DECL_TRACER(string("TouchPanel::writePopups (fstream& pgFile)"));
	bool first = true;
	pgFile << "var Popups = {\"pages\":[";

	for (size_t i = 0; i < stPopups.size(); i++)
	{
		if (!first)
			pgFile << ",";

		pgFile << "\n\t\t{\"name\":\"" << stPopups[i].name << "\",\"ID\":" << stPopups[i].ID << ",\"group\":\"" << stPopups[i].group << "\",\"active\":false,\"lnpage\":\"\",\"modality\":" << ((stPopups[i].modal) ? "true" : "false") << "}";
		first = false;
	}

	pgFile << "\n\t]};\n";
}

void TouchPanel::writeBtArray(fstream& pgFile)
{
	DECL_TRACER(string("TouchPanel::writeBtArray(fstream& pgFile)"));

	pgFile << "var buttonArray = {" << scBtArray << "\n\t};\n";
}

void TouchPanel::writeIconTable(fstream& pgFile)
{
    DECL_TRACER(string("TouchPanel::writeIconTable(fstream& pgFile)"));

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

void TouchPanel::writeBargraphs(fstream& pgFile)
{
	DECL_TRACER(string("TouchPanel::writeBargraphs(fstream& pgFile)"));
	pgFile << "var bargraphs = {\"bargraphs\":[\n" << sBargraphs << "\n]};\n";
}

bool TouchPanel::isParsed()
{
	DECL_TRACER(string("TouchPanel::isParsed()"));
	fstream pgFile;
	// Did we've already parsed?
	try
	{
		string nm = Configuration->getHTTProot()+"/.parsed";
		pgFile.open(nm, ios::in);

		if (pgFile.is_open())
		{
			sysl->log(Syslog::INFO, string("TouchPanel::parsePages: Pages are already parsed. Skipping!"));
			pgFile.close();
			return true;
		}

		pgFile.close();
	}
	catch(const fstream::failure e)
	{
		sysl->errlog(string("TouchPanel::parsePages: I/O Error: ")+e.what());
	}

	return false;
}

void TouchPanel::setWebConnect(bool s, long pan)
{
	std::lock_guard<std::mutex> lock(mut);
	DECL_TRACER("TouchPanel::setWebConnect(bool s, websocketpp::connection_hdl hdl)");

	PANELS_T::iterator itr;
	size_t i = 0;

	for (itr = registration.begin(); itr != registration.end(); ++itr)
	{
		if (itr->second.pan == pan)
		{
			itr->second.status = s;
			break;
		}

		i++;

		if (i >= registration.size())
			break;
	}

	cond.notify_one();
}

void TouchPanel::showContent(long pan)
{
	if (!Configuration->getDebug())
		return;

	DECL_TRACER("TouchPanel::showContent(long pan)");

	bool found = false;
	PANELS_T::iterator itr;
	size_t i = 0;
	sysl->log_serial(Syslog::IDEBUG, "  DBG");
	sysl->log_serial(Syslog::IDEBUG, "  DBG     \"size\" : "+to_string(registration.size()));

	for (itr = registration.begin(); itr != registration.end(); ++itr)
	{
		if (itr->second.pan == pan)
		{
			sysl->log_serial(Syslog::IDEBUG, "  DBG     \"first\": "+to_string(itr->first));
			sysl->log_serial(Syslog::IDEBUG, "  DBG     channel: "+to_string(itr->second.channel));
			sysl->log_serial(Syslog::IDEBUG, "  DBG     pan    : "+to_string(itr->second.pan));
			sysl->log_serial(Syslog::IDEBUG, "  DBG     regID  : "+itr->second.regID);
			sysl->log_serial(Syslog::IDEBUG, "  DBG     status : "+to_string(itr->second.status));
			sysl->log_serial(Syslog::IDEBUG, "  DBG     *amxnet: "+((itr->second.amxnet == 0)?string("NULL"):string("<pointer>")));
			sysl->DebugMsg("Debug:  DBG");
			found = true;
		}

		i++;

		if (i >= registration.size())
			break;
	}

	if (!found)
		sysl->DebugMsg("TouchPanel::showContent: Content not found!");
}

string TouchPanel::getSerialNum()
{
	DECL_TRACER("TouchPanel::getSerialNum()");

	char hv[32];
	snprintf(hv, sizeof(hv), "201903XTHE%ld", serNum);
	string sn(hv);
	serNum++;
	return sn;
}
