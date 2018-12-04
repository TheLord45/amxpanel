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

#ifndef _DAEMONIZE_H_
#define _DAEMONIZE_H_

#include <string>

class Daemonize
{
	public:
		Daemonize() {}
		~Daemonize();

		void daemon_start (bool ignsigcld);
		void changeToUser(const std::string &usr, const std::string &grp);
};

#endif

