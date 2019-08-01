/*
 * Copyright (C) 2019 by Andreas Theofilu <andreas@theosys.at>
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
#include <string.h>
#include "trace.h"
#include "str.h"

extern Syslog *sysl;

using namespace std;

Trace::Trace(const std::string& msg, const char* fname, const int line)
			: message(msg),
			  mFileName(fname),
			  mLine(line)
{
	if (sysl == 0)
		return;

	char buf[64];
	sysl->TRACE(Syslog::ENTRY, getFName(buf, sizeof(buf))+msg);
}

Trace::~Trace()
{
	if (sysl == 0)
		return;

	char buf[64];
	sysl->TRACE(Syslog::EXIT, getFName(buf, sizeof(buf))+string(" ")+message);
}

char *Trace::getFName(char* buf, size_t len)
{
	int pos = 0, i = 0;

	while (*(mFileName+i) != 0)
	{
		if (*(mFileName+i) == '/')
			pos = i;

		i++;
	}

	char nm[16];
	memset(nm, 0, sizeof(nm));
	strncpy(nm, mFileName+pos+1, 15);
	snprintf(buf, len, "%s: %d: ", nm, mLine);
	return buf;
}
