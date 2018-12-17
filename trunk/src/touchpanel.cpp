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
	readPages();
}

TouchPanel::~TouchPanel()
{
	sysl->TRACE(Syslog::EXIT, String("TouchPanel::TouchPanel()"));
}

String TouchPanel::getPage(int id)
{
	sysl->TRACE(String("TouchPanel::getPage(int id)"));

	for (size_t i = 0; i < pages.size(); i++)
	{
		if (pages[i].isOk() && pages[i].getPageID() == id)
			return pages[i].getWebCode();
	}

	return "";
}

String TouchPanel::getPage(const String& name)
{
	sysl->TRACE(String("TouchPanel::getPage(const String& name)"));

	for (size_t i = 0; i < pages.size(); i++)
	{
		if (pages[i].isOk() && pages[i].getPageName().compare(name) == 0)
			return pages[i].getWebCode();
	}

	return "";
}

String TouchPanel::getStartPage()
{
	String pg, styles;

	// The head of the document
	pg = "<!DOCTYPE html>\n";
	pg += "<html>\n<head>\n<meta charset=\"UTF-8\">\n";
	pg += "<title>AMX Panel</title>\n";
	// The styles
	// Style: background
	pg += "<style>\n";
	pg += "main_pg_bg {\n";
	pg += "  ";
	pg += "</head>\n";
	// The page body
	pg += "<body>\n";
}

void TouchPanel::readPages()
{
	sysl->TRACE(String("TouchPanel::readPages()"));
	vector<String> pgs = getPageFileNames();

	for (size_t i = 0; i < pgs.size(); i++)
	{
		Page *p = new Page(pgs[i]);
		p->setPaletteFile(getProject().supportFileList.colorFile);
		p->setParentSize(getProject().panelSetup.screenWidth, getProject().panelSetup.screenHeight);
		pages.push_back(*p);
		delete p;
	}
}
