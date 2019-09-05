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

#ifndef __PANELSTRUCT_H__
#define __PANELSTRUCT_H__

#include <string>
#include <iostream>
#include <vector>
#include "datetime.h"
#include "strings.h"

namespace amx
{
    /**
     * Contains the version information.
     * This is part of the struct PROJECT.
     */
    typedef struct VERSION
    {
        int formatVersion;
        int graphicsVersion;
        int fileVersion;
        int designVersion;
    }VERSION_T;

    /**
     * Contains the infomations about the project.
     * This is part of the struct PROJECT.
     */
	typedef struct PROJECT_INFO_T
	{
		std::string protection;
		bool encrypted;
		std::string password;
		std::string panelType;
		std::string fileRevision;
		std::string dealerID;
		std::string jobName;
		std::string salesOrder;
		std::string purchaseOrder;
		std::string jobComment;
		std::string designerID;
		DateTime creationDate;
		DateTime revisionDate;
		DateTime lastSaveDate;
		std::string fileName;
		std::string colorChoice;
		int specifyPortCount;
		int specifyChanCount;
/*
		PROJECT_INFO_T(const PROJECT_INFO_T& pi)
			: protection(pi.protection),
			  encrypted(pi.encrypted),
			  password(pi.password),
			  panelType(pi.panelType),
			  fileRevision(pi.fileRevision),
			  dealerID(pi.dealerID),
			  jobName(pi.jobName),
			  salesOrder(pi.salesOrder),
			  purchaseOrder(pi.purchaseOrder),
			  jobComment(pi.jobComment),
			  designerID(pi.designerID),
			  creationDate(pi.creationDate),
			  revisionDate(pi.revisionDate),
			  lastSaveDate(pi.lastSaveDate),
			  fileName(pi.fileName),
			  colorChoice(pi.colorChoice),
			  specifyPortCount(pi.specifyPortCount),
			  specifyChanCount(pi.specifyChanCount)
		{} */
	}PROJECT_INFO_T;

    typedef struct SUPPORT_FILE_LIST
    {
        std::string mapFile;
        std::string colorFile;
        std::string fontFile;
        std::string themeFile;
        std::string iconFile;
        std::string externalButtonFile;
    }SUPPORT_FILE_LIST_T;

    typedef struct PANEL_SETUP
    {
        int portCount;
        int setupPort;
        int addressCount;
        int channelCount;
        int levelCount;
        std::string powerUpPage;
        std::vector<std::string> powerUpPopup;
        int feedbackBlinkRate;
        std::string startupString;
        std::string wakeupString;
        std::string sleepString;
        std::string standbyString;
        std::string shutdownString;
        std::string idlePage;
        int idleTimeout;
        int extButtonsKey;
        int screenWidth;
        int screenHeight;
        int screenRefresh;
        int screenRotate;
        std::string screenDescription;
        int pageTracking;
        int cursor;
        int brightness;
        int lightSensorLevelPort;
        int lightSensorLevelCode;
        int lightSensorChannelPort;
        int lightSensorChannelCode;
        int motionSensorChannelPort;
        int motionSensorChannelCode;
        int batteryLevelPort;
        int batteryLevelCode;
        int irPortAMX38Emit;
        int irPortAMX455Emit;
        int irPortAMX38Recv;
        int irPortAMX455Recv;
        int irPortUser1;
        int irPortUser2;
        int cradleChannelPort;
        int cradleChannelCode;
        std::string uniqueID;
        std::string appCreated;
        int buildNumber;
        std::string appModified;
        int buildNumberMod;
        std::string buildStatusMod;
        int activePalette;
        int marqueeSpeed;
        int setupPagesProject;
        int voipCommandPort;
    }PANEL_SETUP_T;

    typedef struct PAGE_ENTRY
    {
        std::string name;
        int pageID;
        std::string file;
        std::string group;
        int isValid;
        int popupType;

        void clear()
        {
            name.clear();
            pageID = 0;
            file.clear();
            group.clear();
            isValid = 0;
            popupType = 0;
        }
    }PAGE_ENTRY_T;

    typedef struct PAGE_LIST
    {
        std::string type;
        std::vector<PAGE_ENTRY_T> pageList;
    }PAGE_LIST_T;

    typedef struct RESOURCE
    {
        std::string name;
        std::string protocol;
        std::string user;
        std::string password;
        bool encrypted;
        std::string host;
        std::string path;
        std::string file;
        int refresh;

        void clear()
        {
            name.clear();
            protocol.clear();
            user.clear();
            password.clear();
            encrypted = false;
            host.clear();
            path.clear();
            file.clear();
            refresh = 0;
        }
    }RESOURCE_T;

    typedef struct RESOURCE_LIST
    {
        std::string type;
        std::vector<RESOURCE_T> ressource;
    }RESOURCE_LIST_T;

