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

extern Syslog *sysl;
extern Config *Configuration;

using namespace std;
using namespace amx;
using namespace strings;

amx::FontList::FontList(const strings::String& file)
{
	sysl->TRACE(Syslog::ENTRY, std::string("FontList::FontList(const strings::String& file)"));
	// Clear the empty font
	emptyFont.number = 0;
	emptyFont.fileSize = 0;
	emptyFont.faceIndex = 0;
	emptyFont.size = 0;
	emptyFont.usageCount = 0;
	fillSysFonts();
	FONT_T font;
	String lastName;
	int fi = 0;
	String uri = "file://";
	uri.append(Configuration->getHTTProot());
	uri.append("/");
	uri.append(file);

	try
	{
		xmlpp::TextReader reader(uri.toString());

		while(reader.read())
		{
			String name = string(reader.get_name());

			if (name.at(0) == '#')
				name = lastName;

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
				sysl->TRACE(String("FontList::FontList: Added font number: ")+font.number);
			}
			else if (name.caseCompare("file") == 0 && reader.has_value())
				fontList.back().file = reader.get_value();
			else if (name.caseCompare("fileSize") == 0 && reader.has_value())
				fontList.back().fileSize = atoi(reader.get_value().c_str());
			else if (name.caseCompare("faceIndex") == 0 && reader.has_value())
				fontList.back().faceIndex = atoi(reader.get_value().c_str());
			else if (name.caseCompare("name") == 0 && reader.has_value())
				fontList.back().name = reader.get_value();
			else if (name.caseCompare("subfamilyName") == 0 && reader.has_value())
				fontList.back().subfamilyName = reader.get_value();
			else if (name.caseCompare("fullName") == 0 && reader.has_value())
				fontList.back().fullName = reader.get_value();
			else if (name.caseCompare("size") == 0 && reader.has_value())
				fontList.back().size = atoi(reader.get_value().c_str());
			else if (name.caseCompare("usageCount") == 0 && reader.has_value())
				fontList.back().usageCount = atoi(reader.get_value().c_str());

			lastName = name;
		}

		reader.close();
		sysl->TRACE(String("FontList::FontList: Found ")+fontList.size()+" fonts.");
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

strings::String amx::FontList::getFontStyles()
{
	sysl->TRACE(String("FontList::getFontStyles()"));

	String styles;
	fontFaces.clear();

	// Fixed system fonts first
	styles += "@font-face {\n";
	styles += String("  font-family: \"Courier New\", Courier, monospace;\n");
	styles += String("  font-style: normal;\n");
	styles += String("  font-weight: normal;\n");
	styles += "}\n";
	styles += "@font-face {\n";
	styles += String("  font-family: \"AMX Bold\";\n");
	styles += String("  src: url(fonts/amxbold_.ttf);\n");
	styles += String("  font-style: normal;\n");
	styles += String("  font-weight: bold;\n");
	styles += "}\n";
	styles += "@font-face {\n";
	styles += String("  font-family: Arial, Helvetica, sans-serif;\n");
	styles += String("  font-style: normal;\n");
	styles += String("  font-weight: normal;\n");
	styles += "}\n";
	styles += "@font-face {\n";
	styles += String("  font-family: \"Arial Black\", Gadget, sans-serif;\n");
	styles += String("  font-style: normal;\n");
	styles += String("  font-weight: bold;\n");
	styles += "}\n\n";

	for (size_t i = 0; i < fontList.size(); i++)
	{
		if (fontList[i].number < 32)
			continue;

		String name = fontList[i].name+","+fontList[i].subfamilyName;

		if (fontFaces.size() == 0 || !exist(name))
		{
			fontFaces.push_back(name);
			styles += "@font-face {\n";
			styles += String("  font-family: \"")+fontList[i].name+"\";\n";
			styles += String("  src: url(fonts/")+NameFormat::toURL(fontList[i].file)+");\n";
			styles += String("  font-style: ")+getFontStyle(fontList[i].subfamilyName)+";\n";
			styles += String("  font-weight: ")+getFontWeight(fontList[i].subfamilyName)+";\n";
			styles += "}\n";
		}
	}

	return styles;
}

FONT_T& FontList::findFont(int idx)
{
	sysl->TRACE(String("FontList::findFont(int idx)"));

	for (size_t i = 0; i < fontList.size(); i++)
	{
		if (fontList[i].number == idx)
			return fontList[i];
	}

	sysl->TRACE(String("FontList::findFont: Font ID ")+idx+" not found. Have "+fontList.size()+" fonts in cache.");
	return emptyFont;
}

bool amx::FontList::exist(const String& ff)
{
	sysl->TRACE(String("FontList::exist(const String& ff)"));

	for (size_t i = 0; i < fontFaces.size(); i++)
	{
		if (fontFaces[i].compare(ff) == 0)
			return true;
	}

	return false;
}

strings::String FontList::getFontStyle(const strings::String& fs)
{
	sysl->TRACE(String("FontList::getFontStyle(const strings::String& fs)"));

	if (fs.caseCompare("Regular") == 0)
		return "normal";

	if (fs.caseCompare("Italic") == 0 || fs.caseCompare("Bold Italic") == 0)
		return "italic";

	return "normal";
}

strings::String amx::FontList::getFontWeight(const strings::String& fw)
{
	sysl->TRACE(String("FontList::getFontWeight(const strings::String& fw)"));

	if (fw.caseCompare("Regular") == 0)
		return "normal";

	if (fw.caseCompare("Bold") == 0 || fw.caseCompare("Bold Italic") == 0)
		return "bold";

	return "normal";
}

void amx::FontList::fillSysFonts()
{
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
