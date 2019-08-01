/*
 * Copyright (C) 2018 by Andreas Theofilu <andreas@theosys.at>
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
#include "trace.h"
#include "str.h"

extern Syslog *sysl;
extern Config *Configuration;

using namespace std;
using namespace amx;

Palette::Palette()
{
	sysl->TRACE(Syslog::ENTRY, "Palette::Palette()");
	status = false;
}

Palette::Palette(const string& file)
{
	sysl->TRACE(Syslog::ENTRY, "Palette::Palette(const string& file)");
	bool hasFile = false;

	for (size_t i = 0; i < paletteFiles.size(); i++)
	{
		if (paletteFiles[i].compare(file) == 0)
		{
			hasFile = true;
			break;
		}
	}

	if (!hasFile)
		paletteFiles.push_back(file);

	for (size_t i = 0; i < paletteFiles.size(); i++)
		parsePalette(paletteFiles[i]);
}

amx::Palette::Palette(const std::vector<PALETTE_T>& pal)
{
	sysl->TRACE(Syslog::ENTRY, "Palette::Palette(const std::vector<PALETTE_T>& pal)");
	paletteFiles.clear();

	for (size_t i = 0; i < pal.size(); i++)
	{
		paletteFiles.push_back(pal[i].file);
		parsePalette(pal[i].file);
	}
}

Palette::Palette(const std::vector<PALETTE_T>& pal, const string& main)
{
	sysl->TRACE(Syslog::ENTRY, "Palette::Palette(const std::vector<PALETTE_T>& pal, const string& main)");
	paletteFiles.clear();

	sysl->TRACE("Palette::Palette: Have "+to_string(pal.size())+" palettes.");

	for (size_t i = 0; i < pal.size(); i++)
		paletteFiles.push_back(pal[i].file);

	parsePalette(main);		// First parse the main palette

	for (size_t i = 0; i < paletteFiles.size(); i++)
	{
		if (paletteFiles[i].compare(main) != 0)
			parsePalette(paletteFiles[i]);
	}
}

void amx::Palette::setPaletteFile(const string& f)
{
	DECL_TRACER(std::string("Palette::setPaletteFile(const string& f)"));

	bool hasFile = false;

	for (size_t i = 0; i < paletteFiles.size(); i++)
	{
		if (paletteFiles[i].compare(f) == 0)
		{
			hasFile = true;
			break;
		}
	}

	if (!hasFile)
		paletteFiles.push_back(f);
}

bool amx::Palette::parsePalette()
{
	DECL_TRACER(std::string("Palette::parsePalette()"));

	for (size_t i = 0; i < paletteFiles.size(); i++)
	{
		if (!parsePalette(paletteFiles[i]))
			return false;
	}

	return true;
}

bool amx::Palette::parsePalette(const string& f)
{
	DECL_TRACER(std::string("Palette::parsePalette(const string& f)"));
	string lastName, at_index, at_name, value;
	status = false;
	string uri = "file://";
	uri.append(Configuration->getHTTProot());
	uri.append("/");
	uri.append(f);
	sysl->TRACE("Palette file: "+uri);

	try
	{
		xmlpp::TextReader reader(uri);

		while(reader.read())
		{
			Str name(reader.get_name().raw());

			if (name.get().at(0) == '#')
				name.set(lastName);

			if (reader.has_attributes())
			{
				at_index = reader.get_attribute("index");
				at_name = reader.get_attribute("name");
			}

			if (reader.has_value())
			{
				string val = reader.get_value();
				value = Str::trim(val);
			}
			else
				value.clear();

			if (name.caseCompare("color") == 0 && !value.empty() && !at_index.empty())
			{
				sysl->TRACE("Palette::parsePalette: Node: "+name+", Value: "+reader.get_value()+", Attr[0].: "+at_name+", Attr[1].: "+at_index);
				PDATA_T color;
				color.clear();
				string sCol;
				color.index = atoi(at_index.c_str());
				color.name = at_name;
				sCol = reader.get_value();
				sCol = "0x"+sCol.substr(1);
				color.color = strtoul(sCol.c_str(), 0, 16);
				// Add only if we've not already in table
				bool has = false;

				for (size_t i = 0; i < palette.size(); i++)
				{
					if ((!palette[i].name.empty() && palette[i].name.compare(at_name) == 0) ||
						(palette[i].name.empty() && palette[i].color == color.color))
					{
						has = true;
						break;
					}
				}

				if (!has)
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

	sysl->TRACE("Palette::parsePalette: Found "+to_string(palette.size())+" colors.");
	status = true;
	return status;
}

Palette::~Palette()
{
    sysl->TRACE(Syslog::EXIT, "Palette::~Palette(...)");
}

unsigned long Palette::getColor(size_t idx)
{
    DECL_TRACER("Palette::getColor(size_t idx)");

    if (idx >= palette.size())
        return 0;

    return palette.at(idx).color;
}

unsigned long Palette::getColor(const string& name)
{
    DECL_TRACER("Palette::getColor(const string& name)");

    if (name.at(0) == '#')
    {
        string sCol = "0x"+name.substr(1);
        return strtoul(sCol.c_str(), 0, 16);
    }

    for (size_t i = 0; i < palette.size(); i++)
    {
        if (Str::caseCompare(palette[i].name, name) == 0)
            return palette[i].color;
    }

    return 0;
}

string Palette::colorToString(unsigned long col)
{
	DECL_TRACER("Palette::colorToString(unsigned long col)");

	int red, green, blue;
	double alpha;
	char calpha[128];
	string color = "rgba(";

	red = (col >> 24) & 0x000000ff;
	green = (col >> 16) & 0x000000ff;
	blue = (col >> 8) & 0x000000ff;
	alpha = 1.0 / 256.0 * (double)(col & 0x000000ff);
	snprintf(calpha, sizeof(calpha), "%1.2f", alpha);
	color += to_string(red)+", "+to_string(green)+", "+to_string(blue)+", "+calpha+")";
	return color;
}

string Palette::colorToSArray(unsigned long col)
{
	DECL_TRACER("Palette::colorToSArray(unsigned long col)");

	int red, green, blue, alpha;
	string color = "[";

	red = (col >> 24) & 0x000000ff;
	green = (col >> 16) & 0x000000ff;
	blue = (col >> 8) & 0x000000ff;
	alpha = (col & 0x000000ff);
	color += to_string(red)+","+to_string(green)+","+to_string(blue)+","+to_string(alpha)+"]";
	return color;
}

string Palette::getJson()
{
	DECL_TRACER("Palette::getJson()");

	string json = "var palette = {\"colors\":[\n";

	for (size_t i = 0; i < palette.size(); i++)
	{
		int red, green, blue;
		double alpha;
		char calpha[128];

		red = (palette[i].color >> 24) & 0x000000ff;
		green = (palette[i].color >> 16) & 0x000000ff;
		blue = (palette[i].color >> 8) & 0x000000ff;
		alpha = 1.0 / 256.0 * (double)(palette[i].color & 0x000000ff);
		snprintf(calpha, sizeof(calpha), "%1.2f", alpha);

		if (i > 0)
			json += ",\n";

		json += "\t{\"name\":\""+palette[i].name+"\",\"id\":"+to_string(palette[i].index)+",\"red\":"+to_string(red)+",\"green\":"+to_string(green)+",\"blue\":"+to_string(blue)+",\"alpha\":"+calpha+"}";
	}

	json += "\n]};\n";
	return json;
}
