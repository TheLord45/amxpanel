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
#include "panel.h"
#include "str.h"
#include "trace.h"

extern Config *Configuration;
extern Syslog *sysl;

using namespace std;
using namespace xmlpp;
using namespace amx;

Panel::Panel(PROJECT_T& prj, Palette *pPalet, Icon *pIco, FontList *pFL)
            : Project{prj}
{
	sysl->TRACE(Syslog::ENTRY, "Panel::Panel(const PROJECT& prj, Palette *pPalet, Icon *pIco, FontList *pFL)");
	pPalettes = pPalet;
	pIcons = pIco;
	pFontLists = pFL;

	if (pPalettes && pPalettes->isOk() && pIcons && pIcons->isOk() && pFontLists && pFontLists->isOk())
		status = true;
	else
		status = false;
}

Panel::~Panel()
{
	sysl->TRACE(Syslog::EXIT, "Panel::Panel(const PROJECT& prj, Palette *pPalet, Icon *pIco, FontList *pFL)");

	if (pPalettes && localPalette)
		delete pPalettes;

	if (pIcons && localIcon)
		delete pIcons;

	if (pFontLists && localFontList)
		delete pFontLists;
}

void Panel::readProject()
{
	DECL_TRACER("Panel::readProject()");

	string name, lastName, attr;
	int depth = 0;
	bool endElement = false;		// end of XML element detected
	status = true;

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

	PART Part = eNone;
	string uri; // = "file://";
	uri.append(Configuration->getHTTProot());
	uri.append("/prj.xma");
	sysl->TRACE("Panel::readProject: Reading from file: "+uri);

	if (Project.fwFeatureList.size() > 0 || Project.pageLists.size() > 0 ||
		Project.paletteList.size() > 0 || Project.resourceLists.size() > 0)
	{
		Project.fwFeatureList.clear();
		Project.pageLists.clear();
		Project.paletteList.clear();
		Project.resourceLists.clear();
	}

	try
	{
		TextReader reader(uri);
		sysl->TRACE("Panel::readProject: XML file was parsed ...");

		while(reader.read())
		{
			name = reader.get_name().raw();

			if (name.compare("#text") == 0)
				name = lastName;

			endElement = (reader.get_depth() < depth);
			sysl->TRACE("Panel::readProject: name="+name+", endElement="+to_string(endElement)+", Part="+to_string(Part)+", depth="+to_string(reader.get_depth())+" ("+to_string(depth)+")");

			if (reader.get_depth() <= 1 && depth > 1)
				Part = eNone;

			if (!endElement && Str::caseCompare(name, "pageList") == 0 && depth == 1)
			{
				Part = ePageList;

				if (reader.has_attributes())
					attr = reader.get_attribute(0).raw();
				else
					attr.clear();

				if (!attr.empty())
				{
					PAGE_LIST_T pageList;
					pageList.type = attr;
					Project.pageLists.push_back(pageList);
					attr.clear();
					sysl->TRACE("Panel::readProject: Added a PAGE_LIST_T entry!");
				}
			}
			else if (!endElement && Part == ePageList && Str::caseCompare(name, "pageEntry") == 0 && reader.get_depth() >= depth)
			{
				if (Project.pageLists.size() > 0)
				{
					PAGE_LIST_T& pageList = Project.pageLists.back();
					PAGE_ENTRY_T pageEntry;
					pageEntry.clear();
					pageList.pageList.push_back(pageEntry);
					sysl->TRACE("Panel::readProject: Added a PAGE_ENTRY_T entry to "+pageList.type+"!");
				}
			}
			else if (!endElement && Str::caseCompare(name, "resourceList") == 0 && depth == 1)
			{
				Part = eResourceList;

				if (reader.has_attributes())
					attr = reader.get_attribute(0).raw();
				else
					attr.clear();

				if (!attr.empty())
				{
					RESOURCE_LIST_T rl;
					rl.type = attr;
					Project.resourceLists.push_back(rl);
					attr.clear();
					sysl->TRACE("Panel::readProject: Added a RESOURCE_LIST_T entry!");
				}
			}
			else if (!endElement && Part == eResourceList && Str::caseCompare(name, "resource") == 0 && reader.get_depth() >= depth)
			{
				if (Project.resourceLists.size() > 0)
				{
					RESOURCE_LIST_T& rl = Project.resourceLists.back();
					RESOURCE_T res;
					res.clear();
					rl.ressource.push_back(res);
					sysl->TRACE("Panel::readProject: Added a RESOURCE_T entry to "+rl.type+"!");
				}
			}
			else if (!endElement && Part == eFwFeatureList && Str::caseCompare(name, "feature") == 0 && reader.get_depth() > depth)
			{
				FEATURE_T fwl;
				Project.fwFeatureList.push_back(fwl);
				sysl->TRACE("Panel::readProject: Added a FEATURE_T entry!");
			}
			else if (!endElement && Part == ePaletteList && Str::caseCompare(name, "palette") == 0 && reader.get_depth() >= depth)
			{
				PALETTE_T pal;
				Project.paletteList.push_back(pal);
				sysl->TRACE("Panel::readProject: Added a PALETTE_T entry!");
			}

			depth = reader.get_depth();

			if(!endElement && (reader.has_value() || reader.has_attributes()))
			{
				string value;

				if (reader.has_value())
				{
					string val = reader.get_value().raw();
					value = Str::trim(val);
				}

				if (reader.has_attributes())
				{
					string att = reader.get_attribute(0).raw();
					attr = Str::trim(att);
				}

				sysl->TRACE("Panel::readProject: name="+name+", value="+value+", attr="+attr+", Part="+to_string(Part)+", endElement="+to_string(endElement));

				if (!value.empty() || !attr.empty())
				{
					switch(Part)
					{
						case eVersionInfo:      setVersionInfo(name, value); attr.clear(); break;
						case eProjectInfo:      setProjectInfo(name, value, attr); attr.clear(); break;
						case eSupportFileList:  setSupportFileList(name, value); attr.clear(); break;
						case ePanelSetup:       setPanelSetup(name, value); attr.clear(); break;
						case ePageList:			setPageList(name, value); break;
						case eResourceList:		setResourceList(name, value, attr); break;
						case eFwFeatureList:	setFwFeatureList(name, value); break;
						case ePaletteList:		setPaletteList(name, value); break;
					}
				}
			}
			else if (!endElement)
			{
				if (Str::caseCompare(name, "versionInfo") == 0)
					Part = eVersionInfo;
				else if (Str::caseCompare(name, "projectInfo") == 0)
					Part = eProjectInfo;
				else if (Str::caseCompare(name, "supportFileList") == 0)
					Part = eSupportFileList;
				else if (Str::caseCompare(name, "panelSetup") == 0)
					Part = ePanelSetup;
				else if (Str::caseCompare(name, "fwFeatureList") == 0)
					Part = eFwFeatureList;
				else if (Str::caseCompare(name, "paletteList") == 0)
					Part = ePaletteList;

				sysl->TRACE("Panel::readProject: *name="+name+", Part="+to_string(Part)+", endElement="+to_string(endElement));
			}

			lastName = name;
		}

		reader.close();
	}
	catch (xmlpp::internal_error& e)
	{
		sysl->errlog(string("Panel::readProject: ")+e.what());
		status = false;
	}

	if (Configuration->getDebug())
	{
		sysl->TRACE("Panel::readProject: pageLists: "+to_string(Project.pageLists.size()));

		for (size_t i = 0; i < Project.pageLists.size(); i++)
		{
			PAGE_LIST_T pl = Project.pageLists[i];
			sysl->TRACE("Panel::readProject: pageList type: "+pl.type+" has "+to_string(pl.pageList.size())+" entries.");

			for (size_t j = 0; j < pl.pageList.size(); j++)
			{
				PAGE_ENTRY_T pe = pl.pageList[j];
				sysl->TRACE("Panel::readProject: name="+pe.name+", ID="+to_string(pe.pageID));
			}
		}

		sysl->TRACE("Panel::readProject: resourceLists: "+to_string(Project.resourceLists.size()));

		for (size_t i = 0; i < Project.resourceLists.size(); i++)
		{
			RESOURCE_LIST_T rl = Project.resourceLists[i];
			sysl->TRACE("Panel::readProject: resourceLists type: "+rl.type+" has "+to_string(rl.ressource.size())+" entries.");

			for (size_t j = 0; j < rl.ressource.size(); j++)
			{
				RESOURCE_T res = rl.ressource[j];
				sysl->TRACE("Panel::readProject: name="+res.name+", File="+res.file);
			}
		}

		sysl->TRACE("Panel::readProject: fwFeatureList: "+to_string(Project.fwFeatureList.size()));

		for (size_t i = 0; i < Project.fwFeatureList.size(); i++)
			sysl->TRACE("Panel::readProject: ID="+Project.fwFeatureList[i].featureID+", count="+to_string(Project.fwFeatureList[i].usageCount));

		sysl->TRACE("Panel::readProject: paletteList: "+to_string(Project.paletteList.size()));

		for (size_t i = 0; i < Project.paletteList.size(); i++)
			sysl->TRACE("Panel::readProject: ID="+to_string(Project.paletteList[i].paletteID)+", name="+Project.paletteList[i].name+", file="+Project.paletteList[i].file);
	}

	try
	{
		// Read the color palette
		if (!pPalettes)
		{
			pPalettes = new Palette(Project.paletteList, Project.supportFileList.colorFile);
			localPalette = true;
		}

		// Read the icon slot table
		if (!pIcons)
		{
			pIcons = new Icon(Project.supportFileList.iconFile);
			localIcon = true;
		}

		// Read the font map list
		if (!pFontLists)
		{
			pFontLists = new FontList(Project.supportFileList.fontFile);
			localFontList = true;
		}

		if (!pPalettes->isOk() || !pIcons->isOk() || !pFontLists->isOk())
		{
			sysl->warnlog("Panel::readProject: Reading the project failed!");
			status = false;
		}
	}
	catch (std::exception& e)
	{
		sysl->errlog(string("Panel::readProject: Memory error: ")+e.what());
		status = false;
	}
}

