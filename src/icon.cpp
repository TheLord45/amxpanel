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
#include "pushbutton.h"
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
				int width, height;

				if (PushButton::getImageDimensions(Configuration->getHTTProot()+"/images/"+ico.file, &width, &height))
				{
					ico.width = width;
					ico.height = height;
				}
				else
				{
					ico.width = 0;
					ico.height = 0;
				}

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

int Icon::getWidth(size_t idx)
{
	sysl->TRACE(Syslog::MESSAGE, std::string("Icon::getWidth(size_t idx)"));

	if (idx > icons.size())
		return -1;

	return icons.at(idx).width;
}

int Icon::getHeight(size_t idx)
{
	sysl->TRACE(Syslog::MESSAGE, std::string("Icon::getHeight(size_t idx)"));

	if (idx > icons.size())
		return -1;

	return icons.at(idx).height;
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
