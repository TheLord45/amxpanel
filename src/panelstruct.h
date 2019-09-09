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
        int formatVersion{0};
        int graphicsVersion{0};
        int fileVersion{0};
        int designVersion{0};
    }VERSION_T;

    /**
     * Contains the infomations about the project.
     * This is part of the struct PROJECT.
     */
	typedef struct PROJECT_INFO_T
	{
		std::string protection;
		bool encrypted{false};
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
		int specifyPortCount{0};
		int specifyChanCount{0};
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
        int portCount{0};
        int setupPort{0};
        int addressCount{0};
        int channelCount{0};
        int levelCount{0};
        std::string powerUpPage;
        std::vector<std::string> powerUpPopup;
        int feedbackBlinkRate{0};
        std::string startupString;
        std::string wakeupString;
        std::string sleepString;
        std::string standbyString;
        std::string shutdownString;
        std::string idlePage;
        int idleTimeout{0};
        int extButtonsKey{0};
        int screenWidth{0};
        int screenHeight{0};
        int screenRefresh{0};
        int screenRotate{0};
        std::string screenDescription;
        int pageTracking{0};
        int cursor{0};
        int brightness{0};
        int lightSensorLevelPort{0};
        int lightSensorLevelCode{0};
        int lightSensorChannelPort{0};
        int lightSensorChannelCode{0};
        int motionSensorChannelPort{0};
        int motionSensorChannelCode{0};
        int batteryLevelPort{0};
        int batteryLevelCode{0};
        int irPortAMX38Emit{0};
        int irPortAMX455Emit{0};
        int irPortAMX38Recv{0};
        int irPortAMX455Recv{0};
        int irPortUser1{0};
        int irPortUser2{0};
        int cradleChannelPort{0};
        int cradleChannelCode{0};
        std::string uniqueID;
        std::string appCreated;
        int buildNumber{0};
        std::string appModified;
        int buildNumberMod{0};
        std::string buildStatusMod;
        int activePalette{0};
        int marqueeSpeed{0};
        int setupPagesProject{0};
        int voipCommandPort{0};
    }PANEL_SETUP_T;

    typedef struct PAGE_ENTRY
    {
        std::string name;
        int pageID{0};
        std::string file;
        std::string group;
        int isValid{0};
        int popupType{0};

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
        bool encrypted{false};
        std::string host;
        std::string path;
        std::string file;
        int refresh{0};

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
        int usageCount{0};
    }FEATURE_T;

    typedef struct PALETTE
    {
        std::string name;
        std::string file;
        int paletteID{0};
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
		int number{0};
		std::string _do;		// Order on how to show a multistate bargraph (010203...)
		std::string bs;			// Frame type (circle, ...)
		std::string mi;			// Chameleon image
		int mi_width{0};		// Width of image
		int mi_height{0};		// Height of image
		std::string cb;			// Border color
		std::string cf;			// Fill color
		std::string ct;			// Text Color
		std::string ec;			// Text effect color
		std::string bm;			// bitmap file name
		std::string sd;			// Sound file to play
		int bm_width{0};		// Width of image
		int bm_height{0};		// Height of image
		bool dynamic{false};	// TRUE = moving image
		int sb{0};				// Index to external graphics download
		int ii{0};				// Icon index number
		int ji{0};				// Icon style / position like "jt"
		int jb{0};				// Image position (center, left, ...)
		int ix{0};				// bitmap X position
		int iy{0};				// bitmap Y position
		int fi{0};				// Font index?
		std::string te;			// Text
		TEXT_ORIENTATION jt{ORI_CENTER_MIDDLE};	// Text orientation
		int tx{0};				// Text X position
		int ty{0};				// Text Y position
		int ww{0};				// line break when 1
		int et{0};				// Text effect (^TEF)
		int oo{0};				// Over all opacity

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
		int bi{0};				// button ID
		std::string na;			// name
		std::string bd;			// Description --> ignored
		int lt{0};				// pixel from left
		int tp{0};				// pixel from top
		int wt{0};				// width
		int ht{0};				// height
		int zo{0};				// Z-Order
		std::string hs;			// bounding, ...
		std::string bs;			// Frame type (circle, ...)
		FEEDBACK fb{FB_NONE};	// momentary, ...
		int ap{0};				// Address port
		int ad{0};				// Address channel
		int ch{0};				// Channel number
		int cp{0};				// Channel port
		int lp{0};				// Level port
		int lv{0};				// Level code
		std::string dr;			// Level "horizontal" or "vertical"
		int va{0};
		int stateCount{0};		// State count with multistate buttons
		int rm{0};				// State count with multistate buttons?
		int nu{0};				// Animate time up
		int nd{0};				// Animate time down
		int ar{0};				// Auto repeat (1 = true)
		int ru{0};				// Animate time up (bargraph)
		int rd{0};				// Animate time down (bargraph)
		int lu{0};				// Animate time up (Bargraph)
		int ld{0};				// Animate time down (Bargraph)
		int rv{0};
		int rl{0};				// Range low
		int rh{0};				// Range high
		int ri{0};				// Bargraph inverted (0 = normal, 1 = inverted)
		int rn{0};				// Bargraph: Range drag increment
		std::string _if;		// Bargraph function: empty = display only, active, active centering, drag, drag centering
		std::string sd;			// Name/Type of slider for a bargraph
		std::string sc;			// Color of slider (for bargraph)
		int mt{0};				// Length of text area (0 = 2000)
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
		int pageID{0};						// Unique ID of popup/page
		std::string name;					// The name of the popup/page
		int left{0};						// Left position of popup
		int top{0};							// Top position of popup
		int width{0};						// Width of popup
		int height{0};						// Height of popup
		int modal{0};						// 0 = Popup/Page = non modal
		std::string group;					// Name of the group the popup belongs
		int timeout{0};						// Time after the popup hides in 1/10 seconds
		SHOWEFFECT showEffect{SE_NONE};		// The effect when the popup is shown
		int showTime{0};					// The time reserved for the show effect
		SHOWEFFECT hideEffect{SE_NONE};		// The effect when the popup hides
		int hideTime{0};					// The time reserved for the hide effect
		std::vector<BUTTON_T> buttons;		// Array of elements
		std::vector<SR_T> sr;				// Page/Popup description
	}PAGE_T;

	typedef struct MAP
	{
		int p{0};		// port number
		int c{0};		// channel number
		int ax{0};
		int pg{0};		// page number
		int bt{0};		// button number
		std::string pn;	// page name
		std::string bn;	// button name
	}MAP_T;

	// Images
	typedef struct MAP_BM
	{
		std::string i;	// name
		int id{0};
		int rt{0};
		int pg{0};
		int bt{0};
		int st{0};
		int sl{0};
		std::string pn;
		std::string bn;
	}MAP_BM_T;

    typedef struct MAP_PM
    {
        int a{0};
        std::string t;	// Text
        int pg{0};		// page number
        int bt{0};		// button number
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