vector<string> Panel::getPageFileNames()
{
	DECL_TRACER("Panel::getPageFileNames()");
	vector<string> pgFnLst;

	sysl->TRACE("Panel::getPageFileNames: Number of pages: "+to_string(Project.pageLists.size()));

	for (size_t i = 0; i < Project.pageLists.size(); i++)
	{
		PAGE_LIST_T pl = Project.pageLists[i];
		sysl->TRACE("Panel::getPageFileNames: Number of pages in pages: "+to_string(pl.pageList.size()));

		for (size_t j = 0; j < pl.pageList.size(); j++)
		{
			PAGE_ENTRY_T pe = pl.pageList[j];

			if (pe.file.length() > 0)
				pgFnLst.push_back(pe.file);
		}
	}

	return pgFnLst;
}

void Panel::setVersionInfo(const string& name, const string& value)
{
	DECL_TRACER("Panel::setVersionInfo(const string& name, const string& value)");

    if (Str::caseCompare(name, "formatVersion") == 0)
        Project.version.formatVersion = atoi(value.c_str());

    if (Str::caseCompare(name, "graphicsVersion") == 0)
        Project.version.graphicsVersion = atoi(value.c_str());

    if (Str::caseCompare(name, "fileVersion") == 0)
        Project.version.fileVersion = atoi(value.c_str());

    if (Str::caseCompare(name, "designVersion") == 0)
        Project.version.designVersion = atoi(value.c_str());
}

