#ifndef __PANELSTRUCT_H__
#define __PANELSTRUCT_H__

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
    struct VERSION
    {
        int formatVersion;
        int graphicVersion;
        int fileVersion;
        int designVersion;
    };

    /**
     * Contains the infomations about the project.
     * This is part of the struct PROJECT.
     */
    struct PROJECT_INFO
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
    };
 
    struct SUPPORT_FILE_LIST
    {
        strings::String mapFile;
        strings::String colorFile;
        strings::String fontFile;
        strings::String themeFile;
        strings::String iconFile;
        strings::String externalButtonFile;
    };

    struct PANEL_SETUP
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
    };

    struct PAGE_ENTRY
    {
        strings::String name;
        int pageID;
        strings::String file;
        strings::String group;
        int isValid;
        int popupType;
    };

    struct PAGE_LIST
    {
        strings::String type;
        std::vector<PAGE_ENTRY> pageList;
    };

    struct RESOURCE
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
    };

    struct RESOURCE_LIST
    {
        strings::String type;
        std::vector<RESOURCE> ressource;
    };

    struct FEATURE
    {
        strings::String featureID;
        int usageCount;
    };

    struct PALETTE
    {
        strings::String name;
        strings::String file;
        int paletteID;
    };

    /**
     * This is the main structure.
     * This structure contains all other structures. Many of them are
     * defined as chains.
     */
    struct PROJECT
    {
        VERSION version;
        PROJECT_INFO projectInfo;
        SUPPORT_FILE_LIST supportFileList;
        PANEL_SETUP panelSetup;
        std::vector<PAGE_LIST> pageLists;
        std::vector<RESOURCE_LIST> resourceLists;
        std::vector<FEATURE> fwFeatureList;
        std::vector<PALETTE> paletteList;
    };

    struct SR
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
    };

    struct BUTTON
    {
        strings::String type;
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
        int ch;
        int rl;
        int rh;
        strings::String pfType;
        strings::String pfName;
        std::vector<SR> sr;
    };

    struct PAGE
    {
        strings::String type;
        int pageID;
        strings::String name;
        int width;
        int height;
        std::string group;
        int showTime;
        int hideTime;
        std::vector<BUTTON> buttons;
        SR sr;
    };
}

#endif

