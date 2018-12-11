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

#include "config.h"
#include "syslog.h"
#include "touchpanel.h"

using namespace amx;
using namespace strings;
using namespace std;

extern Config *Configuration;
extern Syslog *sysl;

TouchPanel::TouchPanel()
{
    sysl->TRACE(Syslog::ENTRY, String("TouchPanel::TouchPanel()"));
    
}

TouchPanel::~TouchPanel()
{
    sysl->TRACE(Syslog::EXIT, String("TouchPanel::TouchPanel()"));
}

void TouchPanel::readPages()
{
    vector<String> pgs = getPageFileNames();

    for (size_t i = 0; i < pgs.size(); i++)
    {
        Page *p = new Page(pgs[i]);
        pages.push_back(*p);
        delete p;
    }
}
