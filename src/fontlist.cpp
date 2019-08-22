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
#include "nameformat.h"
#include "fontlist.h"
#include "str.h"
#include "trace.h"

extern Syslog *sysl;
extern Config *Configuration;

using namespace std;
using namespace amx;

amx::FontList::FontList(const string& file)
{
	sysl->TRACE(Syslog::ENTRY, std::string("FontList::FontList(const string& file)"));
	// Clear the empty font
	emptyFont.number = 0;
	emptyFont.fileSize = 0;
	emptyFont.faceIndex = 0;
	emptyFont.size = 0;
	emptyFont.usageCount = 0;
	fillSysFonts();
	FONT_T font;
	string lastName;
	int fi = 0;
	string uri = "file://";
	uri.append(Configuration->getHTTProot());
	uri.append("/");
	uri.append(file);
	sysl->TRACE("FontList::FontList: Parsing file "+uri);

	try
	{
		xmlpp::TextReader reader(uri);

		while(reader.read())
		{
			Str name(reader.get_name().raw());

			if (name.get().at(0) == '#')
				name.set(lastName);

			if (reader.has_attributes())
				fi = atoi(reader.get_attribute(0).c_str());

			if (name.caseCompare("font") == 0 && reader.has_attributes() && fi != font.number)
			{
				font.number = fi;
				font.file.clear();
				font.fileSize = 0;
				font.faceIndex = 0;
				font.name.clear();
				font.subfamilyName.clear();
				font.fullName.clear();
				font.size = 0;
				font.usageCount = 0;
				fontList.push_back(font);
				sysl->TRACE("FontList::FontList: Added font number: "+to_string(font.number));
			}
			else if (name.caseCompare("file") == 0 && reader.has_value())
				fontList.back().file = reader.get_value().raw();
			else if (name.caseCompare("fileSize") == 0 && reader.has_value())
				fontList.back().fileSize = atoi(reader.get_value().c_str());
			else if (name.caseCompare("faceIndex") == 0 && reader.has_value())
				fontList.back().faceIndex = atoi(reader.get_value().c_str());
			else if (name.caseCompare("name") == 0 && reader.has_value())
				fontList.back().name = reader.get_value().raw();
			else if (name.caseCompare("subfamilyName") == 0 && reader.has_value())
				fontList.back().subfamilyName = reader.get_value().raw();
			else if (name.caseCompare("fullName") == 0 && reader.has_value())
				fontList.back().fullName = reader.get_value().raw();
			else if (name.caseCompare("size") == 0 && reader.has_value())
				fontList.back().size = atoi(reader.get_value().c_str());
			else if (name.caseCompare("usageCount") == 0 && reader.has_value())
				fontList.back().usageCount = atoi(reader.get_value().c_str());

			lastName = name.get();
		}

		reader.close();
		sysl->TRACE("FontList::FontList: Found "+to_string(fontList.size())+" fonts.");
	}
	catch (xmlpp::internal_error& e)
	{
		sysl->errlog(string("FontList::FontList")+e.what());
		status = false;
		return;
	}

	status = true;
}

amx::FontList::~FontList()
{
	sysl->TRACE(Syslog::EXIT, std::string("FontList::FontList(...)"));
}

string amx::FontList::getFontStyles()
{
	DECL_TRACER("FontList::getFontStyles()");

	string styles;
	fontFaces.clear();

	// Fixed system fonts first
	styles += "@font-face {\n";
	styles += "  font-family: \"Courier New\", Courier, monospace;\n";
	styles += "  font-style: normal;\n";
	styles += "  font-weight: normal;\n";
	styles += "}\n";
	styles += "@font-face {\n";
	styles += "  font-family: \"AMX Bold\";\n";
	styles += "  src: url(fonts/amxbold_.ttf);\n";
	styles += "  font-style: normal;\n";
	styles += "  font-weight: bold;\n";
	styles += "}\n";
	styles += "@font-face {\n";
	styles += "  font-family: Arial, Helvetica, sans-serif;\n";
	styles += "  font-style: normal;\n";
	styles += "  font-weight: normal;\n";
	styles += "}\n";
	styles += "@font-face {\n";
	styles += "  font-family: \"Arial Black\", Gadget, sans-serif;\n";
	styles += "  font-style: normal;\n";
	styles += "  font-weight: bold;\n";
	styles += "}\n\n";

	for (size_t i = 0; i < fontList.size(); i++)
	{
		if (fontList[i].number < 32)
			continue;

		string name = fontList[i].name+","+fontList[i].subfamilyName;

		if (fontFaces.size() == 0 || !exist(name))
		{
			fontFaces.push_back(name);
			styles += "@font-face {\n";
			styles += "  font-family: \""+fontList[i].name+"\";\n";
			styles += "  src: url(fonts/"+NameFormat::toURL(fontList[i].file)+");\n";
			styles += "  font-style: "+getFontStyle(fontList[i].subfamilyName)+";\n";
			styles += "  font-weight: "+getFontWeight(fontList[i].subfamilyName)+";\n";
			styles += "}\n";
		}
	}

	return styles;
}

