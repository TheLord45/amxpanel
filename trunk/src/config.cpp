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
	sysl->TRACE(String("Config::parseNets(String& nets)"));

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
	sysl->TRACE(String("Config::isAllowedNet(String& net)"));

	CIDR *addr = cidr_from_str(net.data());

	if (addr == 0)
	{
		sysl->errlog(String("Config::isAllowedNet:"));
	}

	for (size_t i = 0; i < allowedNet.size(); i++)
	{
		CIDR ad = allowedNet.at(i);

		if (cidr_equals(addr, &ad) == 0)
		{
			cidr_free(addr);
			return true;
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
