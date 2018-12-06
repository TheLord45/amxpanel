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

amx::Panel::Panel()
{
    pPalettes = 0;
    pIcons = 0;
    pFontLists = 0;
}

amx::Panel::~Panel()
{
    if (pPalettes)
        delete pPalettes;

    if (pIcons)
        delete pIcons;

    if (pFontLists)
        delete pFontLists;
}

void amx::Panel::readProject()
{
    String name, lastName, attr;
    bool bPageEntry = false;
    bool bResource = false;
    bool bFeature = false;
    bool bPalette = false;
    int depth = 0;
    
    enum PART
    {
        eNone,
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
        
        if (reader.get_depth() < depth)
        {
            bPageEntry = false;
            bResource = false;
            bFeature = false;
            bPalette = false;
            Part = eNone;
        }

        depth = reader.get_depth();

        if(reader.has_value())
        {
            String value = string(reader.get_value());
            value.trim();

            if (value.size() > 0)
            {
                switch(Part)
                {
                    case eVersionInfo:      setVersionInfo(name, value); break;
                    case eProjectInfo:      setProjectInfo(name, value, string(reader.get_attribute(0))); break;
                    case eSupportFileList:  setSupportFileList(name, value); break;
                    case ePanelSetup:       setPanelSetup(name, value); break;

                    case ePageList:
                        if (!bPageEntry)
                        {
                            PAGE_LIST_T pageList;
                            pageList.type = attr;
                            PAGE_ENTRY_T pe;
                            pe.clear();
                            pageList.pageList.push_back(pe);
                            Project.pageLists.push_back(pageList);
                        }
                        else
                            setPageList(name, value);
                    break;

                    case eResourceList:
                        if (!bResource)
                        {
                            RESOURCE_LIST_T resource;
                            resource.type = attr;
                            RESOURCE_T rs;
                            rs.clear();
                            resource.ressource.push_back(rs);
                            Project.resourceLists.push_back(resource);
                        }
                        else
                        {
                            if (reader.has_attributes())
                                attr = reader.get_attribute(0);
                            else
                                attr.clear();

                            setResourceList(name, value, attr);
                        }
                    break;

                    case eFwFeatureList:
                        if (!bFeature)
                        {
                            FEATURE_T fw;
                            Project.fwFeatureList.push_back(fw);
                        }
                        else
                            setFwFeatureList(name, value);
                    break;

                    case ePaletteList:
                        if (!bPalette)
                        {
                            PALETTE_T pa;
                            Project.paletteList.push_back(pa);
                        }
                        else
                            setPaletteList(name, value);
                    break;
                }

                if (name.caseCompare("versionInfo") == 0)
                    Part = eVersionInfo;

                if (name.caseCompare("projectInfo") == 0)
                    Part = eProjectInfo;

                if (name.caseCompare("supportFileList") == 0)
                    Part = eSupportFileList;

                if (name.caseCompare("panelSetup") == 0)
                    Part = ePanelSetup;

                if (name.caseCompare("pageList") == 0)
                {
                    Part = ePageList;

                    if (reader.has_attributes())
                        attr = reader.get_attribute(0);
                    else
                        attr.clear();
                }

                if (name.caseCompare("resourceList") == 0)
                {
                    Part = eResourceList;

                    if (reader.has_attributes())
                        attr = reader.get_attribute(0);
                    else
                        attr.clear();
                }

                if (name.caseCompare("fwFeatureList") == 0)
                    Part = eFwFeatureList;

                if (name.caseCompare("paletteList") == 0)
                    Part = ePaletteList;

                if (Part == ePageList && name.caseCompare("pageEntry") == 0)
                    bPageEntry = true;

                if (Part == eResourceList && name.caseCompare("resource") == 0)
                    bResource = true;

                if (Part == eFwFeatureList && name.caseCompare("feature") == 0)
                    bFeature = true;

                if (Part == ePageList && name.caseCompare("palette") == 0)
                    bPalette = true;

                lastName = name;
            }
        }
    }
    
    reader.close();
    // Read the color palette
    if (!pPalettes)
        pPalettes = new Palette(Project.supportFileList.colorFile);
    
    // Read the icon slot table
    if (!pIcons)
        pIcons = new Icon(Project.supportFileList.iconFile);
    
    // Read the font map list
    if (!pFontLists)
        pFontLists = new FontList(Project.supportFileList.fontFile);
}

