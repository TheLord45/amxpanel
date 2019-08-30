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
#include <iostream>
#include <locale>
#include <cstring>
#include <libxml++/libxml++.h>
#include <libxml++/parsers/textreader.h>
#include <glibmm.h>
#ifdef __APPLE__
   #include <boost/asio.hpp>
#else
   #include <asio.hpp>
#endif
#include "config.h"
#include "syslog.h"
#include "nameformat.h"
#include "trace.h"
#include "str.h"
#include "map.h"

extern Syslog *sysl;
extern Config *Configuration;

using namespace std;
using namespace amx;

Map::Map(const std::string& rFile)
	: fname(rFile)
{
	sysl->TRACE(Syslog::ENTRY, std::string("Map::Map()"));
	done = false;
	parse();
}

Map::~Map()
{
	sysl->TRACE(Syslog::EXIT, std::string("Map::~Map()"));
}

void Map::parse()
{
	DECL_TRACER("Map::parse()");

	if (done)
		return;

	string uri;
	uri.append(Configuration->getHTTProot());
	uri.append("/");
	uri.append(fname);
	sysl->TRACE("Map::parse: Reading file: "+uri);

	try
	{
		ifstream xml(uri.c_str(), ifstream::binary);

		if (!xml)
		{
			sysl->errlog("Map::parse: Error opening the file "+uri);
			return;
		}

		string buffer;

		try
		{
			xml.seekg(0, xml.end);
			size_t length = xml.tellg();
			xml.seekg(0, xml.beg);
			buffer.resize(length, ' ');
			char *begin = &*buffer.begin();
			xml.read(begin, length);
		}
		catch (exception& e)
		{
			sysl->errlog(string("Map::parse: Error reading a file: ")+e.what());
			xml.close();
			return;
		}

		xml.close();
		sysl->TRACE("Map::parse: length of file: "+to_string(buffer.length()));
		// Convert from CP1250 (Windblows) to UTF-8.
		string cbuf = NameFormat::cp1250ToUTF8(buffer);
		buffer.clear();

		xmlpp::TextReader reader((const unsigned char *)cbuf.c_str(), cbuf.length());
		bool cm = false;
		bool am = false;
		bool lm = false;
		bool bm = false;
		bool im = false;
		bool sm = false;
		bool strm = false;
		bool pm = false;
		bool me = false;
		std::string lastName;

		while (reader.read())
		{
			Str name(reader.get_name().raw());

			if (name.caseCompare("cm") == 0 && !cm)
				cm = true;
			else if (name.caseCompare("cm") == 0 && cm)
				cm = false;

			if (name.caseCompare("am") == 0 && !am)
				am = true;
			else if (name.caseCompare("am") == 0 && am)
				am = false;

			if (name.caseCompare("lm") == 0 && !lm)
				lm = true;
			else if (name.caseCompare("lm") == 0 && lm)
				lm = false;

			if (name.caseCompare("bm") == 0 && !bm)
				bm = true;
			else if (name.caseCompare("bm") == 0 && bm)
				bm = false;

			if (name.caseCompare("im") == 0 && !im)
				im = true;
			else if (name.caseCompare("im") == 0 && im)
				im = false;

			if (name.caseCompare("sm") == 0 && !sm)
				sm = true;
			else if (name.caseCompare("sm") == 0 && sm)
				sm = false;

			if (name.caseCompare("strm") == 0 && !strm)
				strm = true;
			else if (name.caseCompare("strm") == 0 && strm)
				strm = false;

			if (name.caseCompare("pm") == 0 && !pm)
				pm = true;
			else if (name.caseCompare("pm") == 0 && pm)
				pm = false;

			if (name.caseCompare("me") == 0 && !me)
			{
				me = true;

				if (cm || am || lm || strm)
				{
					MAP_T map;
					map.ax = 0;
					map.bt = 0;
					map.c = 0;
					map.p = 0;
					map.pg = 0;

					if (cm)
						maps.map_cm.push_back(map);
					else if (am)
						maps.map_am.push_back(map);
					else if (lm)
						maps.map_lm.push_back(map);
					else if (strm)
						maps.map_strm.push_back(map);
				}
				else if (bm && im)
				{
					MAP_BM_T map;
					map.id = 0;
					map.bt = 0;
					map.id = 0;
					map.pg = 0;
					map.rt = 0;
					map.sl = 0;
					map.st = 0;
					maps.map_bm.push_back(map);
				}
				else if (pm)
				{
					MAP_PM_T map;
					map.a = 0;
					map.pg = 0;
					map.bt = 0;
					maps.map_pm.push_back(map);
				}
			}
			else if (name.caseCompare("me") == 0 && me)
				me = false;

			if (name.get().compare("#text") == 0)
				name.set(lastName);

			if (reader.has_value() && reader.get_value().raw().find("\n") != string::npos)
				continue;

			if (cm && me && reader.has_value())
			{
				if (name.caseCompare("p") == 0)
					maps.map_cm.back().p = atoi(reader.get_value().c_str());
				else if (name.caseCompare("c") == 0)
					maps.map_cm.back().c = atoi(reader.get_value().c_str());
				else if (name.caseCompare("pg") == 0)
					maps.map_cm.back().pg = atoi(reader.get_value().c_str());
				else if (name.caseCompare("bt") == 0)
					maps.map_cm.back().bt = atoi(reader.get_value().c_str());
				else if (name.caseCompare("pn") == 0)
					maps.map_cm.back().pn = reader.get_value().raw();
				else if (name.caseCompare("bn") == 0)
					maps.map_cm.back().bn = reader.get_value().raw();
			}
			else if (am && me && reader.has_value())
			{
				if (name.caseCompare("p") == 0)
					maps.map_am.back().p = atoi(reader.get_value().c_str());
				else if (name.caseCompare("c") == 0)
					maps.map_am.back().c = atoi(reader.get_value().c_str());
				else if (name.caseCompare("pg") == 0)
					maps.map_am.back().pg = atoi(reader.get_value().c_str());
				else if (name.caseCompare("bt") == 0)
					maps.map_am.back().bt = atoi(reader.get_value().c_str());
				else if (name.caseCompare("pn") == 0)
					maps.map_am.back().pn = reader.get_value().raw();
				else if (name.caseCompare("bn") == 0)
					maps.map_am.back().bn = reader.get_value().raw();
			}
			else if (lm && me && reader.has_value())
			{
				if (name.caseCompare("p") == 0)
					maps.map_lm.back().p = atoi(reader.get_value().c_str());
				else if (name.caseCompare("c") == 0)
					maps.map_lm.back().c = atoi(reader.get_value().c_str());
				else if (name.caseCompare("ax") == 0)
					maps.map_lm.back().ax = atoi(reader.get_value().c_str());
				else if (name.caseCompare("pg") == 0)
					maps.map_lm.back().pg = atoi(reader.get_value().c_str());
				else if (name.caseCompare("bt") == 0)
					maps.map_lm.back().bt = atoi(reader.get_value().c_str());
				else if (name.caseCompare("pn") == 0)
					maps.map_lm.back().pn = reader.get_value().raw();
				else if (name.caseCompare("bn") == 0)
					maps.map_lm.back().bn = reader.get_value().raw();
			}
			else if (bm && im && me && reader.has_value())
			{
				if (name.caseCompare("i") == 0)
					maps.map_bm.back().i = reader.get_value().raw();
				else if (name.caseCompare("rt") == 0)
					maps.map_bm.back().rt = atoi(reader.get_value().c_str());
				else if (name.caseCompare("pg") == 0)
					maps.map_bm.back().pg = atoi(reader.get_value().c_str());
				else if (name.caseCompare("bt") == 0)
					maps.map_bm.back().bt = atoi(reader.get_value().c_str());
				else if (name.caseCompare("st") == 0)
					maps.map_bm.back().st = atoi(reader.get_value().c_str());
				else if (name.caseCompare("sl") == 0)
					maps.map_bm.back().sl = atoi(reader.get_value().c_str());
				else if (name.caseCompare("pn") == 0)
					maps.map_bm.back().pn = reader.get_value().raw();
				else if (name.caseCompare("bn") == 0)
					maps.map_bm.back().bn = reader.get_value().raw();
			}
			else if (sm && me && reader.has_value())
			{
				if (name.caseCompare("i") == 0)
					maps.map_sm.push_back(reader.get_value().raw());
			}
			else if (strm && me)
			{
				if (name.caseCompare("p") == 0)
					maps.map_strm.back().p = atoi(reader.get_value().c_str());
				else if (name.caseCompare("c") == 0)
					maps.map_strm.back().c = atoi(reader.get_value().c_str());
				else if (name.caseCompare("pg") == 0)
					maps.map_strm.back().pg = atoi(reader.get_value().c_str());
				else if (name.caseCompare("bt") == 0)
					maps.map_strm.back().bt = atoi(reader.get_value().c_str());
				else if (name.caseCompare("pn") == 0)
					maps.map_strm.back().pn = reader.get_value().raw();
				else if (name.caseCompare("bn") == 0)
					maps.map_strm.back().bn = reader.get_value().raw();
			}
			else if (pm && me && reader.has_value())
			{
				if (name.caseCompare("a") == 0)
					maps.map_pm.back().a = atoi(reader.get_value().c_str());
				else if (name.caseCompare("t") == 0)
					maps.map_pm.back().t = reader.get_value().raw();
				else if (name.caseCompare("pg") == 0)
					maps.map_pm.back().pg = atoi(reader.get_value().c_str());
				else if (name.caseCompare("bt") == 0)
					maps.map_pm.back().bt = atoi(reader.get_value().c_str());
				else if (name.caseCompare("pn") == 0)
					maps.map_pm.back().pn = reader.get_value().raw();
				else if (name.caseCompare("bn") == 0)
					maps.map_pm.back().bn = reader.get_value().raw();
			}

			if (reader.get_value().raw().compare("#text") != 0)
				lastName = name.get();
		}

		reader.close();
	}
	catch (xmlpp::internal_error& e)
	{
		sysl->errlog(string("Map::parse: XML parser error: ") + e.what());
		done = false;
		return;
	}
	catch (exception& e)
	{
		sysl->errlog(string("Map::parse: XML fatal error: ") + e.what());
		done = false;
		return;
	}

	done = true;
}


