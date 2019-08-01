/*
 * Copyright (C) 2015 by Andreas Theofilu <andreas@theosys.at>
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
	lastFileError = false;
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

void Syslog::log(Level l, const std::string& str)
{
	if (debug && l == IDEBUG && !LogFile.empty())
	{
		writeToFile(str);
		return;
	}
	else if (!LogFile.empty())
		appendToFile(l, str);

	if (!fflag)
	{
		openlog(pname.c_str(), option, priority);
		fflag = true;
	}

	syslog(l, "%s", str.c_str());
	close();
}

void Syslog::log(Syslog::Level l, const std::string& str) const
{
	Syslog *my = const_cast<Syslog *>(this);
	my->log(l, str);
}

void Syslog::errlog(const std::string& str)
{
	if (!fflag)
	{
		openlog(pname.c_str(), option, priority);
		fflag = true;
	}

	appendToFile(ERR, str);
	syslog(LOG_ERR, "%s", str.c_str());
	close();
}

void Syslog::errlog(const std::string& str) const
{
	Syslog *my = const_cast<Syslog *>(this);
	my->errlog(str);
}

void Syslog::warnlog(const std::string& str)
{
	if (!fflag)
	{
		openlog(pname.c_str(), option, priority);
		fflag = true;
	}

	appendToFile(WARNING, str);
	syslog(LOG_WARNING, "%s", str.c_str());
	close();
}

void Syslog::warnlog(const std::string& str) const
{
	Syslog *my = const_cast<Syslog *>(this);
	my->warnlog(str);
}

void Syslog::log_serial(Level l, const std::string& str)
{
	if (!debug && l == IDEBUG)
		return;

	if (!fflag)
	{
		openlog(pname.c_str(), option, priority);
		fflag = true;
	}

	appendToFile(l, str);
	syslog(l, "%s", str.c_str());
}

void Syslog::log_serial(Syslog::Level l, const std::string& str) const
{
	Syslog *my = const_cast<Syslog *>(this);
	my->log_serial(l, str);
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
	if (!debug || lastFileError)
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
		lastFileError = true;
		std::string err = e.what();
		errlog("Syslog::writeToFile: "+err);
	}
}

void Syslog::appendToFile(Level l, const std::string& str)
{
	if (!debug || lastFileError || LogFile.empty() || str.empty())
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
		lastFileError = true;
		std::string err = e.what();
		errlog("Syslog::appendToFile: "+err);
	}
}

void Syslog::TRACE(FUNCTION f, const std::string& msg)
{
	std::string s;

	if (f == EXIT && deep > 0)
		deep--;

	for (int i = 0; i < deep; i++)
		s += " ";

	if (f == ENTRY)
	{
		s += "{ Entry: ";
		deep++;
	}
	else if (f == EXIT)
		s += "} Exit: ";

	DebugMsg(s+msg);
}