void amx::Panel::setVersionInfo(const strings::String& name, const strings::String& value)
{
    if (name.caseCompare("formatVersion") == 0)
        Project.version.formatVersion = atoi(value.data());
    
    if (name.caseCompare("graphicsVersion") == 0)
        Project.version.graphicsVersion = atoi(value.data());
    
    if (name.caseCompare("fileVersion") == 0)
        Project.version.fileVersion = atoi(value.data());
    
    if (name.caseCompare("designVersion") == 0)
        Project.version.designVersion = atoi(value.data());
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
        Project.projectInfo.dealerID = value;

    if (name.caseCompare("jobName") == 0)
        Project.projectInfo.jobName = value;

    if (name.caseCompare("salesOrder") == 0)
        Project.projectInfo.salesOrder = value;

    if (name.caseCompare("purchaseOrder") == 0)
        Project.projectInfo.purchaseOrder = value;

    if (name.caseCompare("jobComment") == 0)
        Project.projectInfo.jobComment = value;

    if (name.caseCompare("designerId") == 0)
        Project.projectInfo.designerID = value;

    if (name.caseCompare("creationDate") == 0)
        Project.projectInfo.creationDate = getDate(value, Project.projectInfo.creationDate);

    if (name.caseCompare("revisionDate") == 0)
        Project.projectInfo.revisionDate = getDate(value, Project.projectInfo.revisionDate);

    if (name.caseCompare("lastSaveDate") == 0)
        Project.projectInfo.lastSaveDate = getDate(value, Project.projectInfo.lastSaveDate);

    if (name.caseCompare("fileName") == 0)
        Project.projectInfo.fileName = value;

    if (name.caseCompare("colorChoice") == 0)
        Project.projectInfo.colorChoice = value;

    if (name.caseCompare("specifyPortCount") == 0)
        Project.projectInfo.specifyPortCount = atoi(value.data());

    if (name.caseCompare("specifyChanCount") == 0)
        Project.projectInfo.specifyChanCount = atoi(value.data());
}

void amx::Panel::setSupportFileList(const strings::String& name, const strings::String& value)
{
    if (name.caseCompare("mapFile") == 0)
        Project.supportFileList.mapFile = value;
    else if (name.caseCompare("colorFile") == 0)
        Project.supportFileList.colorFile = value;
    else if (name.caseCompare("fontFile") == 0)
        Project.supportFileList.fontFile = value;
    else if (name.caseCompare("themeFile") == 0)
        Project.supportFileList.themeFile = value;
    else if (name.caseCompare("iconFile") == 0)
        Project.supportFileList.iconFile = value;
    else if (name.caseCompare("externalButtonFile") == 0)
        Project.supportFileList.externalButtonFile = value;
}

