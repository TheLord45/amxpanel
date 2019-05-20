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

#ifndef __TOUCHPANEL_H__
#define __TOUCHPANEL_H__

#include <vector>
#include <array>
#include <memory>
#include <fstream>
#include <functional>
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

	typedef struct
	{
		int channel;				// The channel used for the panel (>10000 && <11000)
		strings::String regID;		// The registration ID of the client
		AMXNet *amxnet;				// The class communicating with the AMX controller
		bool status;
	}REGISTRATION_T;

	typedef struct
	{
		AMXNet *amxnet;
		std::thread *thr;
		websocketpp::connection_hdl ws_hdl;
	}PANELMAP_T;

	class TouchPanel : public Panel, WebSocket
	{
		public:
			TouchPanel();
			~TouchPanel();

			int findPage(const strings::String& name);
			bool parsePages();

			void setCommand(const struct ANET_COMMAND& cmd);
			void webMsg(std::string& msg);
			void stopClient();
			void setWebConnect(bool s, websocketpp::connection_hdl& hdl);
			bool getWebConnect(AMXNet *);
			void regWebConnect(websocketpp::connection_hdl& hdl, int id);

		private:
			void writeStyles(std::fstream& pgFile);
			void readPages();
            void writePages(std::fstream& pgFile);
			void writeGroups(std::fstream& pgFile);
			void writePopups(std::fstream& pgFile);
			void writeAllPopups(std::fstream& pgFile);
			void writeBtArray(std::fstream& pgFile);
			void writeIconTable(std::fstream& pgFile);
			void writeBargraphs(std::fstream& pgFile);
			bool isPresent(const std::vector<strings::String>& vs, const strings::String& str);
			bool isParsed();
			uint64_t getMS();
			bool haveFreeSlot();
			int getFreeSlot();
			bool isRegistered(strings::String& regID);
			bool isRegistered(int channel);
			bool registerSlot(int channel, strings::String& regID);
			bool releaseSlot(int channel);
			bool releaseSlot(strings::String& regID);
			bool newConnection(int id);
			AMXNet *getConnection(int id);
			AMXNet *findConnection(int id);
			bool delConnection(int id);

			std::map<int, PANELMAP_T> panels;
			strings::String scrBuffer;
			strings::String scrStart;
			strings::String scBtArray;
			strings::String sBargraphs;
			bool gotPages;
			int openPage;		// The index number of the currently open page
			std::vector<int> openPopups;	// The currently open popups connected
			std::vector<ST_PAGE> stPages;
			std::vector<ST_POPUP> stPopups;
			std::vector<PAGE_T> pageList;
			bool busy;
			bool webConnected;
//			bool registrated;		// TRUE = Client (browser) is identified and has access

			std::vector<ANET_COMMAND> commands;		// Commands from controller
			strings::String amxBuffer;				// This is the cache for incomplete commands
			uint64_t lastDisconnect;
			std::vector<REGISTRATION_T> registration;
	};
}

#endif
