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
#include "fontlist.h"

extern Syslog *sysl;
extern Config *Configuration;

using namespace std;
using namespace amx;
using namespace strings;

amx::FontList::FontList(const strings::String& file)
{
    FONT_T font;
    String uri = "file://";
    uri.append(Configuration->getHTTProot());
    uri.append("/panel/");
    uri.append(file);
    xmlpp::TextReader reader(uri.toString());

    while(reader.read())
    {
        String name = string(reader.get_name());
        
        if (name.caseCompare("font") == 0 && reader.has_attributes())
        {
            font.number = atoi(reader.get_attribute(0).c_str());
            font.file.clear();
            font.fileSize = 0;
            font.faceIndex = 0;
            font.name.clear();
            font.subfamilyName.clear();
            font.fullName.clear();
            font.size = 0;
            font.usageCount = 0;
            fontList.push_back(font);
        }
        else if (name.caseCompare("file") == 0)
            fontList.back().file = reader.get_value();
        else if (name.caseCompare("fileSize") == 0)
            fontList.back().fileSize = atoi(reader.get_value().c_str());
        else if (name.caseCompare("faceIndex") == 0)
            fontList.back().faceIndex = atoi(reader.get_value().c_str());
        else if (name.caseCompare("name") == 0)
            fontList.back().name = reader.get_value();
        else if (name.caseCompare("subfamilyName") == 0)
            fontList.back().subfamilyName = reader.get_value();
        else if (name.caseCompare("fullName") == 0)
            fontList.back().fullName = reader.get_value();
        else if (name.caseCompare("size") == 0)
            fontList.back().size = atoi(reader.get_value().c_str());
        else if (name.caseCompare("usageCount") == 0)
            fontList.back().usageCount = atoi(reader.get_value().c_str());
    }

    reader.close();
}