void amx::Panel::setPanelSetup(const strings::String& name, const strings::String& value)
{
    if (name.caseCompare("portCount") == 0)
        Project.panelSetup.portCount = atoi(value.data());
    else if (name.caseCompare("setupPort") == 0)
        Project.panelSetup.setupPort = atoi(value.data());
    else if (name.caseCompare("addressCount") == 0)
        Project.panelSetup.addressCount = atoi(value.data());
    else if (name.caseCompare("channelCount") == 0)
        Project.panelSetup.channelCount = atoi(value.data());
    else if (name.caseCompare("levelCount") == 0)
        Project.panelSetup.levelCount = atoi(value.data());
    else if (name.caseCompare("powerUpPage") == 0)
        Project.panelSetup.powerUpPage = value;
    else if (name.caseCompare("powerUpPopup") == 0)
        Project.panelSetup.powerUpPopup.push_back(value);
    else if (name.caseCompare("feedbackBlinkRate") == 0)
        Project.panelSetup.feedbackBlinkRate = atoi(value.data());
    else if (name.caseCompare("startupString") == 0)
        Project.panelSetup.startupString = value;
    else if (name.caseCompare("wakeupString") == 0)
        Project.panelSetup.wakeupString = value;
    else if (name.caseCompare("sleepString") == 0)
        Project.panelSetup.sleepString = value;
    else if (name.caseCompare("standbyString") == 0)
        Project.panelSetup.standbyString = value;
    else if (name.caseCompare("shutdownString") == 0)
        Project.panelSetup.shutdownString = value;
    else if (name.caseCompare("idlePage") == 0)
        Project.panelSetup.idlePage = value;
    else if (name.caseCompare("idleTimeout") == 0)
        Project.panelSetup.idleTimeout = atoi(value.data());
    else if (name.caseCompare("extButtonsKey") == 0)
        Project.panelSetup.extButtonsKey = atoi(value.data());
    else if (name.caseCompare("screenWidth") == 0)
        Project.panelSetup.screenWidth = atoi(value.data());
    else if (name.caseCompare("screenHeight") == 0)
        Project.panelSetup.screenHeight = atoi(value.data());
    else if (name.caseCompare("screenRefresh") == 0)
        Project.panelSetup.screenRefresh = atoi(value.data());
    else if (name.caseCompare("screenRotate") == 0)
        Project.panelSetup.screenRotate = atoi(value.data());
    else if (name.caseCompare("screenDescription") == 0)
        Project.panelSetup.screenDescription = value;
    else if (name.caseCompare("pageTracking") == 0)
        Project.panelSetup.pageTracking = atoi(value.data());
    else if (name.caseCompare("brightness") == 0)
        Project.panelSetup.brightness = atoi(value.data());
    else if (name.caseCompare("lightSensorLevelPort") == 0)
        Project.panelSetup.lightSensorLevelPort = atoi(value.data());
    else if (name.caseCompare("lightSensorLevelCode") == 0)
        Project.panelSetup.lightSensorLevelCode = atoi(value.data());
    else if (name.caseCompare("lightSensorChannelPort") == 0)
        Project.panelSetup.lightSensorChannelPort = atoi(value.data());
    else if (name.caseCompare("lightSensorChannelCode") == 0)
        Project.panelSetup.lightSensorChannelCode = atoi(value.data());
    else if (name.caseCompare("motionSensorChannelPort") == 0)
        Project.panelSetup.motionSensorChannelPort = atoi(value.data());
    else if (name.caseCompare("motionSensorChannelCode") == 0)
        Project.panelSetup.motionSensorChannelCode = atoi(value.data());
    else if (name.caseCompare("batteryLevelPort") == 0)
        Project.panelSetup.batteryLevelPort = atoi(value.data());
    else if (name.caseCompare("batteryLevelCode") == 0)
        Project.panelSetup.batteryLevelCode = atoi(value.data());
    else if (name.caseCompare("irPortAMX38Emit") == 0)
        Project.panelSetup.irPortAMX38Emit = atoi(value.data());
    else if (name.caseCompare("irPortAMX455Emit") == 0)
        Project.panelSetup.irPortAMX455Emit = atoi(value.data());
    else if (name.caseCompare("irPortAMX38Recv") == 0)
        Project.panelSetup.irPortAMX38Recv = atoi(value.data());
    else if (name.caseCompare("irPortAMX455Recv") == 0)
        Project.panelSetup.irPortAMX455Recv = atoi(value.data());
    else if (name.caseCompare("irPortUser1") == 0)
        Project.panelSetup.irPortUser1 = atoi(value.data());
    else if (name.caseCompare("irPortUser2") == 0)
        Project.panelSetup.irPortUser2 = atoi(value.data());
    else if (name.caseCompare("cradleChannelPort") == 0)
        Project.panelSetup.cradleChannelPort = atoi(value.data());
    else if (name.caseCompare("cradleChannelCode") == 0)
        Project.panelSetup.cradleChannelCode = atoi(value.data());
    else if (name.caseCompare("uniqueID") == 0)
        Project.panelSetup.uniqueID = value;
    else if (name.caseCompare("appCreated") == 0)
        Project.panelSetup.appCreated = value;
    else if (name.caseCompare("buildNumber") == 0)
        Project.panelSetup.buildNumber = atoi(value.data());
    else if (name.caseCompare("appModified") == 0)
        Project.panelSetup.appModified = value;
    else if (name.caseCompare("buildNumberMod") == 0)
        Project.panelSetup.buildNumberMod = atoi(value.data());
    else if (name.caseCompare("buildStatusMod") == 0)
        Project.panelSetup.buildStatusMod = value;
    else if (name.caseCompare("activePalette") == 0)
        Project.panelSetup.activePalette = atoi(value.data());
    else if (name.caseCompare("marqueeSpeed") == 0)
        Project.panelSetup.marqueeSpeed = atoi(value.data());
    else if (name.caseCompare("setupPagesProject") == 0)
        Project.panelSetup.setupPagesProject = atoi(value.data());
    else if (name.caseCompare("voipCommandPort") == 0)
        Project.panelSetup.voipCommandPort = atoi(value.data());
}