    typedef struct FEATURE
    {
        std::string featureID;
        int usageCount;
    }FEATURE_T;

    typedef struct PALETTE
    {
        std::string name;
        std::string file;
        int paletteID;
    }PALETTE_T;

	/**
	 * This is the main structure.
	 * This structure contains all other structures. Many of them are
	 * defined as chains.
	 */
	typedef struct PROJECT
	{
		VERSION_T version;
		PROJECT_INFO_T projectInfo;
		SUPPORT_FILE_LIST_T supportFileList;
		PANEL_SETUP_T panelSetup;
		std::vector<PAGE_LIST_T> pageLists;
		std::vector<RESOURCE_LIST_T> resourceLists;
		std::vector<FEATURE_T> fwFeatureList;
		std::vector<PALETTE_T> paletteList;
/*
		PROJECT(const PROJECT& p)
			: version(p.version),
			  projectInfo(p.projectInfo),
			  supportFileList(p.supportFileList),
			  panelSetup(p.panelSetup),
			  pageLists(p.pageLists),
			  resourceLists(p.resourceLists),
			  fwFeatureList(p.fwFeatureList),
			  paletteList(p.paletteList)
		{} */
	}PROJECT_T;

	enum TEXT_ORIENTATION
	{
		ORI_ABSOLUT,
		ORI_TOP_LEFT,
		ORI_TOP_MIDDLE,
		ORI_TOP_RIGHT,
		ORI_CENTER_LEFT,
		ORI_CENTER_MIDDLE,		// default
		ORI_CENTER_RIGHT,
		ORI_BOTTOM_LEFT,
		ORI_BOTTOM_MIDDLE,
		ORI_BOTTOM_RIGHT
	};

	typedef struct SR
	{
		int number;
		std::string _do;		// Order on how to show a multistate bargraph (010203...)
		std::string bs;			// Frame type (circle, ...)
		std::string mi;			// Chameleon image
		int mi_width;			// Width of image
		int mi_height;			// Height of image
		std::string cb;			// Border color
		std::string cf;			// Fill color
		std::string ct;			// Text Color
		std::string ec;			// Text effect color
		std::string bm;			// bitmap file name
		std::string sd;			// Sound file to play
		int bm_width;			// Width of image
		int bm_height;			// Height of image
		bool dynamic;			// TRUE = moving image
		int sb;					// Index to external graphics download
		int ii;					// Icon index number
		int ji;					// Icon style / position like "jt"
		int jb;					// Image position (center, left, ...)
		int ix;					// bitmap X position
		int iy;					// bitmap Y position
		int fi;					// Font index?
		std::string te;			// Text
		TEXT_ORIENTATION jt;	// Text orientation
		int tx;					// Text X position
		int ty;					// Text Y position
		int ww;					// line break when 1
		int et;					// Text effect (^TEF)
		int oo;					// Over all opacity

		void clear()
		{
			number = 0;
			_do.clear();
			bs.clear();
			mi.clear();
			cb.clear();
			cf.clear();
			ct.clear();
			ec.clear();
			bm.clear();
			sd.clear();
			dynamic = false;
			ii = 0;
			sb = 0;
			ji = 5;
			jb = 5;
			ix = 0;
			iy = 0;
			fi = 0;
			te.clear();
			jt = ORI_CENTER_MIDDLE;
			tx = 0;
			ty = 0;
			ww = 0;
			et = 0;
			oo = 0x00ff;		// by default the button is fully opaque.
			mi_width = 0;
			mi_height = 0;
			bm_width = 0;
			bm_height = 0;
		}
	}SR_T;

    enum BUTTONTYPE
    {
        NONE,
        GENERAL,
        MULTISTATE_GENERAL,
        BARGRAPH,
        MULTISTATE_BARGRAPH,
        JOISTICK,
        TEXT_INPUT,
        COMPUTER_CONTROL,
        TAKE_NOTE,
        SUBPAGE_VIEW
    };

	enum FEEDBACK
	{
		FB_NONE,
		FB_CHANNEL,
		FB_INV_CHANNEL,		// inverted channel
		FB_ALWAYS_ON,
		FB_MOMENTARY,
		FB_BLINK
	};

	typedef struct PUSH_FUNC
	{
		std::string pfType;	// command to execute when button was pushed
		std::string pfName;	// Name of popup
	}PUSH_FUNC_T;

