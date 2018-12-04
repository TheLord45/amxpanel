#ifndef __PANELSTRUCT_H__
#define __PANELSTRUCT_H__

#include <iostream>
#include "datetime.h"
#include "string.h"

namespace Panel
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
    };
}

#endif

