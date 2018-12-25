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
	AMXPort = 1319;
	AMXChanel = 0;
	AMXSystem = 1;

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

	if (!access("/etc/hvl.conf", R_OK))
		sFileName = "/etc/hvl.conf";
	else if (!access("/etc/hvl/hvl.conf", R_OK))
		sFileName = "/etc/hvl/hvl.conf";
	else if (!access("/usr/etc/hvl.conf", R_OK))
		sFileName = "/usr/etc/hvl.conf";
	else if (HOME)
	{
		sFileName = HOME;
		sFileName += "/.hvl.conf";

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

	for (std::string line; std::getline(fs, line);)
	{
		String ln = line;
		std::vector<String> parts = ln.split('=');

		if (parts.size() == 2)
		{
			String left = parts[0];
			String right = parts[1];

			left.toUpper();
			sysl->log(Syslog::INFO, String("Config::readConfig: ")+left+" : "+right);

			if (left.compare("LISTEN") == 0 && !right.empty())
				sListen = right.trim();
			else if (left.compare("PORT") == 0 && !right.empty())
				nPort = std::stoi(right.data());
			else if (left.compare("HTTP_ROOT") == 0 && !right.empty())
				sHTTProot = right.trim();
			else if (left.compare("PIDFILE") == 0 && !right.empty())
				sPidFile = right.trim();
			else if (left.compare("USER") == 0 && !right.empty())
				usr = right.trim();
			else if (left.compare("GROUP") == 0 && !right.empty())
				grp = right.trim();
			else if (left.compare("LOGFILE") == 0 && !right.empty())
				LogFile = right.trim();
			else if (left.compare("FONTPATH") == 0 && !right.empty())
				FontPath = right.trim();
			else if (left.compare("WEBLOCATION") == 0 && !right.empty())
				web_location = right.trim();
			else if (left.compare("AMXPanelType") == 0 && !right.empty())
				AMXPanelType = right.trim();
			else if (left.compare("AMXController") == 0 && !right.empty())
				AMXController = right.trim();
			else if (left.compare("AMXPort") == 0 && !right.empty())
				AMXPort = std::stoi(right.data());
			else if (left.compare("AMXChannel") == 0 && !right.empty())
				AMXChanel = std::stoi(right.data());
			else if (left.compare("AMXSystem") == 0 && !right.empty())
				AMXSystem = std::stoi(right.data());
			else if (left.compare("DEBUG") == 0 && !right.empty())
			{
				String b = right.trim();
				b.toUpper();

				if (b.compare("1") == 0 || b.compare("TRUE") == 0 || b.compare("YES") == 0)
					Debug = true;
			}
		}
	}

	fs.close();
	this->fflag = true;
	initialized = true;
}

Config::~Config()
{
	sysl->TRACE(Syslog::EXIT, std::string("Config::Config()"));

	if (this->fflag)
	{
		this->fflag = false;
		fs.close();
	}
}