void Panel::setProjectInfo(const string& name, const string& value, const string& attr)
{
	DECL_TRACER("Panel::setProjectInfo(const string& name, const string& value, const string& attr)");

    if (Str::caseCompare(name, "protection") == 0)
        Project.projectInfo.protection = value;

    if (Str::caseCompare(name, "password") == 0)
    {
        if (attr.compare("1") == 0)
            Project.projectInfo.encrypted = true;
        else
            Project.projectInfo.encrypted = false;

        Project.projectInfo.password = value;
    }

    if (Str::caseCompare(name, "panelType") == 0)
        Project.projectInfo.panelType = value;

    if (Str::caseCompare(name, "fileRevision") == 0)
        Project.projectInfo.fileRevision = value;

    if (Str::caseCompare(name, "dealerId") == 0)
        Project.projectInfo.dealerID = value;

    if (Str::caseCompare(name, "jobName") == 0)
        Project.projectInfo.jobName = value;

    if (Str::caseCompare(name, "salesOrder") == 0)
        Project.projectInfo.salesOrder = value;

    if (Str::caseCompare(name, "purchaseOrder") == 0)
        Project.projectInfo.purchaseOrder = value;

    if (Str::caseCompare(name, "jobComment") == 0)
        Project.projectInfo.jobComment = value;

    if (Str::caseCompare(name, "designerId") == 0)
        Project.projectInfo.designerID = value;

    if (Str::caseCompare(name, "creationDate") == 0)
        Project.projectInfo.creationDate = getDate(value, Project.projectInfo.creationDate);

    if (Str::caseCompare(name, "revisionDate") == 0)
        Project.projectInfo.revisionDate = getDate(value, Project.projectInfo.revisionDate);

    if (Str::caseCompare(name, "lastSaveDate") == 0)
        Project.projectInfo.lastSaveDate = getDate(value, Project.projectInfo.lastSaveDate);

    if (Str::caseCompare(name, "fileName") == 0)
        Project.projectInfo.fileName = value;

    if (Str::caseCompare(name, "colorChoice") == 0)
        Project.projectInfo.colorChoice = value;

    if (Str::caseCompare(name, "specifyPortCount") == 0)
        Project.projectInfo.specifyPortCount = atoi(value.c_str());

    if (Str::caseCompare(name, "specifyChanCount") == 0)
        Project.projectInfo.specifyChanCount = atoi(value.c_str());
}