	typedef struct BUTTON
	{
		BUTTONTYPE type;
		int bi;					// button ID
		std::string na;			// name
		std::string bd;			// Description --> ignored
		int lt;					// pixel from left
		int tp;					// pixel from top
		int wt;					// width
		int ht;					// height
		int zo;					// Z-Order
		std::string hs;			// bounding, ...
		std::string bs;			// Frame type (circle, ...)
		FEEDBACK fb;			// momentary, ...
		int ap;					// Address port
		int ad;					// Address channel
		int ch;					// Channel number
		int cp;					// Channel port
		int lp;					// Level port
		int lv;					// Level code
		std::string dr;			// Level "horizontal" or "vertical"
		int va;
		int stateCount;			// State count with multistate buttons
		int rm;					// State count with multistate buttons?
		int nu;					// Animate time up
		int nd;					// Animate time down
		int ar;					// Auto repeat (1 = true)
		int ru;					// Animate time up (bargraph)
		int rd;					// Animate time down (bargraph)
		int lu;					// Animate time up (Bargraph)
		int ld;					// Animate time down (Bargraph)
		int rv;
		int rl;					// Range low
		int rh;					// Range high
		int ri;					// Bargraph inverted (0 = normal, 1 = inverted)
		int rn;					// Bargraph: Range drag increment
		std::string _if;		// Bargraph function: empty = display only, active, active centering, drag, drag centering
		std::string sd;			// Name/Type of slider for a bargraph
		std::string sc;			// Color of slider (for bargraph)
		int mt;					// Length of text area (0 = 2000)
		std::string dt;			// "multiple" textarea has multiple lines, else single line
		std::string im;			// Input mask of a text area
		std::string op;			// String the button send
		std::vector<PUSH_FUNC_T> pushFunc;	// Push functions: This are executed on button press
		std::vector<SR_T> sr;

		void clear()
		{
			bi = 0;
			bd.clear();
			na.clear();
			lt = 0;
			tp = 0;
			wt = 0;
			ht = 0;
			zo = 0;
			hs.clear();
			bs.clear();
			fb = FB_NONE;
			ap = 1;
			ad = 0;
			ch = 0;
			cp = 1;
			lp = 1;
			va = 0;
			rv = 0;
			rl = 0;
			rh = 0;
			lv = 0;
			stateCount = 0;
			rm = 0;
			nu = 0;
			nd = 0;
			ar = 0;
			ru = 0;
			rd = 0;
			lu = 0;
			ld = 0;
			rn = 0;
			ri = 0;
			mt = 0;
			sd.clear();
			sc.clear();
			dt.clear();
			im.clear();
			op.clear();
			_if.clear();
			pushFunc.clear();
			sr.clear();
		}
	}BUTTON_T;

    enum PAGETYPE
    {
        PNONE,
        PAGE,
        SUBPAGE
    };

	enum SHOWEFFECT
	{
		SE_NONE,
		SE_FADE,
		SE_SLIDE_LEFT,
		SE_SLIDE_RIGHT,
		SE_SLIDE_TOP,
		SE_SLIDE_BOTTOM,
		SE_SLIDE_LEFT_FADE,
		SE_SLIDE_RIGHT_FADE,
		SE_SLIDE_TOP_FADE,
		SE_SLIDE_BOTTOM_FADE
	};

	typedef struct PAGE
	{
		PAGETYPE type;						// Type: PAGE | SUBPAGE
		int pageID;							// Unique ID of popup/page
		std::string name;					// The name of the popup/page
		int left;							// Left position of popup
		int top;							// Top position of popup
		int width;							// Width of popup
		int height;							// Height of popup
		int modal;							// 0 = Popup/Page = non modal
		std::string group;					// Name of the group the popup belongs
		int timeout;						// Time after the popup hides in 1/10 seconds
		SHOWEFFECT showEffect;				// The effect when the popup is shown
		int showTime;						// The time reserved for the show effect
		SHOWEFFECT hideEffect;				// The effect when the popup hides
		int hideTime;						// The time reserved for the hide effect
		std::vector<BUTTON_T> buttons;		// Array of elements
		std::vector<SR_T> sr;				// Page/Popup description
	}PAGE_T;

	typedef struct MAP
	{
		int p;			// port number
		int c;			// channel number
		int ax;
		int pg;			// page number
		int bt;			// button number
		std::string pn;	// page name
		std::string bn;	// button name
	}MAP_T;

	// Images
	typedef struct MAP_BM
	{
		std::string i;	// name
		int id;
		int rt;
		int pg;
		int bt;
		int st;
		int sl;
		std::string pn;
		std::string bn;
	}MAP_BM_T;

    typedef struct MAP_PM
    {
        int a;
        std::string t;	// Text
        int pg;			// page number
        int bt;			// button number
        std::string pn;	// page name
        std::string bn;	// button name
    }MAP_PM_T;

    typedef struct MAPS
    {
        std::vector<MAP_T> map_cm;
        std::vector<MAP_T> map_am;
        std::vector<MAP_T> map_lm;
        std::vector<MAP_BM_T> map_bm;       // Images
        std::vector<std::string> map_sm;// sound file names
        std::vector<MAP_T> map_strm;        // System resources
        std::vector<MAP_PM_T> map_pm;       // Button -> text
    }MAPS_T;
}

#endif

