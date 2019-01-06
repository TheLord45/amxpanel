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
	sysl->TRACE(Syslog::ENTRY, std::string("Panel::Panel()"));
    pPalettes = 0;
    pIcons = 0;
    pFontLists = 0;
    status = false;
    readProject();
}

Panel::Panel(const PROJECT& prj, Palette *pPalet, Icon *pIco, FontList *pFL)
            : Project(prj)
{
	sysl->TRACE(Syslog::ENTRY, std::string("Panel::Panel(const PROJECT& prj, Palette *pPalet, Icon *pIco, FontList *pFL)"));
    pPalettes = pPalet;
    pIcons = pIco;
    pFontLists = pFL;

    if (pPalettes && pPalettes->isOk() && pIcons && pIcons->isOk() && pFontLists && pFontLists->isOk())
        status = true;
    else
        status = false;
}

amx::Panel::~Panel()
{
	sysl->TRACE(Syslog::EXIT, std::string("Panel::Panel(const PROJECT& prj, Palette *pPalet, Icon *pIco, FontList *pFL)"));

    if (pPalettes)
        delete pPalettes;

    if (pIcons)
        delete pIcons;

    if (pFontLists)
        delete pFontLists;
}

void amx::Panel::readProject()
{
	sysl->TRACE(Syslog::MESSAGE, std::string("Panel::readProject()"));

	String name, lastName, attr;
	int depth = 0;
	bool endElement = false;		// end of XML element detected
	bool addElement = false;		// a new structure should be added
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

	PART Part;
	String uri; // = "file://";
	uri.append(Configuration->getHTTProot());
	uri.append("/prj.xma");
	sysl->TRACE(String("Panel::readProject: Reading from file: ")+uri);

	try
	{
		TextReader reader(uri.toString());
		sysl->TRACE(String("Panel::readProject: XML file was parsed ..."));

		while(reader.read())
		{
			name = reader.get_name();

			if (name.at(0) == '#')
				name = lastName;

			sysl->TRACE(String("Panel::readProject: name=")+name+", depth="+reader.get_depth()+" ("+depth+")");
			endElement = (reader.get_depth() < depth);

			if (endElement)
				addElement = false;

			if (reader.get_depth() <= 1 && depth > 1)
				Part = eNone;

			if (name.caseCompare("pageList") == 0 && depth == 1)
			{
				Part = ePageList;

				if (reader.has_attributes())
					attr = reader.get_attribute(0);
				else
					attr.clear();

				if (!endElement && !attr.empty())
				{
					PAGE_LIST_T pageList;
					pageList.type = attr;
					Project.pageLists.push_back(pageList);
					attr.clear();
					sysl->TRACE(String("Panel::readProject: Added a PAGE_LIST_T entry!"));
				}
			}
			else if (Part == ePageList && name.caseCompare("pageEntry") == 0 && reader.get_depth() > depth)
			{
				if (!endElement)
				{
					if (Project.pageLists.size() > 0)
					{
						PAGE_LIST_T& pageList = Project.pageLists.back();
						PAGE_ENTRY_T pageEntry;
						pageEntry.clear();
						pageList.pageList.push_back(pageEntry);
						sysl->TRACE(String("Panel::readProject: Added a PAGE_ENTRY_T entry to ")+pageList.type+"!");
					}
				}
			}
			else if (name.caseCompare("resourceList") == 0 && depth == 1)
			{
				Part = eResourceList;

				if (reader.has_attributes())
					attr = reader.get_attribute(0);
				else
					attr.clear();

				if (!endElement && !attr.empty())
				{
					RESOURCE_LIST_T rl;
					rl.type = attr;
					Project.resourceLists.push_back(rl);
					attr.clear();
					sysl->TRACE(String("Panel::readProject: Added a RESOURCE_LIST_T entry!"));
				}
			}
			else if (Part == eResourceList && name.caseCompare("resource") == 0 && reader.get_depth() > depth)
			{
				if (!endElement)
				{
					if (Project.resourceLists.size() > 0)
					{
						RESOURCE_LIST_T& rl = Project.resourceLists.back();
						RESOURCE_T res;
						res.clear();
						rl.ressource.push_back(res);
						sysl->TRACE(String("Panel::readProject: Added a RESOURCE_T entry to ")+rl.type+"!");
					}
				}
			}
			else if (Part == eFwFeatureList && name.caseCompare("feature") == 0 && reader.get_depth() > depth)
			{
				if (!endElement)
				{
					FEATURE_T fwl;
					Project.fwFeatureList.push_back(fwl);
					sysl->TRACE(String("Panel::readProject: Added a FEATURE_T entry!"));
				}
			}
			else if (Part == ePaletteList && name.caseCompare("palette") == 0 && reader.get_depth() > depth)
			{
				if (!endElement)
				{
					PALETTE_T pal;
					Project.paletteList.push_back(pal);
					sysl->TRACE(String("Panel::readProject: Added a PALETTE_T entry!"));
				}
			}

			depth = reader.get_depth();

			if(!endElement && (reader.has_value() || reader.has_attributes()))
			{
				String value;

				if (reader.has_value())
				{
					value = string(reader.get_value());
					value.trim();
					value.replace("\n", "");
					value.replace("\r", "");
				}

				if (reader.has_attributes())
				{
					attr = reader.get_attribute(0);
					attr.trim();
				}

				sysl->TRACE(String("Panel::readProject: name=")+name+", value="+value+", attr="+attr+", Part="+Part+", addElement="+addElement);

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
				if (name.caseCompare("versionInfo") == 0)
					Part = eVersionInfo;
				else if (name.caseCompare("projectInfo") == 0)
					Part = eProjectInfo;
				else if (name.caseCompare("supportFileList") == 0)
					Part = eSupportFileList;
				else if (name.caseCompare("panelSetup") == 0)
					Part = ePanelSetup;
				else if (name.caseCompare("fwFeatureList") == 0)
					Part = eFwFeatureList;
				else if (name.caseCompare("paletteList") == 0)
					Part = ePaletteList;

				sysl->TRACE(String("Panel::readProject: *name=")+name+", Part="+Part+", addElement="+addElement);
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
		sysl->TRACE(String("Panel::readProject: pageLists: ")+Project.pageLists.size());

		for (size_t i = 0; i < Project.pageLists.size(); i++)
		{
			PAGE_LIST_T pl = Project.pageLists[i];
			sysl->TRACE(String("Panel::readProject: pageList type: ")+pl.type+" has "+pl.pageList.size()+" entries.");

			for (size_t j = 0; j < pl.pageList.size(); j++)
			{
				PAGE_ENTRY_T pe = pl.pageList[j];
				sysl->TRACE(String("Panel::readProject: name=")+pe.name+", ID="+pe.pageID);
			}
		}

		sysl->TRACE(String("Panel::readProject: resourceLists: ")+Project.resourceLists.size());

		for (size_t i = 0; i < Project.resourceLists.size(); i++)
		{
			RESOURCE_LIST_T rl = Project.resourceLists[i];
			sysl->TRACE(String("Panel::readProject: resourceLists type: ")+rl.type+" has "+rl.ressource.size()+" entries.");

			for (size_t j = 0; j < rl.ressource.size(); j++)
			{
				RESOURCE_T res = rl.ressource[j];
				sysl->TRACE(String("Panel::readProject: name=")+res.name+", File="+res.file);
			}
		}

		sysl->TRACE(String("Panel::readProject: fwFeatureList: ")+Project.fwFeatureList.size());

		for (size_t i = 0; i < Project.fwFeatureList.size(); i++)
			sysl->TRACE(String("Panel::readProject: ID=")+Project.fwFeatureList[i].featureID+", count="+Project.fwFeatureList[i].usageCount);

		sysl->TRACE(String("Panel::readProject: paletteList: ")+Project.paletteList.size());

		for (size_t i = 0; i < Project.paletteList.size(); i++)
			sysl->TRACE(String("Panel::readProject: ID=")+Project.paletteList[i].paletteID+", name="+Project.paletteList[i].name+", file="+Project.paletteList[i].file);
	}

	// Read the color palette
	if (!pPalettes)
		pPalettes = new Palette(Project.supportFileList.colorFile);

	// Read the icon slot table
	if (!pIcons)
		pIcons = new Icon(Project.supportFileList.iconFile);

	// Read the font map list
	if (!pFontLists)
		pFontLists = new FontList(Project.supportFileList.fontFile);

	if (!pPalettes->isOk() || !pIcons->isOk() || !pFontLists->isOk())
	{
		sysl->warnlog(String("Panel::readProject: Reading the project failed!"));
		status = false;
	}
}

vector<String> Panel::getPageFileNames()
{
	sysl->TRACE(Syslog::MESSAGE, std::string("Panel::getPageFileNames()"));
	vector<String> pgFnLst;

	sysl->TRACE(String("Panel::getPageFileNames: Number of pages: ")+Project.pageLists.size());

	for (size_t i = 0; i < Project.pageLists.size(); i++)
	{
		PAGE_LIST_T pl = Project.pageLists[i];
		sysl->TRACE(String("Panel::getPageFileNames: Number of pages in pages: ")+pl.pageList.size());

		for (size_t j = 0; j < pl.pageList.size(); j++)
		{
			PAGE_ENTRY_T pe = pl.pageList[j];

			if (pe.file.length() > 0)
				pgFnLst.push_back(pe.file);
		}
	}

	return pgFnLst;
}

void amx::Panel::setVersionInfo(const strings::String& name, const strings::String& value)
{
	sysl->TRACE(Syslog::MESSAGE, std::string("Panel::setVersionInfo(const strings::String& name, const strings::String& value)"));

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
	sysl->TRACE(Syslog::MESSAGE, std::string("Panel::setProjectInfo(const strings::String& name, const strings::String& value, const strings::String& attr)"));

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
	sysl->TRACE(Syslog::MESSAGE, std::string("Panel::setSupportFileList(const strings::String& name, const strings::String& value)"));

    if (name.caseCompare("mapFile") == 0 && !value.empty())
        Project.supportFileList.mapFile = value;
	else if (name.caseCompare("colorFile") == 0 && !value.empty())
        Project.supportFileList.colorFile = value;
	else if (name.caseCompare("fontFile") == 0 && !value.empty())
        Project.supportFileList.fontFile = value;
	else if (name.caseCompare("themeFile") == 0 && !value.empty())
        Project.supportFileList.themeFile = value;
	else if (name.caseCompare("iconFile") == 0 && !value.empty())
        Project.supportFileList.iconFile = value;
	else if (name.caseCompare("externalButtonFile") == 0 && !value.empty())
        Project.supportFileList.externalButtonFile = value;
}

void amx::Panel::setPanelSetup(const strings::String& name, const strings::String& value)
{
	sysl->TRACE(Syslog::MESSAGE, std::string("Panel::setPanelSetup(const strings::String& name, const strings::String& value)"));

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
	sysl->TRACE(Syslog::MESSAGE, std::string("Panel::setPageList(const strings::String& name, const strings::String& value)"));

	if (Project.pageLists.size() == 0)
		return;

	PAGE_LIST_T& pl = Project.pageLists.back();

	if (pl.pageList.size() == 0)
		return;

	PAGE_ENTRY_T& pe = pl.pageList.back();

	if (name.caseCompare("name") == 0 && !value.empty())
		pe.name = value;
	else if (name.caseCompare("pageID") == 0 && !value.empty())
		pe.pageID = atoi(value.data());
	else if (name.caseCompare("file") == 0 && !value.empty())
		pe.file = value;
	else if (name.caseCompare("isValid") == 0 && !value.empty())
		pe.isValid = atoi(value.data());
	else if (name.caseCompare("group") == 0 && !value.empty())
		pe.group = value;
	else if (name.caseCompare("popupType") == 0 && !value.empty())
		pe.popupType = atoi(value.data());
}

void amx::Panel::setResourceList(const strings::String& name, const strings::String& value, const strings::String& attr)
{
	sysl->TRACE(Syslog::MESSAGE, std::string("Panel::setResourceList(const strings::String& name, const strings::String& value, const strings::String& attr)"));

	if (Project.resourceLists.size() == 0)
		return;

	RESOURCE_LIST_T& rl = Project.resourceLists.back();

	if (rl.ressource.size() == 0)
		return;

	RESOURCE_T& rs = rl.ressource.back();

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
	sysl->TRACE(std::string("Panel::setFwFeatureList(const strings::String& name, const strings::String& value)"));

	if (Project.fwFeatureList.size() == 0)
		return;

	FEATURE_T& fw = Project.fwFeatureList.back();

	if (name.caseCompare("featureID") == 0)
		fw.featureID = value;
	else if (name.caseCompare("usageCount") == 0)
		fw.usageCount = atoi(value.data());
}

void amx::Panel::setPaletteList(const strings::String& name, const strings::String& value)
{
	sysl->TRACE(Syslog::MESSAGE, std::string("Panel::setPaletteList(const strings::String& name, const strings::String& value)"));

	if (Project.paletteList.size() == 0)
		return;

    PALETTE_T& pa = Project.paletteList.back();

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
	sysl->TRACE(Syslog::MESSAGE, std::string("Panel::getDate(const strings::String& dat, DateTime& dt)"));

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
	sysl->TRACE(String("Panel::getDate: ")+dt.toString());
	return dt;
}
