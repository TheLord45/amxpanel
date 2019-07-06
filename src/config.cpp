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
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <bitset>
#include <iostream>
#include <vector>
#include <unistd.h>
#include "config.h"
#include "syslog.h"

extern std::string pName;
extern Syslog *sysl;
std::runtime_error rConfig("Error opening a file");

using namespace strings;

Config::Config()
{
	sysl->TRACE(Syslog::ENTRY, std::string("Config::Config()"));
	this->fflag = false;
	initialized = false;
	sFileName.clear();
	sPidFile.clear();
	Debug = false;
	AMXPanelType.clear();
	AMXController.clear();

	init();

	try
	{
		readConfig(sFileName);
	}
	catch (const std::exception e)
	{
		sysl->errlog(String("Error reading config file ")+sFileName+": "+e.what());
	}
}

void Config::init()
{
	sysl->TRACE(Syslog::MESSAGE, std::string("Config::init()"));

	if (!(HOME = std::getenv("HOME")))
	{
		sysl->errlog(String("Error getting environment variable HOME!"));
		HOME = 0;
	}

	if (!access("/etc/amxpanel.conf", R_OK))
		sFileName = "/etc/amxpanel.conf";
	else if (!access("/etc/amxpanel/amxpanel.conf", R_OK))
		sFileName = "/etc/amxpanel/amxpanel.conf";
	else if (!access("/usr/etc/amxpanel.conf", R_OK))
		sFileName = "/usr/etc/amxpanel.conf";
#ifdef __APPLE__
	if (!access("/opt/local/etc/amxpanel.conf", R_OK))
		sFileName = "/opt/local/etc/amxpanel.conf";
	else if (!access("/opt/local/etc/amxpanel/amxpanel.conf", R_OK))
		sFileName = "/opt/local/etc/amxpanel/amxpanel.conf";
	else if (!access("/opt/local/usr/etc/amxpanel.conf", R_OK))
		sFileName = "/opt/local/usr/etc/amxpanel.conf";
#endif
	else if (HOME)
	{
		sFileName = HOME;
		sFileName += "/.amxpanel.conf";

		if (access(sFileName.data(), R_OK))
		{
			sysl->errlog(String("Error: Can't find any configuration file!"));
			sFileName.clear();
		}
	}
	else
		sFileName.clear();

	// Initialize Variables
	sListen = "0.0.0.0";
	nPort = 11011;
	sHTTProot = "/usr/share/amxpanel/html";
	sPidFile = "/var/run/amxpanel.pid";
	usr = "nobody";
	grp = "nobody";
	LogFile = "";
	FontPath = "/usr/share/amxpanel/fonts";
	web_location = "/amxpanel";
	AMXPanelType = "MVS5200i";
	AMXController = "0.0.0.0";
	AMXPort = 1319;
	AMXChanel = 0;
	AMXSystem = 1;
	sidePort = 11012;
	sshServerFile = "server.pem";
	sshDHFile = "dh.pem";
	sshPassword.clear();
	webSocketServer = "www.theosys.at";
	hashTablePath = "/etc/amxpanel/hashtable.tbl";
	wsStatus = true;	// We'll use WSS WEB sockets by default
	CIDR *addr = cidr_from_str("127.0.0.1");
	allowedNet.push_back(*addr);
	cidr_free(addr);
	addr = cidr_from_str("::1");
	allowedNet.push_back(*addr);
	cidr_free(addr);
}

