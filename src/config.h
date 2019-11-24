/*
 *   Copyright (C) 2018 by Andreas Theofilu (TheoSys) <andreas@theosys.at>
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

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <fstream>
extern "C" {
#include <libcidr.h>
}
#include <string>
#include <vector>

class Config
{
	public:
		Config();
		~Config();

		std::string getListen() { return sListen; }
		int getPort() { return nPort; }
		std::string getHTTProot() { return sHTTProot; }
		std::string getPidFile() { return sPidFile; }
		std::string getUser() { return usr; }
		std::string getGroup() { return grp; }
		bool getDebug() { return Debug; }
		std::string getLogFile() { return LogFile; }
		std::string getFontPath() { return FontPath; }
		std::string getWebLocation() { return web_location; }
		std::string getAMXPanelType() { return AMXPanelType; }
		std::string getAMXController() { return AMXController; }
		int getAMXPort() { return AMXPort; }
		int getAMXChannel() { return AMXChanel; }
		std::vector<int>& getAMXChannels() { return AMXChanels; }
		int getAMXSystem() { return AMXSystem; }
		int getSidePort() { return sidePort; }
		std::string getSSHServerFile() { return sshServerFile; }
		std::string getSSHDHFile() { return sshDHFile; }
		std::string getSSHPassword() { return sshPassword; }
		std::string getWebSocketServer() { return webSocketServer; }
		std::string getHashTablePath() { return hashTablePath; }
		std::vector<std::string>& getHashTable(const std::string& path);
		bool isAllowedNet(std::string& net);
		bool getWSStatus() { return wsStatus; }
		std::string getClientLog() { return ClientLog; }

		void setHOME(const std::string& hm) { HOME = hm.data(); }

		void Initialize() { readConfig(sFileName); }
		bool isInitialized() { return initialized; }

	protected:
		void init();
		void readConfig(const std::string &file);

	private:
		void parseNets(std::string& nets);

		std::string sListen;
		int nPort;
		std::string sHTTProot;
		bool Debug;
		std::string LogFile;
		std::string ClientLog;
		std::string FontPath;
		std::string web_location;
		std::string AMXPanelType;
		std::string AMXController;
		int AMXPort;
		int AMXChanel;
		std::vector<int> AMXChanels;
		int AMXSystem;
		int sidePort;
		std::string sshServerFile;
		std::string sshDHFile;
		std::string sshPassword;
		std::string webSocketServer;
		std::string hashTablePath;
		std::vector<std::string> hashTable;
		std::vector<CIDR> allowedNet;

		std::ifstream fs;
		std::string sFileName;
		bool fflag;
		std::string sPidFile;
		std::string grp;
		std::string usr;
		const char *HOME;
		bool initialized;
		bool wsStatus;
};

#endif  // __CONFIG_H__
