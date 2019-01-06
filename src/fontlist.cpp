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

	for (size_t i = 0; i < fontList.size(); i++)
	{
		String name = fontList[i].name+","+fontList[i].subfamilyName;

		if (fontFaces.size() == 0 || !exist(name))
		{
			fontFaces.push_back(name);
			styles += "@font-face {\n";
			styles += String("  font-family: ")+NameFormat::toValidName(fontList[i].name)+";\n";
			styles += String("  src: url(")+NameFormat::toURL(fontList[i].file)+");\n";
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
