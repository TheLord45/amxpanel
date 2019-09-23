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
#include <string>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <bitset>
#include <iostream>
#include <vector>
#include <unistd.h>
#include "config.h"
#include "syslog.h"
#include "str.h"
#include "trace.h"

using namespace std;

extern string pName;
extern Syslog *sysl;
runtime_error rConfig("Error opening a file");

Config::Config()
{
	sysl->TRACE(Syslog::ENTRY, string("Config::Config()"));
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
	catch (const exception e)
	{
		sysl->errlog(string("Error reading config file ")+sFileName+": "+e.what());
	}
}

void Config::init()
{
	DECL_TRACER("Config::init()");

	if (!(HOME = getenv("HOME")))
	{
		sysl->errlog(string("Error getting environment variable HOME!"));
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
			sysl->errlog(string("Error: Can't find any configuration file!"));
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

void Config::readConfig(const string &sFile)
{
	DECL_TRACER("Config::readConfig(const string &sFile)");

	if (this->fflag)
	{
		this->fflag = false;
		initialized = false;
	}

	if (fs.is_open())
		fs.close();

	try
	{
		fs.open(sFile.c_str(), fstream::in);
	}
	catch (const fstream::failure e)
	{
		sysl->errlog(string("Error on file ")+sFile+": "+e.what());
		throw rConfig;
	}

	sysl->log(Syslog::INFO, "Config::readConfig: Reading from: "+sFile);

	for (string line; getline(fs, line);)
	{
		size_t pos;

		if ((pos = line.find("#")) != string::npos)
		{
			if (pos == 0)
				line.clear();
			else
				line = line.substr(0, pos);
		}

		if (line.empty() || line.find("=") == string::npos)
			continue;

		vector<string> parts = Str(line).split('=');

		if (parts.size() == 2)
		{
			string left = parts[0];
			string right = Str::trim(parts[1]);

			sysl->log(Syslog::INFO, "Config::readConfig: "+left+" : "+right);

			if (Str::caseCompare(left, "LISTEN") == 0 && !right.empty())
				sListen = right;
			else if (Str::caseCompare(left, "PORT") == 0 && !right.empty())
				nPort = stoi(right.c_str());
			else if (Str::caseCompare(left, "HTTP_ROOT") == 0 && !right.empty())
				sHTTProot = right;
			else if (Str::caseCompare(left, "PIDFILE") == 0 && !right.empty())
				sPidFile = right;
			else if (Str::caseCompare(left, "USER") == 0 && !right.empty())
				usr = right;
			else if (Str::caseCompare(left, "GROUP") == 0 && !right.empty())
				grp = right;
			else if (Str::caseCompare(left, "LOGFILE") == 0 && !right.empty())
				LogFile = right;
			else if (Str::caseCompare(left, "FONTPATH") == 0 && !right.empty())
				FontPath = right;
			else if (Str::caseCompare(left, "WEBLOCATION") == 0 && !right.empty())
				web_location = right;
			else if (Str::caseCompare(left, "AMXPanelType") == 0 && !right.empty())
				AMXPanelType = right;
			else if (Str::caseCompare(left, "AMXController") == 0 && !right.empty())
				AMXController = right;
			else if (Str::caseCompare(left, "AMXPort") == 0 && !right.empty())
				AMXPort = stoi(right.c_str());
			else if (Str::caseCompare(left, "AMXChannel") == 0 && !right.empty())
			{
				AMXChanel = stoi(right.c_str());

				if (AMXChanel >= 10000 && AMXChanel < 11000)
					AMXChanels.push_back(AMXChanel);
			}
			else if (Str::caseCompare(left, "AMXSystem") == 0 && !right.empty())
				AMXSystem = stoi(right.c_str());
			else if (Str::caseCompare(left, "SIDEPORT") == 0 && !right.empty())
				sidePort = stoi(right.c_str());
			else if (Str::caseCompare(left, "SSHSERVER") == 0 && !right.empty())
				sshServerFile = right;
			else if (Str::caseCompare(left, "SSHDH") == 0 && !right.empty())
				sshDHFile = right;
			else if (Str::caseCompare(left, "SSHPassword") == 0 && !right.empty())
				sshPassword = right;
			else if (Str::caseCompare(left, "WEBSOCKETSERVER") == 0 && !right.empty())
				webSocketServer = right;
			else if (Str::caseCompare(left, "HashTablePath") == 0 && !right.empty())
				hashTablePath = right;
			else if (Str::caseCompare(left, "WSStatus") == 0 && !right.empty())
			{
				string b = right;

				if (b.compare("0") == 0 || Str::caseCompare(b, "FALSE") == 0 ||
					Str::caseCompare(b, "NO") == 0 || Str::caseCompare(b, "OFF") == 0)
					wsStatus = false;
			}
			else if (Str::caseCompare(left, "AllowedNets") == 0 && !right.empty())
			{
				parseNets(right);
			}
			else if (Str::caseCompare(left, "DEBUG") == 0 && !right.empty())
			{
				string b = right;

				if (b.compare("1") == 0 || Str::caseCompare(b, "TRUE") == 0 ||
					Str::caseCompare(b, "YES") == 0 || Str::caseCompare(b, "ON") == 0)
					Debug = true;
			}
		}
	}

	fs.close();
	this->fflag = true;
	initialized = true;
}

vector<string>& Config::getHashTable(const string& path)
{
	DECL_TRACER("Config::getHashTable(const string& path)");

	hashTable.clear();
	ifstream fl;

	try
	{
		fl.open(path.c_str(), fstream::in);
	}
	catch (const fstream::failure e)
	{
		sysl->errlog("Error on file "+path+": "+e.what());
		throw rConfig;
	}

	for (string line; getline(fl, line);)
	{
		string ln = line;
		hashTable.push_back(ln);
	}

	fl.close();
	return hashTable;
}

void Config::parseNets(string& nets)
{
	DECL_TRACER("Config::parseNets(string& nets)");

	vector<string> parts = Str::split(nets, ',');

	for (size_t i = 0; i < parts.size(); i++)
	{
		CIDR *addrs = cidr_from_str(Str::trim(parts[i]).c_str());

		if (addrs == 0)
			continue;

		allowedNet.push_back(*addrs);
		cidr_free(addrs);
	}
}

bool Config::isAllowedNet(string& net)
{
	DECL_TRACER("Config::isAllowedNet(string& net)");

	CIDR *addr = cidr_from_str(net.data());

	if (addr == 0)
	{
		sysl->errlog(string("Config::isAllowedNet: No or invalid IP address was given!"));
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
				case EFAULT:	sysl->errlog(string("Config::isAllowedNet: A NULL parameter was passed to method!")); break;
				case EINVAL:	sysl->errlog(string("Config::isAllowedNet: Invalid argument passed!")); break;
				case ENOENT:	sysl->errlog(string("Config::isAllowedNet: Internal error!")); break;
				case EPROTO:	sysl->errlog(string("Config::isAllowedNet: Protocolls don't match!")); break;
				default:
					sysl->errlog(string("Config::isAllowedNet: Unknown error!"));
			}
		}
	}

	cidr_free(addr);
	return false;
}

Config::~Config()
{
//	sysl->TRACE(Syslog::EXIT, string("Config::Config()"));

	if (this->fflag)
	{
		this->fflag = false;
		fs.close();
	}
}
