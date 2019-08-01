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

#ifndef __TRACE_H__
#define __TRACE_H__

#include "syslog.h"

#define DECL_TRACER(msg)\
	Trace _hidden_tracer(msg, __FILE__, __LINE__);

#define TRACER(msg)\
	sysl->TRACE(msg);

class Trace
{
	public:
		Trace(const std::string& msg, const char *fname, const int line);
		~Trace();

	private:
		char *getFName(char *buf, size_t len);

		std::string message;
		const char *mFileName;
		int mLine;
};

#endif
