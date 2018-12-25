/*
 *   Copyright (C) 2018 by Andreas Theofilu (TheoSys) <andreas@theosys.at>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation version 3 of the License.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <fstream>
#include "strings.h"

class Config
{
	public:
		Config();
		~Config();

		strings::String getListen() { return sListen; }
		int getPort() { return nPort; }
		strings::String getHTTProot() { return sHTTProot; }
		strings::String getPidFile() { return sPidFile; }
		strings::String getUser() { return usr; }
		strings::String getGroup() { return grp; }
		bool getDebug() { return Debug; }
		strings::String getLogFile() { return LogFile; }
		strings::String getFontPath() { return FontPath; }
		strings::String getWebLocation() { return web_location; }
		strings::String getAMXPanelType() { return AMXPanelType; }
		strings::String getAMXController() { return AMXController; }
		int getAMXPort() { return AMXPort; }
		int getAMXChannel() { return AMXChanel; }
		int getAMXSystem() { return AMXSystem; }

		void setHOME(const strings::String& hm) { HOME = hm.data(); }

		void Initialize() { readConfig(sFileName); }
		bool isInitialized() { return initialized; }

	protected:
		void init();
		void readConfig(const strings::String &file);

	private:
		strings::String sListen;
		int nPort;
		strings::String sHTTProot;
		bool Debug;
		strings::String LogFile;
		strings::String FontPath;
		strings::String web_location;
		strings::String AMXPanelType;
		strings::String AMXController;
		int AMXPort;
		int AMXChanel;
		int AMXSystem;

		std::ifstream fs;
		strings::String sFileName;
		bool fflag;
		strings::String sPidFile;
		strings::String grp;
		strings::String usr;
		const char *HOME;
		bool initialized;
};

#endif  // __CONFIG_H__
