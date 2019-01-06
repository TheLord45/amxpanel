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
#include <iostream>
#include <locale>
#include <iomanip>
#include <libxml++/libxml++.h>
#include <libxml++/parsers/textreader.h>
#include <glibmm.h>
#ifdef __APPLE__
   #include <boost/asio.hpp>
#else
   #include <asio.hpp>
#endif
#include "datetime.h"
#include "config.h"
#include "syslog.h"
#include "palette.h"

extern Syslog *sysl;
extern Config *Configuration;

using namespace std;
using namespace amx;
using namespace strings;

Palette::Palette()
{
	sysl->TRACE(Syslog::ENTRY, String("Palette::Palette()"));
	status = false;
}

Palette::Palette(const strings::String& file)
{
	sysl->TRACE(Syslog::ENTRY, String("Palette::Palette(const strings::String& file)"));
	paletteFile = file;
	parsePalette();
}

bool amx::Palette::parsePalette(const strings::String& f)
{
	sysl->TRACE(String("Palette::parsePalette(const strings::String& f)"));
	paletteFile = f;
	return parsePalette();
}

bool amx::Palette::parsePalette()
{
	sysl->TRACE(String("Palette::parsePalette()"));
	String lastName, at_index, at_name, value;
	status = false;
	String uri = "file://";
	uri.append(Configuration->getHTTProot());
	uri.append("/");
	uri.append(paletteFile);
	sysl->TRACE(String("Palette file: ")+uri);

	try
	{
		xmlpp::TextReader reader(uri.toString());

		while(reader.read())
		{
			String name = string(reader.get_name());

			if (name.at(0) == '#')
				name = lastName;

			if (reader.has_attributes())
			{
				at_index = reader.get_attribute("index");
				at_name = reader.get_attribute("name");
			}

			if (reader.has_value())
			{
				value = reader.get_value();
				value.trim();
			}
			else
				value.clear();

			if (name.caseCompare("color") == 0 && !value.empty() && !at_index.empty())
			{
				sysl->TRACE(String("Palette::parsePalette: Node: ")+name+", Value: "+reader.get_value()+", Attr[0].: "+at_name+", Attr[1].: "+at_index);
				PDATA_T color;
				color.clear();
				String sCol;
				color.index = atoi(at_index.data());
				color.name = at_name;
				sCol = reader.get_value();
				sCol = String("0x")+sCol.substring(1);
				color.color = strtoul(sCol.data(), 0, 16);
				palette.push_back(color);
				at_index.clear();
				at_name.clear();
			}

			lastName = name;
		}

		reader.close();
	}
	catch (xmlpp::internal_error& e)
	{
		sysl->errlog(string("Palette::parsePalette: XML parser error: ")+e.what());
		status = false;
		return status;
	}
	catch (exception& e)
	{
		sysl->errlog(string("Palette::parsePalette: Error: ")+e.what());
		status = false;
		return status;
	}

	sysl->TRACE(String("Palette::parsePalette: Found ")+palette.size()+" colors.");
	status = true;
	return status;
}

Palette::~Palette()
{
    sysl->TRACE(Syslog::EXIT, String("Palette::Palette(...)"));
}

unsigned long amx::Palette::getColor(size_t idx)
{
    sysl->TRACE(Syslog::MESSAGE, std::string("Palette::getColor(size_t idx)"));

    if (idx >= palette.size())
        return 0;

    return palette.at(idx).color;
}

unsigned long amx::Palette::getColor(const strings::String& name)
{
    sysl->TRACE(Syslog::MESSAGE, std::string("Palette::getColor(const strings::String& name)"));

    if (name.at(0) == '#')
    {
        String sCol = String("0x")+name.substring(1);
        return strtoul(sCol.data(), 0, 16);
    }

    for (size_t i = 0; i < palette.size(); i++)
    {
        if (palette[i].name.caseCompare(name) == 0)
            return palette[i].color;
    }

    return 0;
}

String Palette::colorToString(unsigned long col)
{
    sysl->TRACE(Syslog::MESSAGE, std::string("Palette::colorToString(unsigned long col)"));

    std::stringstream stream;
    stream << "#" << std::setfill('0') << std::setw(8) << std::hex << col;
    String sCol(stream.str());
    return sCol;
}
