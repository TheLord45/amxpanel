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
#include "palette.h"

extern Syslog *sysl;
extern Config *Configuration;

using namespace std;
using namespace amx;
using namespace strings;

amx::Palette::Palette(const strings::String& file)
{
    String uri = "file://";
    uri.append(Configuration->getHTTProot());
    uri.append("/panel/");
    uri.append(file);
    xmlpp::TextReader reader(uri.toString());

    while(reader.read())
    {
        String name = string(reader.get_name());
        
        if (name.caseCompare("color") == 0 && reader.has_attributes() && reader.has_value())
        {
            PDATA_T color;
            color.clear();
            String sCol;
            color.index = atoi(reader.get_attribute("index").c_str());
            color.name = reader.get_attribute("name");
            sCol = reader.get_value();
            sCol = String("0x")+sCol.substring(1);
            color.color = strtoul(sCol.data(), 0, 16);
            palette.push_back(color);
        }
    }

    reader.close();
}

unsigned long amx::Palette::getColor(size_t idx)
{
    if (idx >= palette.size())
        return 0;

    return palette.at(idx).color;
}

unsigned long amx::Palette::getColor(const strings::String& name)
{
    for (size_t i = 0; i < palette.size(); i++)
    {
        if (palette[i].name.caseCompare(name) == 0)
            return palette[i].color;
    }

    return 0;
}