void Panel::setSupportFileList(const string& name, const string& value)
{
	DECL_TRACER("Panel::setSupportFileList(const string& name, const string& value)");

    if (Str::caseCompare(name, "mapFile") == 0 && !value.empty())
        Project.supportFileList.mapFile = value;
	else if (Str::caseCompare(name, "colorFile") == 0 && !value.empty())
        Project.supportFileList.colorFile = value;
	else if (Str::caseCompare(name, "fontFile") == 0 && !value.empty())
        Project.supportFileList.fontFile = value;
	else if (Str::caseCompare(name, "themeFile") == 0 && !value.empty())
        Project.supportFileList.themeFile = value;
	else if (Str::caseCompare(name, "iconFile") == 0 && !value.empty())
        Project.supportFileList.iconFile = value;
	else if (Str::caseCompare(name, "externalButtonFile") == 0 && !value.empty())
        Project.supportFileList.externalButtonFile = value;
}

void Panel::setPanelSetup(const string& name, const string& value)
{
	DECL_TRACER("Panel::setPanelSetup(const string& name, const string& value)");

    if (Str::caseCompare(name, "portCount") == 0)
        Project.panelSetup.portCount = atoi(value.c_str());
    else if (Str::caseCompare(name, "setupPort") == 0)
        Project.panelSetup.setupPort = atoi(value.c_str());
    else if (Str::caseCompare(name, "addressCount") == 0)
        Project.panelSetup.addressCount = atoi(value.c_str());
    else if (Str::caseCompare(name, "channelCount") == 0)
        Project.panelSetup.channelCount = atoi(value.c_str());
    else if (Str::caseCompare(name, "levelCount") == 0)
        Project.panelSetup.levelCount = atoi(value.c_str());
    else if (Str::caseCompare(name, "powerUpPage") == 0)
        Project.panelSetup.powerUpPage = value;
    else if (Str::caseCompare(name, "powerUpPopup") == 0)
        Project.panelSetup.powerUpPopup.push_back(value);
    else if (Str::caseCompare(name, "feedbackBlinkRate") == 0)
        Project.panelSetup.feedbackBlinkRate = atoi(value.c_str());
    else if (Str::caseCompare(name, "startupString") == 0)
        Project.panelSetup.startupString = value;
    else if (Str::caseCompare(name, "wakeupString") == 0)
        Project.panelSetup.wakeupString = value;
    else if (Str::caseCompare(name, "sleepString") == 0)
        Project.panelSetup.sleepString = value;
    else if (Str::caseCompare(name, "standbyString") == 0)
        Project.panelSetup.standbyString = value;
    else if (Str::caseCompare(name, "shutdownString") == 0)
        Project.panelSetup.shutdownString = value;
    else if (Str::caseCompare(name, "idlePage") == 0)
        Project.panelSetup.idlePage = value;
    else if (Str::caseCompare(name, "idleTimeout") == 0)
        Project.panelSetup.idleTimeout = atoi(value.c_str());
    else if (Str::caseCompare(name, "extButtonsKey") == 0)
        Project.panelSetup.extButtonsKey = atoi(value.c_str());
    else if (Str::caseCompare(name, "screenWidth") == 0)
        Project.panelSetup.screenWidth = atoi(value.c_str());
    else if (Str::caseCompare(name, "screenHeight") == 0)
        Project.panelSetup.screenHeight = atoi(value.c_str());
    else if (Str::caseCompare(name, "screenRefresh") == 0)
        Project.panelSetup.screenRefresh = atoi(value.c_str());
    else if (Str::caseCompare(name, "screenRotate") == 0)
        Project.panelSetup.screenRotate = atoi(value.c_str());
    else if (Str::caseCompare(name, "screenDescription") == 0)
        Project.panelSetup.screenDescription = value;
    else if (Str::caseCompare(name, "pageTracking") == 0)
        Project.panelSetup.pageTracking = atoi(value.c_str());
    else if (Str::caseCompare(name, "brightness") == 0)
        Project.panelSetup.brightness = atoi(value.c_str());
    else if (Str::caseCompare(name, "lightSensorLevelPort") == 0)
        Project.panelSetup.lightSensorLevelPort = atoi(value.c_str());
    else if (Str::caseCompare(name, "lightSensorLevelCode") == 0)
        Project.panelSetup.lightSensorLevelCode = atoi(value.c_str());
    else if (Str::caseCompare(name, "lightSensorChannelPort") == 0)
        Project.panelSetup.lightSensorChannelPort = atoi(value.c_str());
    else if (Str::caseCompare(name, "lightSensorChannelCode") == 0)
        Project.panelSetup.lightSensorChannelCode = atoi(value.c_str());
    else if (Str::caseCompare(name, "motionSensorChannelPort") == 0)
        Project.panelSetup.motionSensorChannelPort = atoi(value.c_str());
    else if (Str::caseCompare(name, "motionSensorChannelCode") == 0)
        Project.panelSetup.motionSensorChannelCode = atoi(value.c_str());
    else if (Str::caseCompare(name, "batteryLevelPort") == 0)
        Project.panelSetup.batteryLevelPort = atoi(value.c_str());
    else if (Str::caseCompare(name, "batteryLevelCode") == 0)
        Project.panelSetup.batteryLevelCode = atoi(value.c_str());
    else if (Str::caseCompare(name, "irPortAMX38Emit") == 0)
        Project.panelSetup.irPortAMX38Emit = atoi(value.c_str());
    else if (Str::caseCompare(name, "irPortAMX455Emit") == 0)
        Project.panelSetup.irPortAMX455Emit = atoi(value.c_str());
    else if (Str::caseCompare(name, "irPortAMX38Recv") == 0)
        Project.panelSetup.irPortAMX38Recv = atoi(value.c_str());
    else if (Str::caseCompare(name, "irPortAMX455Recv") == 0)
        Project.panelSetup.irPortAMX455Recv = atoi(value.c_str());
    else if (Str::caseCompare(name, "irPortUser1") == 0)
        Project.panelSetup.irPortUser1 = atoi(value.c_str());
    else if (Str::caseCompare(name, "irPortUser2") == 0)
        Project.panelSetup.irPortUser2 = atoi(value.c_str());
    else if (Str::caseCompare(name, "cradleChannelPort") == 0)
        Project.panelSetup.cradleChannelPort = atoi(value.c_str());
    else if (Str::caseCompare(name, "cradleChannelCode") == 0)
        Project.panelSetup.cradleChannelCode = atoi(value.c_str());
    else if (Str::caseCompare(name, "uniqueID") == 0)
        Project.panelSetup.uniqueID = value;
    else if (Str::caseCompare(name, "appCreated") == 0)
        Project.panelSetup.appCreated = value;
    else if (Str::caseCompare(name, "buildNumber") == 0)
        Project.panelSetup.buildNumber = atoi(value.c_str());
    else if (Str::caseCompare(name, "appModified") == 0)
        Project.panelSetup.appModified = value;
    else if (Str::caseCompare(name, "buildNumberMod") == 0)
        Project.panelSetup.buildNumberMod = atoi(value.c_str());
    else if (Str::caseCompare(name, "buildStatusMod") == 0)
        Project.panelSetup.buildStatusMod = value;
    else if (Str::caseCompare(name, "activePalette") == 0)
        Project.panelSetup.activePalette = atoi(value.c_str());
    else if (Str::caseCompare(name, "marqueeSpeed") == 0)
        Project.panelSetup.marqueeSpeed = atoi(value.c_str());
    else if (Str::caseCompare(name, "setupPagesProject") == 0)
        Project.panelSetup.setupPagesProject = atoi(value.c_str());
    else if (Str::caseCompare(name, "voipCommandPort") == 0)
        Project.panelSetup.voipCommandPort = atoi(value.c_str());
}

