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
#include "atomicvector.h"

#define VERSION		"1.1.0"
#define PAIR(ID, REG)	std::pair<int, REGISTRATION_T>(ID, REG)

namespace amx
{
	typedef struct ST_PAGE
	{
		int ID;						// ID of page
		std::string name;		// Name of page
		std::string file;		// File name of page
		bool active;				// true = active/visible.
		std::string styles;		// The needed styles
		std::string webcode;	// The webcode
	}ST_PAGE;

	typedef struct ST_POPUP
	{
		int ID;						// ID of popup
		std::string name;			// Name of page
		std::string file;			// File name of page
		bool active;				// true = visible
		bool modal;					// true = popup is modal
		std::string group;			// Group name
		std::vector<int> onPages;	// Linked to page ID
		std::string styles;			// The needed styles
		std::string webcode;		// The webcode
	}ST_POPUP;

	typedef struct REGISTRATION_T
	{
		int channel;						// The channel used for the panel (>10000 && <11000)
		std::string regID;					// The registration ID of the client
		AMXNet *amxnet;						// The class communicating with the AMX controller
		long pan;							// The handle to the connection to the internet browser
		bool status;
		std::string amxBuffer;				// Individual buffer for inclomplete commands

		REGISTRATION_T& operator= (REGISTRATION_T& reg) {
			channel = reg.channel;
			regID = reg.regID;
			amxnet = reg.amxnet;
			pan = reg.pan;
			status = reg.status;
			amxBuffer = reg.amxBuffer;
			return reg;
		}
	}REGISTRATION_T;

	typedef std::map<int, REGISTRATION_T> PANELS_T;

	class TouchPanel : public Panel, WebSocket
	{
		PANELS_T registration{};
		std::string scrBuffer{""};
		std::string scrStart{""};
		std::string scBtArray{""};
		std::string sBargraphs{""};
		std::vector<ST_PAGE> stPages{};
		std::vector<ST_POPUP> stPopups{};
		std::vector<PAGE_T> pageList{};
		std::atomic<bool> busy{};
		std::string none{""};
		long serNum{0};

		AtomicVector<ANET_COMMAND> commands{};		// Commands from controller
		std::mutex mut{};
		std::condition_variable cond{};

		public:
			TouchPanel();
			~TouchPanel();

			int findPage(const std::string& name);
			bool parsePages();

			void setCommand(const ANET_COMMAND& cmd);
			void webMsg(std::string& msg, long pan);
			void stopClient();
			void setWebConnect(bool s, long pan);
			void regWebConnect(long pan, int id);

		private:
			void readPages();
			void writePages(std::fstream& pgFile);
			void writeGroups(std::fstream& pgFile);
			void writePopups(std::fstream& pgFile);
			void writeBtArray(std::fstream& pgFile);
			void writeIconTable(std::fstream& pgFile);
			void writeBargraphs(std::fstream& pgFile);
			bool isPresent(const std::vector<std::string>& vs, const std::string& str);
			bool isParsed();
			bool haveFreeSlot();
			int getFreeSlot();
			bool isRegistered(const std::string& regID);
			bool isRegistered(int channel);
			bool registerSlot(int channel, std::string& regID, long pan);
			bool releaseSlot(int channel);
			bool releaseSlot(const std::string& regID);
			bool newConnection(int id);
			AMXNet *getConnection(int id);
			bool delConnection(int id);
			std::string& getAMXBuffer(int id);
			void setAMXBuffer(int id, const std::string& buf);
			bool send(int id, std::string& msg);
			bool replaceSlot(PANELS_T::iterator key, REGISTRATION_T& reg);
			std::string getSerialNum();


			void showContent(long pan);
	};
}

#endif
