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

#ifndef __SYSLOG_H_
#define __SYSLOG_H_

#include <cstdlib>
#include <string>
#include <sstream>
#include <syslog.h>

class Syslog
{
	public:
		enum FUNCTION
		{
			ENTRY,
			MESSAGE,
			EXIT
		};

		enum Priority
		{
			KERN = LOG_KERN,
			USER = LOG_USER,
			MAIL = LOG_MAIL,
			DAEMON = LOG_DAEMON,
			AUTH = LOG_AUTH,
			SYSLOG = LOG_SYSLOG,
			LPR = LOG_LPR,
			NEWS = LOG_NEWS,
			UUCP = LOG_UUCP,
			CRON = LOG_CRON,
			AUTHPRIV = LOG_AUTHPRIV,
			FTP = LOG_FTP,
			LOCAL0 = LOG_LOCAL0,
			LOCAL1 = LOG_LOCAL1,
			LOCAL2 = LOG_LOCAL2,
			LOCAL3 = LOG_LOCAL3,
			LOCAL4 = LOG_LOCAL4,
			LOCAL5 = LOG_LOCAL5,
			LOCAL6 = LOG_LOCAL6,
			LOCAL7 = LOG_LOCAL7
		};

		enum Level
		{
			EMERG = LOG_EMERG,
			ALERT = LOG_ALERT,
			CRIT = LOG_CRIT,
			ERR = LOG_ERR,
			WARNING = LOG_WARNING,
			NOTICE = LOG_NOTICE,
			INFO = LOG_INFO,
			IDEBUG = LOG_DEBUG
		};

		enum Option
		{
			CONSOLE = LOG_CONS,
			NDELAY = LOG_NDELAY,
			NOWAIT = LOG_NOWAIT,
			ODELAY = LOG_ODELAY,
			PERROR = LOG_PERROR,
			PID = LOG_PID
		};

	public:
		Syslog(const std::string &name, Priority p = DAEMON, Option o = PID);
		~Syslog();

		void log(Level l, const std::string& str);
		void log(Level l, const std::string& str) const;
		void errlog(const std::string& str);
		void errlog(const std::string& str) const;
		void warnlog(const std::string& str);
		void warnlog(const std::string& str) const;
		void log_serial(Level l, const std::string& str);
		void log_serial(Level l, const std::string& str) const;
		void setPriority(Priority p);
		void setOption(Option o);
		void setDebug(bool d) { debug = d; }
		void setLogFile(const std::string& lf) { LogFile = lf; }

		void DebugMsg(const std::string& msg)
		{
			if (debug)
				log(IDEBUG, msg);
		}

		void TRACE(FUNCTION f, const std::string& msg);
		void TRACE(const std::string& msg) { TRACE(MESSAGE, msg); }

	private:
		void writeToFile(const std::string& str);
		void appendToFile(Level l, const std::string& str);
		void close();

		bool fflag;			// true = log is open
		bool debug;
		std::string LogFile;
		const std::string pname;
		Priority priority;
		Option option;
		std::ostringstream _ibuf;
		int deep;
		bool lastFileError;
};

#endif