void Panel::setPageList(const string& name, const string& value)
{
	DECL_TRACER("Panel::setPageList(const string& name, const string& value)");

	if (Project.pageLists.size() == 0)
		return;

	PAGE_LIST_T& pl = Project.pageLists.back();

	if (pl.pageList.size() == 0)
		return;

	PAGE_ENTRY_T& pe = pl.pageList.back();

	if (Str::caseCompare(name, "name") == 0 && !value.empty())
		pe.name = value;
	else if (Str::caseCompare(name, "pageID") == 0 && !value.empty())
		pe.pageID = atoi(value.c_str());
	else if (Str::caseCompare(name, "file") == 0 && !value.empty())
		pe.file = value;
	else if (Str::caseCompare(name, "isValid") == 0 && !value.empty())
		pe.isValid = atoi(value.c_str());
	else if (Str::caseCompare(name, "group") == 0 && !value.empty())
		pe.group = value;
	else if (Str::caseCompare(name, "popupType") == 0 && !value.empty())
		pe.popupType = atoi(value.c_str());
}

void Panel::setResourceList(const string& name, const string& value, const string& attr)
{
	DECL_TRACER("Panel::setResourceList(const string& name, const string& value, const string& attr)");

	if (Project.resourceLists.size() == 0)
		return;

	RESOURCE_LIST_T& rl = Project.resourceLists.back();

	if (rl.ressource.size() == 0)
		return;

	bool hasValue = false, hasAttr = false;
	RESOURCE_T& rs = rl.ressource.back();
	sysl->TRACE("Panel::setResourceList: name="+name+", value="+value+", attr="+attr);

	if (value.length() > 0)
		hasValue = true;

	if (attr.length() > 0)
		hasAttr = true;

	if (Str::caseCompare(name, "name") == 0 && hasValue)
		rs.name = value;
	else if (Str::caseCompare(name, "protocol") == 0 && hasValue)
		rs.protocol = value;
	else if (Str::caseCompare(name, "user") == 0 && hasValue)
		rs.user = value;
	else if (Str::caseCompare(name, "password") == 0)
	{
		if (hasValue)
			rs.password = value;

		if (hasAttr && Str::isNumeric(attr))
			rs.encrypted = atoi(attr.c_str());
	}
	else if (Str::caseCompare(name, "host") == 0 && hasValue)
		rs.host = value;
	else if (Str::caseCompare(name, "path") == 0 && hasValue)
		rs.path = value;
	else if (Str::caseCompare(name, "file") == 0 && hasValue)
		rs.file = value;
	else if (Str::caseCompare(name, "refresh") == 0 && hasValue)
		rs.refresh = atoi(value.c_str());
}

