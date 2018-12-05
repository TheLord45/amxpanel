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

#ifndef __SYSLOG_H_
#define __SYSLOG_H_

#include <cstdlib>
#include <string>
#include <sstream>
#include <syslog.h>
#include "strings.h"

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
		void log(Level l, const strings::String& str) { log(l, str.toString()); }
		void errlog(const std::string& str) const;
		void errlog(const strings::String& str) const { errlog(str.toString()); }
		void warnlog(const std::string& str) const;
		void warnlog(const strings::String& str) const { warnlog(str.toString()); }
		void log_serial(Level l, const std::string& str);
		void log_serial(Level l, const strings::String& str) { log_serial(l, str.toString()); }
		void setPriority(Priority p);
		void setOption(Option o);
		void setDebug(bool d) { debug = d; }
		void setLogFile(const std::string& lf) { LogFile = lf; }
		void setLogFile(const strings::String& lf) { LogFile = lf.toString(); }

		void DebugMsg(const std::string& msg)
		{
			if (debug)
				log(IDEBUG, msg);
		}

		void DebugMsg(const strings::String& msg) { DebugMsg(msg.toString()); }
		void TRACE(FUNCTION f, const std::string& msg);
		void TRACE(FUNCTION f, const strings::String& msg) { TRACE(f, msg.toString()); }

	private:
		void writeToFile(const std::string& str);
		void close();
		void cclose() const;

		bool fflag;			// true = log is open
		bool debug;
		std::string LogFile;
		const std::string pname;
		Priority priority;
		Option option;
		std::ostringstream _ibuf;
		int deep;
};

#endif

