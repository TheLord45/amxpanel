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
#include "strings.h"
#include "panel.h"

extern Config *Configuration;
extern Syslog *sysl;

using namespace std;
using namespace xmlpp;
using namespace amx;
using namespace strings;

void amx::Panel::readProject()
{
    String name, lastName;
    
    enum PART
    {
        eVersionInfo,
        eProjectInfo,
        eSupportFileList,
        ePanelSetup,
        ePageList,
        eResourceList,
        eFwFeatureList,
        ePaletteList
    };

    PART Part;
    String uri = "file://";
    uri.append(Configuration->getHTTProot());
    uri.append("/panel/main.xma");
    TextReader reader(uri.toString());

    while(reader.read())
    {
        name = reader.get_name();

        if(reader.has_value())
        {
            String value = string(reader.get_value());
            value.trim();

            if (value.size() > 0)
            {
                if (name.caseCompare("versionInfo") == 0)
                    Part = eVersionInfo;

                if (name.caseCompare("projectInfo") == 0)
                    Part = eProjectInfo;

                if (name.caseCompare("supportFileList") == 0)
                    Part = eSupportFileList;

                if (name.caseCompare("panelSetup") == 0)
                    Part = ePanelSetup;

                if (name.caseCompare("pageList") == 0)
                    Part = ePageList;

                if (name.caseCompare("resourceList") == 0)
                    Part = eResourceList;

                if (name.caseCompare("fwFeatureList") == 0)
                    Part = eFwFeatureList;

                if (name.caseCompare("paletteList") == 0)
                    Part = ePaletteList;

                // versionInfo
                if (Part == eVersionInfo)
                    setVersionInfo(name, value);

                // projectInfo
                if (Part == eProjectInfo)
                    setProjectInfo(name, value, string(reader.get_attribute(1)));

                lastName = name;
            }
        }
    }
}

void amx::Panel::setVersionInfo(const strings::String& name, const strings::String& value)
{
    if (name.caseCompare("formatVersion") == 0)
        Project.versionInfo.formatVersion = atoi(value.data());
    
    if (name.caseCompare("graphicsVersion") == 0)
        Project.versionInfo.graphicsVersion = atoi(value.data());
    
    if (name.caseCompare("fileVersion") == 0)
        Project.versionInfo.fileVersion = atoi(value.data());
    
    if (name.caseCompare("designVersion") == 0)
        Project.versionInfo.designVersion = atoi(value.data());
}

void amx::Panel::setProjectInfo(const strings::String& name, const strings::String& value, const strings::String& attr)
{
    if (name.caseCompare("protection") == 0)
        Project.projectInfo.protection = value;

    if (name.caseCompare("password") == 0)
    {
        if (attr.compare("1") == 0)
            Project.projectInfo.encrypted = true;
        else
            Project.projectInfo.encrypted = false;

        Project.projectInfo.password = value;
    }

    if (name.caseCompare("panelType") == 0)
        Project.projectInfo.panelType = value;

    if (name.caseCompare("fileRevision") == 0)
        Project.projectInfo.fileRevision = value;

    if (name.caseCompare("dealerId") == 0)
        Project.projectInfo.dealerId = value;

    if (name.caseCompare("jobName") == 0)
        Project.projectInfo.jobName = value;

    if (name.caseCompare("salesOrder") == 0)
        Project.projectInfo.salesOrder = value;

    if (name.caseCompare("purchaseOrder") == 0)
        Project.projectInfo.purchaseOrder = value;

    if (name.caseCompare("jobComment") == 0)
        Project.projectInfo.jobComment = value;

    if (name.caseCompare("designerId") == 0)
        Project.projectInfo.designerId = value;

    if (name.caseCompare("creationDate") == 0)
        Project.projectInfo.creationDate = getDate(value);

    if (name.caseCompare("revisionDate") == 0)
        Project.projectInfo.revisionDate = getDate(value);

    if (name.caseCompare("lastSaveDate") == 0)
        Project.projectInfo.lastSaveDate = getDate(value);

    if (name.caseCompare("fileName") == 0)
        Project.projectInfo.fileName = value;

    if (name.caseCompare("colorChoice") == 0)
        Project.projectInfo.colorChoice = value;

    if (name.caseCompare("specifyPortCount") == 0)
        Project.projectInfo.specifyPortCount = atoi(value.data());

    if (name.caseCompare("specifyChanCount") == 0)
        Project.projectInfo.specifyChanCount = atoi(value.data());
}

DateTime amx::Panel::getDate(const strings::String& dat)
{
    int day, month, year, hour, min, sec;
    DateTime dt;
    std::vector<String> teile = dat.split(' ');
    
    if (teile.size() < 5)
        return dt;

    if (teile[1].caseCompare("Jan") == 0)
        month = 1;
    else if (teile[1].caseCompare("Feb") == 0)
        month = 2;
    else if (teile[1].caseCompare("Mar") == 0)
        month = 3;
    else if (teile[1].caseCompare("Apr") == 0)
        month = 4;
    else if (teile[1].caseCompare("Mai") == 0)
        month = 5;
    else if (teile[1].caseCompare("Jun") == 0)
        month = 6;
    else if (teile[1].caseCompare("Jul") == 0)
        month = 7;
    else if (teile[1].caseCompare("Aug") == 0)
        month = 8;
    else if (teile[1].caseCompare("Sep") == 0)
        month = 9;
    else if (teile[1].caseCompare("Oct") == 0)
        month = 10;
    else if (teile[1].caseCompare("Nov") == 0)
        month = 11;
    else if (teile[1].caseCompare("Dec") == 0)
        month = 12;
    
    day = atoi(teile[2].data());
    std::vector<String> tim = teile[3].split(':');
    
    if (tim.size() == 3)
    {
        hour = atoi(tim[0].data());
        min = atoi(tim[1].data());
        sec = atoi(tim[2].data());
    }
    else
        hour = min = sec = 0;

    year = atoi(teile[4].data());

    dt.setTimestamp(year, month, day, hour, min, sec);
    return dt;
}
