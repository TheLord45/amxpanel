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
#include "panel.h"
#include "page.h"
#include "request.h"

namespace amx
{
	class TouchPanel : public Panel
	{
		public:
			TouchPanel();
			~TouchPanel();

			strings::String getPage(int id);
			strings::String getPage(const strings::String& name);
			strings::String getStartPage();

			void setCommand(const strings::String& cmd);
			strings::String requestPage(const http::server::Request& req);

		private:
			void readPages();
			std::vector<Page> pages;
			Panel panel;
			int openPage;		// The index number of the currently open page
			std::vector<int> openPopups;	// The currently open popups connected
			bool busy;

			std::vector<strings::String> commands;		// Commands from controller
	};
}

#endif