void Panel::setFwFeatureList(const string& name, const string& value)
{
	DECL_TRACER("Panel::setFwFeatureList(const string& name, const string& value)");

	if (Project.fwFeatureList.size() == 0)
		return;

	FEATURE_T& fw = Project.fwFeatureList.back();

	if (Str::caseCompare(name, "featureID") == 0)
		fw.featureID = value;
	else if (Str::caseCompare(name, "usageCount") == 0)
		fw.usageCount = atoi(value.c_str());
}

void Panel::setPaletteList(const string& name, const string& value)
{
	DECL_TRACER("Panel::setPaletteList(const string& name, const string& value)");

	if (Project.paletteList.size() == 0)
		return;

    PALETTE_T& pa = Project.paletteList.back();

    if (Str::caseCompare(name, "name") == 0)
        pa.name = value;
    else if (Str::caseCompare(name, "file") == 0)
        pa.file = value;
    else if (Str::caseCompare(name, "paletteID") == 0)
        pa.paletteID = atoi(value.c_str());
}

/*
 * Hier folgen einige private Hilfsfunktionen.
 */
DateTime& Panel::getDate(const string& dat, DateTime& dt)
{
	DECL_TRACER("Panel::getDate(const string& dat, DateTime& dt)");

	int day, month, year, hour, min, sec;
	vector<string> teile = Str::split(dat, ' ');

	if (teile.size() < 5)
		return dt;

	if (Str::caseCompare(teile[1], "Jan") == 0)
		month = 1;
	else if (Str::caseCompare(teile[1], "Feb") == 0)
		month = 2;
	else if (Str::caseCompare(teile[1], "Mar") == 0)
		month = 3;
	else if (Str::caseCompare(teile[1], "Apr") == 0)
		month = 4;
	else if (Str::caseCompare(teile[1], "Mai") == 0)
		month = 5;
	else if (Str::caseCompare(teile[1], "Jun") == 0)
		month = 6;
	else if (Str::caseCompare(teile[1], "Jul") == 0)
		month = 7;
	else if (Str::caseCompare(teile[1], "Aug") == 0)
		month = 8;
	else if (Str::caseCompare(teile[1], "Sep") == 0)
		month = 9;
	else if (Str::caseCompare(teile[1], "Oct") == 0)
		month = 10;
	else if (Str::caseCompare(teile[1], "Nov") == 0)
		month = 11;
	else
		month = 12;

	day = atoi(teile[2].c_str());
	vector<string> tim = Str::split(teile[3], ':');

	if (tim.size() == 3)
	{
		hour = atoi(tim[0].c_str());
		min = atoi(tim[1].c_str());
		sec = atoi(tim[2].c_str());
	}
	else
		hour = min = sec = 0;

	year = atoi(teile[4].c_str());

	dt.setTimestamp(year, month, day, hour, min, sec);
	sysl->TRACE("Panel::getDate: "+dt.toString());
	return dt;
}
