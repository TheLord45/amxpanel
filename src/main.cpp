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
#include <utility>
#include <unistd.h>
#include <thread>
#include <exception>
#include "config.h"
#include "syslog.h"
#include "daemonize.h"
#include "touchpanel.h"
#include "websocket.h"

Config *Configuration;
std::string pName;
Syslog *sysl;
amx::TouchPanel *pTouchPanel;
std::atomic<bool> killed;

using namespace std;

int main(int /* argc */, const char **argv)
{
	std::string prog(*argv);
	size_t pos;
	killed = false;

	if ((pos = prog.find_last_of("/")) != std::string::npos)
		pName = prog.substr(pos+1);
	else
		pName = prog;

	sysl = new Syslog(pName);
	Configuration = new Config();
	sysl->setDebug(Configuration->getDebug());
	sysl->setLogFile(Configuration->getLogFile());
	sysl->TRACE(Syslog::ENTRY, "main(int /* argc */, const char **argv)");
	sysl->log(Syslog::INFO, pName + " v" + VERSION);
	sysl->log(Syslog::INFO, "(C) Copyright by Andreas Theofilu <andreas@theosys.at>. All rights reserved!");
	sysl->log(Syslog::INFO, "Daemon is starting ...");

	Daemonize daemon;
	daemon.daemon_start(true);
	daemon.changeToUser(Configuration->getUser(), Configuration->getGroup());
	sysl->log(Syslog::INFO, pName + " v" + VERSION + ": Startup finished. All components should run now.");
	// Create the panel
	pTouchPanel = new amx::TouchPanel();
//	pTouchPanel->parsePages();
	// Start thread for AMX communication
	while (!killed)
	{
		sleep (1);
	}

	// Upon the previous function exits, clean up end exit.
	sysl->TRACE(Syslog::EXIT, "main(int /* argc */, const char **argv)");
	delete pTouchPanel;
	delete sysl;
	delete Configuration;
	return 0;
}

