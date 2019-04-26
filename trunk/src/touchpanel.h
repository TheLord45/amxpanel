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

	class TouchPanel : public Panel, WebSocket
	{
		public:
			TouchPanel();
			~TouchPanel();

//			strings::String getPage(int id);
//			strings::String getPageStyle(int id);
//			strings::String getPage(const strings::String& name);
//			strings::String getPageStyle(const strings::String& name);
			int findPage(const strings::String& name);
//			int getActivePage();
			bool parsePages();

			void setCommand(const struct ANET_COMMAND& cmd);
			void webMsg(std::string& msg);
			bool startClient();
			void stopClient();
			void setWebConnect(bool s) { webConnected = s; }
			bool getWebConnect() { return webConnected; }

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

			AMXNet *amxnet;
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
			bool registrated;		// TRUE = Client (browser) is identified and has access

			std::vector<ANET_COMMAND> commands;		// Commands from controller
			strings::String amxBuffer;				// This is the cache for incomplete commands
	};
}

#endif
