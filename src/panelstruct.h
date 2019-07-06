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
    typedef struct PROJECT_INFO
    {
        strings::String protection;
        bool encrypted;
        strings::String password;
        strings::String panelType;
        strings::String fileRevision;
        strings::String dealerID;
        strings::String jobName;
        strings::String salesOrder;
        strings::String purchaseOrder;
        strings::String jobComment;
        strings::String designerID;
        DateTime creationDate;
        DateTime revisionDate;
        DateTime lastSaveDate;
        strings::String fileName;
        strings::String colorChoice;
        int specifyPortCount;
        int specifyChanCount;
    }PROJECT_INFO_T;

    typedef struct SUPPORT_FILE_LIST
    {
        strings::String mapFile;
        strings::String colorFile;
        strings::String fontFile;
        strings::String themeFile;
        strings::String iconFile;
        strings::String externalButtonFile;
    }SUPPORT_FILE_LIST_T;

    typedef struct PANEL_SETUP
    {
        int portCount;
        int setupPort;
        int addressCount;
        int channelCount;
        int levelCount;
        strings::String powerUpPage;
        std::vector<strings::String> powerUpPopup;
        int feedbackBlinkRate;
        strings::String startupString;
        strings::String wakeupString;
        strings::String sleepString;
        strings::String standbyString;
        strings::String shutdownString;
        strings::String idlePage;
        int idleTimeout;
        int extButtonsKey;
        int screenWidth;
        int screenHeight;
        int screenRefresh;
        int screenRotate;
        strings::String screenDescription;
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
        strings::String uniqueID;
        strings::String appCreated;
        int buildNumber;
        strings::String appModified;
        int buildNumberMod;
        strings::String buildStatusMod;
        int activePalette;
        int marqueeSpeed;
        int setupPagesProject;
        int voipCommandPort;
    }PANEL_SETUP_T;

    typedef struct PAGE_ENTRY
    {
        strings::String name;
        int pageID;
        strings::String file;
        strings::String group;
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
        strings::String type;
        std::vector<PAGE_ENTRY_T> pageList;
    }PAGE_LIST_T;

    typedef struct RESOURCE
    {
        strings::String name;
        strings::String protocol;
        strings::String user;
        strings::String password;
        bool encrypted;
        strings::String host;
        strings::String path;
        strings::String file;
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
        strings::String type;
        std::vector<RESOURCE_T> ressource;
    }RESOURCE_LIST_T;

    typedef struct FEATURE
    {
        strings::String featureID;
        int usageCount;
    }FEATURE_T;

    typedef struct PALETTE
    {
        strings::String name;
        strings::String file;
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
		strings::String _do;	// Order on how to show a multistate bargraph (010203...)
		strings::String bs;     // Frame type (circle, ...)
		strings::String mi;     // Chameleon image
		int mi_width;           // Width of image
		int mi_height;          // Height of image
		strings::String cb;     // Border color
		strings::String cf;     // Fill color
		strings::String ct;     // Text Color
		strings::String ec;     // Text effect color
		strings::String bm;     // bitmap file name
		int bm_width;           // Width of image
		int bm_height;          // Height of image
		bool dynamic;			// TRUE = moving image
		int sb;					// Index to external graphics download
		int ii;					// Icon index number
		int ji;					// Icon style / position like "jt"
		int jb;
		int ix;                 // bitmap X position
		int iy;                 // bitmap Y position
		int fi;                 // Font index?
		strings::String te;     // Text
		TEXT_ORIENTATION jt;	// Text orientation
		int tx;                 // Text X position
		int ty;                 // Text Y position
		int ww;                 // line break when 1
		int et;

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
		strings::String pfType;	// command to execute when button was pushed
		strings::String pfName;	// Name of popup
	}PUSH_FUNC_T;

	typedef struct BUTTON
	{
		BUTTONTYPE type;
		int bi;                	// button ID
		strings::String na;		// name
		int lt;					// pixel from left
		int tp;					// pixel from top
		int wt;                	// width
		int ht;                	// height
		int zo;                	// Z-Order
		strings::String hs;    	// bounding, ...
		strings::String bs;    	// Frame type (circle, ...)
		FEEDBACK fb;			// momentary, ...
		int ap;					// Address port
		int ad;					// Address channel
		int ch;					// Channel number
		int cp;					// Channel port
		int lp;					// Level port
		int lv;					// Level code
		strings::String dr;		// Level "horizontal" or "vertical"
		int va;
		int stateCount;			// State count with multistate buttons
		int rm;					// State count with multistate buttons?
		int nu;					// Animate time up
		int nd;					// Animate time down
		int ar;					// Auto repeat (1 = true)
		int ru;					// Animate time up (again)
		int rd;					// Animate time down (again)
		int rv;
		int rl;					// Range low
		int rh;					// Range high
		std::vector<PUSH_FUNC_T> pushFunc;	// Push functions: This are executed on button press
		std::vector<SR_T> sr;

		void clear()
		{
			bi = 0;
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
        PAGETYPE type;
        int pageID;
        strings::String name;
        int left;
        int top;
        int width;
        int height;
        strings::String group;
        SHOWEFFECT showEffect;
        int showTime;
        SHOWEFFECT hideEffect;
        int hideTime;
        std::vector<BUTTON_T> buttons;
        std::vector<SR_T> sr;
    }PAGE_T;

    typedef struct MAP
    {
        int p;              // port number
        int c;              // channel number
        int ax;
        int pg;             // page number
        int bt;             // button number
        strings::String pn; // page name
        strings::String bn; // button name
    }MAP_T;

    // Images
    typedef struct MAP_BM
    {
        strings::String i;  // name
        int id;
    }MAP_BM_T;

    typedef struct MAP_PM
    {
        int a;
        strings::String t;  // Text
        int pg;             // page number
        int bt;             // button number
        strings::String pn; // page name
        strings::String bn; // button name
    }MAP_PM_T;

    typedef struct MAPS
    {
        std::vector<MAP_T> map_cm;
        std::vector<MAP_T> map_am;
        std::vector<MAP_T> map_lm;
        std::vector<MAP_BM_T> map_bm;       // Images
        std::vector<strings::String> map_sm;// sound file names
        std::vector<MAP_T> map_strm;        // System resources
        std::vector<MAP_PM_T> map_pm;       // Button -> text
    }MAPS_T;
}

#endif