bool FontList::serializeToJson()
{
	DECL_TRACER("FontList::serializeToJson()");

	fstream pgFile;
	string fname = Configuration->getHTTProot()+"/scripts/fonts.js";

	try
	{
		pgFile.open(fname, ios::in | ios::out | ios::trunc | ios::binary);

		if (!pgFile.is_open())
		{
			sysl->errlog("Page::serializeToFile: Error opening file "+fname);
			return false;
		}
	}
	catch (const std::fstream::failure e)
	{
		sysl->errlog(std::string("Page::serializeToFile: I/O Error: ")+e.what());
		return false;
	}

	pgFile << "var fontList = {\"fonts\":[";

	for (size_t i = 0; i < fontList.size(); i++)
	{
		if (i > 0)
			pgFile << ",";

		pgFile << std::endl << "\t{\"name\":\"" << fontList[i].name << "\",\"subfamilyName\":\"" << fontList[i].subfamilyName << "\",";
		pgFile << "\"fullName\":\"" << fontList[i].fullName << "\",";
		pgFile << "\"number\":" << fontList[i].number << ",\"faceIndex\":" << fontList[i].faceIndex << ",";
		pgFile << "\"file\":\"" << fontList[i].file << "\",\"size\":" << fontList[i].size << "}";
	}

	pgFile << std::endl << "]};" << std::endl;
	pgFile.close();
	return true;
}

FONT_T& FontList::findFont(int idx)
{
	DECL_TRACER("FontList::findFont(int idx)");

	for (size_t i = 0; i < fontList.size(); i++)
	{
		if (fontList[i].number == idx)
			return fontList[i];
	}

	sysl->TRACE("FontList::findFont: Font ID "+to_string(idx)+" not found. Have "+to_string(fontList.size())+" fonts in cache.");
	return emptyFont;
}

bool amx::FontList::exist(const string& ff)
{
	DECL_TRACER("FontList::exist(const string& ff)");

	for (size_t i = 0; i < fontFaces.size(); i++)
	{
		if (fontFaces[i].compare(ff) == 0)
			return true;
	}

	return false;
}

string FontList::getFontStyle(const string& fs)
{
	DECL_TRACER("FontList::getFontStyle(const string& fs)");

	if (Str::caseCompare(fs, "Regular") == 0)
		return "normal";

	if (Str::caseCompare(fs, "Italic") == 0 || Str::caseCompare(fs, "Bold Italic") == 0)
		return "italic";

	return "normal";
}

string amx::FontList::getFontWeight(const string& fw)
{
	DECL_TRACER("FontList::getFontWeight(const string& fw)");

	if (Str::caseCompare(fw, "Regular") == 0)
		return "normal";

	if (Str::caseCompare(fw, "Bold") == 0 || Str::caseCompare(fw, "Bold Italic") == 0)
		return "bold";

	return "normal";
}

void amx::FontList::fillSysFonts()
{
	DECL_TRACER("FontList::fillSysFonts()");
	FONT_T font;

	font.number = 1;
	font.faceIndex = 1;
	font.fullName = "Courier New";
	font.name = "Courier New";
	font.size = 9;
	font.subfamilyName = "normal";
	font.fileSize = 0;
	font.usageCount = 0;
	fontList.push_back(font);

	font.number = 2;
	font.faceIndex = 2;
	font.size = 12;
	fontList.push_back(font);

	font.number = 3;
	font.faceIndex = 3;
	font.size = 18;
	fontList.push_back(font);

	font.number = 4;
	font.faceIndex = 4;
	font.size = 26;
	fontList.push_back(font);

	font.number = 5;
	font.faceIndex = 5;
	font.size = 32;
	fontList.push_back(font);

	font.number = 6;
	font.faceIndex = 6;
	font.size = 18;
	fontList.push_back(font);

	font.number = 7;
	font.faceIndex = 7;
	font.size = 26;
	fontList.push_back(font);

	font.number = 8;
	font.faceIndex = 8;
	font.size = 34;
	fontList.push_back(font);

	font.number = 9;
	font.faceIndex = 9;
	font.fullName = "AMX Bold";
	font.name = "AMX Bold";
	font.size = 14;
	font.subfamilyName = "bold";
	fontList.push_back(font);

	font.number = 10;
	font.faceIndex = 10;
	font.size = 20;
	fontList.push_back(font);

	font.number = 11;
	font.faceIndex = 11;
	font.size = 36;
	fontList.push_back(font);

	font.number = 19;
	font.faceIndex = 19;
	font.fullName = "Arial";
	font.name = "Arial";
	font.size = 9;
	font.subfamilyName = "normal";
	fontList.push_back(font);

	font.number = 20;
	font.faceIndex = 20;
	font.size = 10;
	fontList.push_back(font);

	font.number = 21;
	font.faceIndex = 21;
	font.size = 12;
	fontList.push_back(font);

	font.number = 22;
	font.faceIndex = 22;
	font.size = 14;
	fontList.push_back(font);

	font.number = 23;
	font.faceIndex = 23;
	font.size = 16;
	fontList.push_back(font);

	font.number = 24;
	font.faceIndex = 24;
	font.size = 18;
	fontList.push_back(font);

	font.number = 25;
	font.faceIndex = 25;
	font.size = 20;
	fontList.push_back(font);

	font.number = 26;
	font.faceIndex = 26;
	font.size = 24;
	fontList.push_back(font);

	font.number = 27;
	font.faceIndex = 27;
	font.size = 36;
	fontList.push_back(font);

	font.number = 28;
	font.faceIndex = 28;
	font.fullName = "Arial Bold";
	font.name = "Arial Bold";
	font.size = 10;
	font.subfamilyName = "bold";
	fontList.push_back(font);

	font.number = 29;
	font.faceIndex = 29;
	font.size = 8;
	fontList.push_back(font);
}