void amx::Panel::setPageList(const strings::String& name, const strings::String& value)
{
    PAGE_LIST_T pl = Project.pageLists.back();
    PAGE_ENTRY_T pe = pl.pageList.back();

    if (name.caseCompare("name") == 0)
        pe.name = value;
    else if (name.caseCompare("pageID") == 0)
        pe.pageID = atoi(value.data());
    else if (name.caseCompare("file") == 0)
        pe.file = value;
    else if (name.caseCompare("isValid") == 0)
        pe.isValid = atoi(value.data());
    else if (name.caseCompare("group") == 0)
        pe.group = value;
    else if (name.caseCompare("popupType") == 0)
        pe.popupType = atoi(value.data());
}

void amx::Panel::setResourceList(const strings::String& name, const strings::String& value, const strings::String& attr)
{
    RESOURCE_LIST_T rl = Project.resourceLists.back();
    RESOURCE_T rs = rl.ressource.back();
    
    if (name.caseCompare("name") == 0)
        rs.name = value;
    else if (name.caseCompare("protocol") == 0)
        rs.protocol = value;
    else if (name.caseCompare("user") == 0)
        rs.user = value;
    else if (name.caseCompare("password") == 0)
    {
        rs.password = value;

        if (attr.length() > 0 && attr.isNumeric())
            rs.encrypted = atoi(attr.data());
    }
    else if (name.caseCompare("host") == 0)
        rs.host = value;
    else if (name.caseCompare("path") == 0)
        rs.path = value;
    else if (name.caseCompare("file") == 0)
        rs.file = value;
    else if (name.caseCompare("refresh") == 0)
        rs.refresh = atoi(value.data());
}

void amx::Panel::setFwFeatureList(const strings::String& name, const strings::String& value)
{
    FEATURE_T fw = Project.fwFeatureList.back();
    
    if (name.caseCompare("featureID") == 0)
        fw.featureID = value;
    else if (name.caseCompare("usageCount") == 0)
        fw.usageCount = atoi(value.data());
}

void amx::Panel::setPaletteList(const strings::String& name, const strings::String& value)
{
    PALETTE_T pa = Project.paletteList.back();
    
    if (name.caseCompare("name") == 0)
        pa.name = value;
    else if (name.caseCompare("file") == 0)
        pa.file = value;
    else if (name.caseCompare("paletteID") == 0)
        pa.paletteID = atoi(value.data());
}

/*
 * Hier folgen einige private Hilfsfunktionen.
 */
DateTime& amx::Panel::getDate(const strings::String& dat, DateTime& dt)
{
    int day, month, year, hour, min, sec;
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
    else
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
