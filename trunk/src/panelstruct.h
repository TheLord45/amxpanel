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

    typedef struct SR
    {
        int number;
        strings::String bs;     // Frame type (circle, ...)
        strings::String mi;     // Chameleon image
        strings::String cb;     // Background color
        strings::String cf;     // Frame color
        strings::String ct;     // Foreground Color
        strings::String ec;     // Fill color
        strings::String bm;     // bitmap file name
        bool dynamic;
        int sb;
        int ji;
        int jb;
        int ix;                 // bitmap X position
        int iy;                 // bitmap Y position
        int fi;                 // Font index?
        strings::String te;     // Text
        int jt;
        int tx;                 // Text X position
        int ty;                 // Text Y position
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

    typedef struct BUTTON
    {
        BUTTONTYPE type;
        int bi;                 // button ID
        strings::String na;     // name
        int lt;                 // pixel fron left
        int tp;                 // pixel from top
        int wt;                 // width
        int ht;                 // height
        int zo;                 // Z-Order
        strings::String hs;     // bounding, ...
        strings::String bs;     // Frame type (circle, ...)
        strings::String fb;     // momentary, ...
        int ap;
        int cp;
        int lp;
        int va;
        int rv;
        int ch;
        int rl;
        int rh;
        strings::String pfType;
        strings::String pfName;
        std::vector<SR_T> sr;
    }BUTTON_T;

    enum PAGETYPE
    {
        PNONE,
        PAGE,
        SUBPAGE
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
        std::string group;
        int showTime;
        int hideTime;
        std::vector<BUTTON_T> buttons;
        std::vector<SR_T> sr;
    }PAGE_T;
}

#endif

