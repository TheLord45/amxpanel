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
#include "mapping.h"

extern Syslog *sysl;
extern Config *Configuration;

using namespace std;
using namespace amx;
using namespace strings;

Mapping::Mapping()
{
    sysl->TRACE(Syslog::ENTRY, std::string("Mapping::Mapping()"));
    bool cm = false;
    bool am = false;
    bool lm = false;
    bool bm = false;
    bool sm = false;
    bool strm = false;
    bool pm = false;
    bool me = false;
    bool im = false;
    MAP_T map;
    MAP_BM_T map_bm;
    MAP_PM_T map_pm;
    int depth = 0;

    String uri = "file://";
    uri.append(Configuration->getHTTProot());
    uri.append("/map.xma");
    xmlpp::TextReader reader(uri.toString());

    while(reader.read())
    {
        String name = string(reader.get_name());

        if (depth > reader.get_depth())
        {
            depth = 0;

            if (cm)
            {
                maps.map_cm.push_back(map);
                cm = false;
            }

            if (am)
            {
                maps.map_am.push_back(map);
                am = false;
            }

            if (lm)
            {
                maps.map_lm.push_back(map);
                lm = false;
            }

            if (bm && im)
            {
                maps.map_bm.push_back(map_bm);
                bm = im = false;
            }

            if (strm)
            {
                maps.map_strm.push_back(map);
                strm = false;
            }

            if (pm)
            {
                maps.map_pm.push_back(map_pm);
                pm = false;
            }
        }

        if (name.caseCompare("cm") == 0)
        {
            cm = true;
            am = false;
            lm = false;
            bm = false;
            sm = false;
            strm = false;
            pm = false;
            me = false;
        }

        if (name.caseCompare("am") == 0)
        {
            cm = false;
            am = true;
            lm = false;
            bm = false;
            sm = false;
            strm = false;
            pm = false;
            me = false;
        }

        if (name.caseCompare("lm") == 0)
        {
            cm = false;
            am = false;
            lm = true;
            bm = false;
            sm = false;
            strm = false;
            pm = false;
            me = false;
        }

        if (name.caseCompare("lm") == 0)
        {
            cm = false;
            am = false;
            lm = false;
            bm = true;
            sm = false;
            strm = false;
            pm = false;
            me = false;
        }

        if (name.caseCompare("lm") == 0)
        {
            cm = false;
            am = false;
            lm = false;
            bm = false;
            sm = true;
            strm = false;
            pm = false;
            me = false;
        }

        if (name.caseCompare("strm") == 0)
        {
            cm = false;
            am = false;
            lm = false;
            bm = false;
            sm = false;
            strm = true;
            pm = false;
            me = false;
        }

        if (name.caseCompare("pm") == 0)
        {
            cm = false;
            am = false;
            lm = false;
            bm = false;
            sm = false;
            strm = false;
            pm = true;
            me = false;
        }

        if ((cm || am || lm || strm) && name.caseCompare("me") == 0)
        {
            me = true;
            map.ax = 0;
            map.bn.clear();
            map.bt = 0;
            map.c = 0;
            map.p = 0;
            map.pg = 0;
            map.pn.clear();
            depth = reader.get_depth() + 1;
        }

        if (bm && name.caseCompare("im") == 0)
        {
            im = true;
            map_bm.i.clear();
            map_bm.id = 0;
        }

        if (bm && im && name.caseCompare("me") == 0)
            me = true;

        if (sm && name.caseCompare("me") == 0)
            me = true;

        if (pm && name.caseCompare("me") == 0)
        {
            me = true;
            map_pm.a = 0;
            map_pm.t.clear();
            map_pm.pg = 0;
            map_pm.bt = 0;
            map_pm.pn.clear();
            map_pm.bn.clear();
        }

        if ((cm || am || lm || strm) && me)
        {
            if (name.caseCompare("p") == 0)
                map.p = atoi(reader.get_value().c_str());

            if (name.caseCompare("c") == 0)
                map.c = atoi(reader.get_value().c_str());

            if (name.caseCompare("ax") == 0)
                map.ax = atoi(reader.get_value().c_str());

            if (name.caseCompare("pg") == 0)
                map.pg = atoi(reader.get_value().c_str());

            if (name.caseCompare("bt") == 0)
                map.bt = atoi(reader.get_value().c_str());

            if (name.caseCompare("pn") == 0)
                map.pn = reader.get_value();

            if (name.caseCompare("bn") == 0)
                map.bn = reader.get_value();
        }

        if (bm && im && me)
        {
            if (name.caseCompare("i") == 0)
                map_bm.i = reader.get_value();

            if (name.caseCompare("id") == 0)
                map_bm.id = atoi(reader.get_value().c_str());
        }

        if (sm && me && name.caseCompare("i") == 0)
            maps.map_sm.push_back(String(reader.get_value()));

        if (pm && me)
        {
            if (name.caseCompare("a") == 0)
                map_pm.a = atoi(reader.get_value().c_str());

            if (name.caseCompare("t") == 0)
                map_pm.t = reader.get_value();

            if (name.caseCompare("pg") == 0)
                map_pm.pg = atoi(reader.get_value().c_str());

            if (name.caseCompare("bt") == 0)
                map_pm.bt = atoi(reader.get_value().c_str());

            if (name.caseCompare("pn") == 0)
                map_pm.pn = reader.get_value();

            if (name.caseCompare("bn") == 0)
                map_pm.bn = reader.get_value();
        }
    }
}

Mapping::~Mapping()
{
    sysl->TRACE(Syslog::EXIT, std::string("Mapping::Mapping()"));
}

