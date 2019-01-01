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
#ifdef __APPLE__
#include <boost/asio.hpp>
#else
#include <asio.hpp>
#endif
#include "panel.h"
#include "page.h"
#include "amxnet.h"
#include "request.h"
#include "websocket.h"
#include "fontlist.h"

namespace amx
{
	typedef struct ST_PAGE
	{
		int ID;				// ID of page
		bool active;		// true = active/visible.
	}ST_PAGE;

	typedef struct ST_POPUP
	{
		int ID;						// ID of popup
		bool active;				// true = visible
		strings::String group;		// Group name
		std::vector<int> onPages;	// Linked to page ID
	}ST_POPUP;

	class TouchPanel : public Panel, WebSocket
	{
		public:
			TouchPanel();
			~TouchPanel();

			strings::String getPage(int id);
			strings::String getPageStyle(int id);
			strings::String getPage(const strings::String& name);
			strings::String getPageStyle(const strings::String& name);
			strings::String getStartPage();
			int findPage(const strings::String& name);
			int getActivePage();
			bool parsePages();

			void setCommand(const struct ANET_COMMAND& cmd);
			strings::String requestPage(const http::server::Request& req);
			bool startClient();

		private:
			void readPages();
			void writeGroups(std::fstream& pgFile);
			void writePopups(std::fstream& pgFile);
			void writeAllPopups(std::fstream& pgFile);

			std::vector<Page> pages;
			Panel panel;
			int openPage;		// The index number of the currently open page
			std::vector<int> openPopups;	// The currently open popups connected
			std::vector<ST_PAGE> stPages;
			std::vector<ST_POPUP> stPopups;
			bool busy;

			std::vector<ANET_COMMAND> commands;		// Commands from controller
	};
}

#endif
