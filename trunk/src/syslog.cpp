/*
 * Copyright (C) 2015 by Andreas Theofilu <andreas@theosys.at>
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

#include <syslog.h>
#include <iostream>
#include <fstream>
#include "syslog.h"
#include "datetime.h"

Syslog::Syslog(const std::string &name, Priority p, Option o)
			: pname(name),
			  priority(p),
			  option(o)
{
	fflag = false;
	LogFile = "";
	deep = 0;
}

Syslog::~Syslog()
{
	if (fflag)
		closelog();
}

void Syslog::close()
{
	if (fflag)
		closelog();
}

void Syslog::cclose() const
{
	if (fflag)
		closelog();
}

void Syslog::log(Level l, const std::string& str)
{
	if (debug && l == IDEBUG && !LogFile.empty())
	{
		writeToFile(str);
		return;
	}
	else
		appendToFile(l, str);

	if (!fflag)
	{
		openlog(pname.c_str(), option, priority);
		fflag = true;
	}

	syslog(l, "%s", str.c_str());
	close();
}

void Syslog::errlog(const std::string& str) const
{
	if (!fflag)
		openlog(pname.c_str(), option, priority);

	appendToFile(ERR, str);
	syslog(LOG_ERR, "%s", str.c_str());
	cclose();
}

void Syslog::warnlog(const std::string& str) const
{
	if (!fflag)
		openlog(pname.c_str(), option, priority);

	appendToFile(WARNING, str);
	syslog(LOG_WARNING, "%s", str.c_str());
	cclose();
}

void Syslog::log_serial(Level l, const std::string& str)
{
	if (!fflag)
	{
		openlog(pname.c_str(), option, priority);
		fflag = true;
	}

	appendToFile(l, str);
	syslog(l, "%s", str.c_str());
}

void Syslog::setPriority(Priority p)
{
	close();
	priority = p;
}

void Syslog::setOption(Option o)
{
	close();
	option = o;
}

void Syslog::writeToFile(const std::string& str)
{
	if (!debug)
		return;

	if (str.empty())
		return;

	DateTime dt;
	std::fstream file;

	try
	{
		file.open(LogFile, std::ios::out | std::ios::app);
		file << dt.toString() << ": " << str << std::endl;
		file.close();
	}
	catch (std::exception& e)
	{
		std::string err = e.what();
		errlog("Syslog::writeToFile: "+err);
	}
}

void Syslog::appendToFile(Level l, const std::string& str)
{
	if (!debug || LogFile.empty() || str.empty())
		return;

	DateTime dt;
	std::fstream file;
	std::string lvl;

	switch (l)
	{
		case EMERG:		lvl = "Emergency"; break;
		case ALERT:		lvl = "Alert"; break;
		case CRIT:		lvl = "Critical"; break;
		case ERR:		lvl = "Error"; break;
		case WARNING:	lvl = "Warning"; break;
		case NOTICE:	lvl = "Notice"; break;
		case INFO:		lvl = "Info"; break;
		case IDEBUG:	lvl = "Debug"; break;
	}

	try
	{
		file.open(LogFile, std::ios::out | std::ios::app);
		file << dt.toString() << ": " << lvl << ": " << str << std::endl;
		file.close();
	}
	catch (std::exception& e)
	{
		std::string err = e.what();
		errlog("Syslog::writeToFile: "+err);
	}
}

void Syslog::appendToFile(Level l, const std::string& str) const
{
	Syslog *my = const_cast<Syslog *>(this);
	my->appendToFile(l, str);
}

void Syslog::TRACE(FUNCTION f, const std::string& msg)
{
	std::string s;

	if (f == EXIT && deep > 0)
		deep--;

	for (int i = 0; i < deep; i++)
		s += "  ";

	if (f == ENTRY)
	{
		s += "{ Entry: ";
		deep++;
	}
	else if (f == EXIT)
		s += "} Exit: ";

	DebugMsg(s+msg);
}

