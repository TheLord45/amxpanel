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
#include "icon.h"

extern Syslog *sysl;
extern Config *Configuration;

using namespace std;
using namespace amx;
using namespace strings;

amx::Icon::Icon(const strings::String& file)
{
	sysl->TRACE(Syslog::ENTRY, std::string("Icon::Icon(const strings::String& file)"));
	int index = 0;
	String lastName;
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

			if (name.caseCompare("icon") == 0 && reader.has_attributes())
				index = atoi(reader.get_attribute(0).c_str());
			else if (name.caseCompare("file") == 0 && reader.has_value() && index >= 0)
			{
				ICON_T ico;
				ico.index = index;
				ico.file = reader.get_value();
				icons.push_back(ico);
				index = -1;
				sysl->TRACE(String("Icon::Icon: index=")+ico.index+", file="+ico.file);
			}

			lastName = name;
		}

		reader.close();
	}
	catch (xmlpp::internal_error& e)
	{
		sysl->errlog(string("Icon::Icon: ")+e.what());
		status = false;
		return;
	}

	status = true;
}

amx::Icon::~Icon()
{
    sysl->TRACE(Syslog::EXIT, std::string("Icon::Icon(const strings::String& file)"));
}

strings::String amx::Icon::getFileName(size_t idx)
{
	sysl->TRACE(Syslog::MESSAGE, std::string("Icon::getFileName(size_t idx)"));

	if (idx > icons.size())
		return "";

	return icons.at(idx).file;
}

int Icon::getID(size_t idx)
{
	sysl->TRACE(Syslog::MESSAGE, std::string("Icon::getID(size_t idx)"));

	if (idx > icons.size())
		return -1;

	return icons.at(idx).index;
}

strings::String amx::Icon::getFileFromID(int id)
{
	sysl->TRACE(String("Icon::getFileFromID(int id)"));

	for (size_t i = 0; i < icons.size(); i++)
	{
		if (icons[i].index == id)
		{
			sysl->TRACE(String("Icon::getFileFromID: ID ")+id+" with file "+icons[i].file+" found.");
			return icons[i].file;
		}
	}

	sysl->TRACE(String("Icon::getFileFromID: No icon for ID ")+id+" found!");
	return "";
}