void Config::readConfig(const String &sFile)
{
	sysl->TRACE(Syslog::MESSAGE, std::string("Config::readConfig(const String &sFile)"));

	if (this->fflag)
	{
		this->fflag = false;
		initialized = false;
	}

	if (fs.is_open())
		fs.close();

	try
	{
		fs.open(sFile.data(), std::fstream::in);
	}
	catch (const std::fstream::failure e)
	{
		sysl->errlog(String("Error on file ")+sFile+": "+e.what());
		throw rConfig;
	}

	sysl->log(Syslog::INFO,String("Config::readConfig: Reading from: ")+sFile);

	for (std::string line; std::getline(fs, line);)
	{
		String ln = line;
		std::vector<String> parts = ln.split('=');

		if (parts.size() == 2)
		{
			String left = parts[0];
			String right = parts[1];

//			left.toUpper();
			sysl->log(Syslog::INFO, String("Config::readConfig: ")+left+" : "+right);

			if (left.caseCompare("LISTEN") == 0 && !right.empty())
				sListen = right.trim();
			else if (left.caseCompare("PORT") == 0 && !right.empty())
				nPort = std::stoi(right.data());
			else if (left.caseCompare("HTTP_ROOT") == 0 && !right.empty())
				sHTTProot = right.trim();
			else if (left.caseCompare("PIDFILE") == 0 && !right.empty())
				sPidFile = right.trim();
			else if (left.caseCompare("USER") == 0 && !right.empty())
				usr = right.trim();
			else if (left.caseCompare("GROUP") == 0 && !right.empty())
				grp = right.trim();
			else if (left.caseCompare("LOGFILE") == 0 && !right.empty())
				LogFile = right.trim();
			else if (left.caseCompare("FONTPATH") == 0 && !right.empty())
				FontPath = right.trim();
			else if (left.caseCompare("WEBLOCATION") == 0 && !right.empty())
				web_location = right.trim();
			else if (left.caseCompare("AMXPanelType") == 0 && !right.empty())
				AMXPanelType = right.trim();
			else if (left.caseCompare("AMXController") == 0 && !right.empty())
				AMXController = right.trim();
			else if (left.caseCompare("AMXPort") == 0 && !right.empty())
				AMXPort = std::stoi(right.data());
			else if (left.caseCompare("AMXChannel") == 0 && !right.empty())
			{
				AMXChanel = std::stoi(right.data());

				if (AMXChanel >= 10000 && AMXChanel < 11000)
					AMXChanels.push_back(AMXChanel);
			}
			else if (left.caseCompare("AMXSystem") == 0 && !right.empty())
				AMXSystem = std::stoi(right.data());
			else if (left.caseCompare("SIDEPORT") == 0 && !right.empty())
				sidePort = std::stoi(right.data());
			else if (left.caseCompare("SSHSERVER") == 0 && !right.empty())
				sshServerFile = right.trim();
			else if (left.caseCompare("SSHDH") == 0 && !right.empty())
				sshDHFile = right.trim();
			else if (left.caseCompare("SSHPassword") == 0 && !right.empty())
				sshPassword = right.trim();
			else if (left.caseCompare("WEBSOCKETSERVER") == 0 && !right.empty())
				webSocketServer = right.trim();
			else if (left.caseCompare("HashTablePath") == 0 && !right.empty())
				hashTablePath = right.trim();
			else if (left.caseCompare("WSStatus") == 0 && !right.empty())
			{
				String b = right.trim();

				if (b.compare("0") == 0 || b.caseCompare("FALSE") == 0 ||
					b.caseCompare("NO") == 0 || b.caseCompare("OFF") == 0)
					wsStatus = false;
			}
			else if (left.caseCompare("AllowedNets") == 0 && !right.empty())
			{
				parseNets(right);
			}
			else if (left.caseCompare("DEBUG") == 0 && !right.empty())
			{
				String b = right.trim();

				if (b.compare("1") == 0 || b.caseCompare("TRUE") == 0 ||
					b.caseCompare("YES") == 0 || b.caseCompare("ON") == 0)
					Debug = true;
			}
		}
	}

	fs.close();
	this->fflag = true;
	initialized = true;
}

std::vector<String>& Config::getHashTable(const String& path)
{
	sysl->TRACE(String("Config::getHashTable(const String& path)"));

	hashTable.clear();
	std::ifstream fl;

	try
	{
		fl.open(path.data(), std::fstream::in);
	}
	catch (const std::fstream::failure e)
	{
		sysl->errlog(String("Error on file ")+path+": "+e.what());
		throw rConfig;
	}

	for (std::string line; std::getline(fl, line);)
	{
		String ln = line;
		hashTable.push_back(ln);
	}

	fl.close();
	return hashTable;
}

void Config::parseNets(String& nets)
{
	sysl->TRACE(std::string("Config::parseNets(String& nets)"));

	std::vector<String> parts = nets.split(',');

	for (size_t i = 0; i < parts.size(); i++)
	{
		CIDR *addrs = cidr_from_str(parts[i].trim().data());

		if (addrs == 0)
			continue;

		allowedNet.push_back(*addrs);
		cidr_free(addrs);
	}
}

bool Config::isAllowedNet(String& net)
{
	sysl->TRACE(std::string("Config::isAllowedNet(String& net)"));

	CIDR *addr = cidr_from_str(net.data());

	if (addr == 0)
	{
		sysl->errlog(std::string("Config::isAllowedNet: No or invalid IP address was given!"));
		return false;
	}

	for (size_t i = 0; i < allowedNet.size(); i++)
	{
		CIDR ad = allowedNet.at(i);
		int protoBig = cidr_get_proto(&ad);
		int protoSmall = cidr_get_proto(addr);

		if (protoBig != protoSmall)
		{
			if (protoBig == CIDR_IPV4)
			{
				char *a = cidr_to_str(&ad, CIDR_USEV6);
				CIDR *ac = cidr_from_str(a);
				free (a);
				ad = *ac;
				cidr_free(ac);
			}
			else
			{
				char *a = cidr_to_str(addr, CIDR_USEV6);
				CIDR *ac = cidr_from_str(a);
				free (a);
				cidr_free(addr);
				addr = ac;
			}
		}

		int stat = cidr_contains(&ad, addr);

		if (stat == 0)
		{
			cidr_free (addr);
			return true;
		}
		else
		{
			switch (errno)
			{
				case 0: break;
				case EFAULT:	sysl->errlog(std::string("Config::isAllowedNet: A NULL parameter was passed to method!")); break;
				case EINVAL:	sysl->errlog(std::string("Config::isAllowedNet: Invalid argument passed!")); break;
				case ENOENT:	sysl->errlog(std::string("Config::isAllowedNet: Internal error!")); break;
				case EPROTO:	sysl->errlog(std::string("Config::isAllowedNet: Protocolls don't match!")); break;
				default:
					sysl->errlog(std::string("Config::isAllowedNet: Unknown error!"));
			}
		}
	}

	cidr_free(addr);
	return false;
}

Config::~Config()
{
//	sysl->TRACE(Syslog::EXIT, std::string("Config::Config()"));

	if (this->fflag)
	{
		this->fflag = false;
		fs.close();
	}
}
