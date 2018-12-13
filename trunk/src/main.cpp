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

#include <string>
#include <unistd.h>
#include "config.h"
#include "syslog.h"
#include "daemonize.h"
#include "server.h"

Config *Configuration;
std::string pName;
Syslog *sysl;

#define VERSION		"1.0"

using namespace strings;

int main(int /* argc */, const char **argv)
{
	std::string prog(*argv);
	size_t pos;

	if ((pos = prog.find_last_of("/")) != std::string::npos)
		pName = prog.substr(pos+1);
	else
		pName = prog;

	sysl = new Syslog(pName);
	Configuration = new Config();
	sysl->setDebug(Configuration->getDebug());
	sysl->setLogFile(Configuration->getLogFile());
	sysl->TRACE(Syslog::ENTRY, std::string("main(int /* argc */, const char **argv)"));
	sysl->log(Syslog::INFO, pName + " v" + VERSION);
	sysl->log(Syslog::INFO, String("(C) Copyright by Andreas Theofilu <andreas@theosys.at>. All rights reserved!"));
	sysl->log(Syslog::INFO, String("Daemon is starting ..."));

	Daemonize daemon;
	daemon.daemon_start(true);
	daemon.changeToUser(Configuration->getUser().toString(), Configuration->getGroup().toString());
	sysl->DebugMsg(String("Starting up Server ..."));
	http::server::Server server(Configuration->getListen().toString(), Configuration->getPort(), Configuration->getHTTProot().toString());
	sysl->log(Syslog::INFO, pName + " v" + VERSION + ": Startup finished. All components should run now.");
	server.run();

	// Upon the previous function exits, clean up end exit.
	sysl->TRACE(Syslog::EXIT, std::string("main(int /* argc */, const char **argv)"));
	delete sysl;
	delete Configuration;
	return 0;
}

