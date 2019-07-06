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
#include <iostream>
#include <cstring>
#include <thread>
#include <atomic>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <pwd.h>
#include <grp.h>
#include "syslog.h"
#include "daemonize.h"
#include "config.h"

using namespace std;

extern Config *Configuration;
extern string pName;
extern Syslog *sysl;
extern std::atomic<bool> killed;

#ifndef SIGCLD
#   define SIGCLD SIGCHLD
#endif

void sig_child (int x);
void sig_handler(int sig);

Daemonize::Daemonize()
{
	sysl->TRACE(Syslog::ENTRY, std::string("Daemonize::Daemonize()"));
}

/*
 * Detach application from console and make it a daemon.
 */
void Daemonize::daemon_start (bool ignsigcld)
{
	sysl->TRACE(std::string("Daemonize::daemon_start (bool ignsigcld)"));

	int childpid, fd;

	if (getpid () == 1)
		goto out;

#ifdef SIGTTOU
	signal (SIGTTOU, SIG_IGN);
#endif
#ifdef SIGTTIN
	signal (SIGTTIN, SIG_IGN);
#endif
#ifdef SIGTSTP
	signal (SIGTSTP, SIG_IGN);
#endif

	if ((childpid = fork ()) < 0)
		sysl->errlog(string("Can't fork this child"));
	else if (childpid > 0)
		exit (0);            /* Parent */

    if (setpgrp () == -1)
        sysl->errlog(string("Can't change process group"));

    signal (SIGHUP, SIG_IGN);

	if ((childpid = fork ()) < 0)
		sysl->errlog (string("Can't fork second child"));
	else if (childpid > 0)
		exit (0);            /* first child */

	/* second child */
out:

	for (fd = 0; fd < NOFILE; fd++)
		close (fd);

	errno = 0;
	chdir ("/");
	umask (0);
#ifdef SIGCLD
	if (ignsigcld)
		signal (SIGCLD, SIG_IGN);
	else
		signal (SIGCLD, &sig_child);
#endif

	// Define a signal handler for terminate signals
	if (signal(SIGTERM, sig_handler) == SIG_ERR)
		sysl->warnlog(string("Can't catch signal SIGTERM!"));

	ofstream of;
	/* Create PID file */
	of.open(Configuration->getPidFile().data(), ofstream::out | ofstream::binary | ofstream::trunc);

	if (!of.is_open())
	{
		sysl->warnlog(string("Can't create PID file ") + Configuration->getPidFile().toString() + ": " + strerror(errno));
		return;
	}

	of << getpid();
	of.close();
}

void Daemonize::changeToUser(const std::string &usr, const std::string &grp)
{
	sysl->TRACE(std::string("Daemonize::changeToUser(const std::string &usr, const std::string &grp)"));
	gid_t gr_gid;

	if (!usr.empty())
	{
		/* get uid */
		struct passwd *userpwd;
		struct group *usergrp = NULL;

		if ((userpwd = getpwnam(usr.c_str())) == NULL)
		{
			sysl->errlog("no such user: "+usr);
			exit(EXIT_FAILURE);
		}

		if (!grp.empty() && (usergrp = getgrnam(grp.c_str())) == NULL)
		{
			if (!grp.empty())
				sysl->errlog("no such group: "+grp);

			gr_gid = userpwd->pw_gid;
		}
		else if (usergrp != NULL)
			gr_gid = usergrp->gr_gid;
		else
			gr_gid = userpwd->pw_gid;

		if (setgid(gr_gid) == -1)
		{
			sysl->errlog("cannot setgid of user "+usr+": "+std::string(strerror(errno)));
			return;
//			exit(EXIT_FAILURE);
		}

#ifdef _BSD_SOURCE
		/* init suplementary groups
		 * (must be done before we change our uid)
		 */
		if (initgroups(usr, gr_gid) == -1)
			sysl->errlog("cannot init suplementary groups of user "+usr+": "+std::string(strerror(errno)));
#endif

		/* set uid */
		if (setuid(userpwd->pw_uid) == -1)
		{
			sysl->errlog("cannot change to uid of user "+usr+": "+std::string(strerror(errno)));
			return;
//			exit(EXIT_FAILURE);
		}

		if (userpwd->pw_dir)
		{
			setenv("HOME", userpwd->pw_dir, 1);
			Configuration->setHOME(userpwd->pw_dir);
			Configuration->Initialize();
		}
	}
}

Daemonize::~Daemonize()
{
	sysl->TRACE(Syslog::EXIT, std::string("Daemonize::Daemonize()"));
}

void sig_child (int /* x */)
{
	sysl->TRACE(std::string("sig_child(int)"));

#if defined(BSD) && !defined(sinix) && !defined(Linux)
	int pid;
	int status;

	while ((pid = wait4 (0, &status, WNOHANG, (struct rusage *)0)) > 0)
		sleep(1);
#endif
}

/*
 * This is the signal handler who disconnects from network and terminates
 * this daemon
 */
void sig_handler(int sig)
{
	sysl->TRACE(std::string("sig_handler(int sig) [sig=")+std::to_string(sig)+"]");

	if (sig == SIGTERM || sig == SIGKILL)
	{
		sysl->log(Syslog::INFO, "Terminating program! Killed by signal " + std::to_string(sig));
		killed = true;
		sleep(3);
		exit(0);
	}
}
