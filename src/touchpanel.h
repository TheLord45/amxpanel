/*
 * Copyright (C) 2018 by Andreas Theofilu <andreas@theosys.at>
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

#ifndef __TOUCHPANEL_H__
#define __TOUCHPANEL_H__

#include <vector>
#include <array>
#include <memory>
#include <fstream>
#include <functional>
#include <iterator>
#include <map>
#ifdef __APPLE__
#include <boost/asio.hpp>
#else
#include <asio.hpp>
#endif
#include "panel.h"
#include "page.h"
#include "amxnet.h"
#include "websocket.h"
#include "fontlist.h"

#define VERSION		"1.0.1"
#define PAIR(ID, REG)	std::pair<int, REGISTRATION_T>(ID, REG)

namespace amx
{
	typedef struct ST_PAGE
	{
		int ID;						// ID of page
		strings::String name;		// Name of page
		strings::String file;		// File name of page
		bool active;				// true = active/visible.
		strings::String styles;		// The needed styles
		strings::String webcode;	// The webcode
	}ST_PAGE;

	typedef struct ST_POPUP
	{
		int ID;						// ID of popup
		strings::String name;		// Name of page
		strings::String file;		// File name of page
		bool active;				// true = visible
		strings::String group;		// Group name
		std::vector<int> onPages;	// Linked to page ID
		strings::String styles;		// The needed styles
		strings::String webcode;	// The webcode
	}ST_POPUP;

	typedef struct REGISTRATION_T
	{
		int channel;						// The channel used for the panel (>10000 && <11000)
		strings::String regID;				// The registration ID of the client
		amx::AMXNet *amxnet;				// The class communicating with the AMX controller
		long pan;							// The handle to the connection to the internet browser
		bool status;

		REGISTRATION_T& operator= (REGISTRATION_T& reg) {
			channel = reg.channel;
			regID = reg.regID;
			amxnet = reg.amxnet;
			pan = reg.pan;
			status = reg.status;
			return reg;
		}
	}REGISTRATION_T;

	typedef std::map<int, REGISTRATION_T> PANELS_T;

	class TouchPanel : public Panel, WebSocket
	{
		public:
			TouchPanel();
			~TouchPanel();

			int findPage(const strings::String& name);
			bool parsePages();

			void setCommand(const struct ANET_COMMAND& cmd);
			void webMsg(std::string& msg, long pan);
			void stopClient();
			void setWebConnect(bool s, long pan);
			bool getWebConnect(AMXNet *);
			void regWebConnect(long pan, int id);

		private:
			void readPages();
			void writePages(std::fstream& pgFile);
			void writeGroups(std::fstream& pgFile);
			void writePopups(std::fstream& pgFile);
			void writeBtArray(std::fstream& pgFile);
			void writeIconTable(std::fstream& pgFile);
			void writeBargraphs(std::fstream& pgFile);
			bool isPresent(const std::vector<strings::String>& vs, const strings::String& str);
			bool isParsed();
			bool haveFreeSlot();
			int getFreeSlot();
			bool isRegistered(strings::String& regID);
			bool isRegistered(int channel);
			bool registerSlot(int channel, strings::String& regID, long pan);
			bool releaseSlot(int channel);
			bool releaseSlot(strings::String& regID);
			bool newConnection(int id);
			AMXNet *getConnection(int id);
			bool delConnection(int id);
			bool send(int id, strings::String& msg);
			bool replaceSlot(PANELS_T::iterator key, REGISTRATION_T& reg);

			void showContent(long pan);

			PANELS_T registration;
			strings::String scrBuffer;
			strings::String scrStart;
			strings::String scBtArray;
			strings::String sBargraphs;
			bool gotPages;
			std::vector<ST_PAGE> stPages;
			std::vector<ST_POPUP> stPopups;
			std::vector<PAGE_T> pageList;
			bool busy;

			std::vector<ANET_COMMAND> commands;		// Commands from controller
			strings::String amxBuffer;				// This is the cache for incomplete commands
	};
}

#endif
