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

#ifndef __PANEL_H__
#define __PANEL_H__

#include "panelstruct.h"
#include "palette.h"
#include "icon.h"
#include "fontlist.h"

namespace amx
{
    class Panel
    {
        public:
            Panel();
            Panel(const PROJECT& prj, Palette *pPalet, Icon *pIco, FontList *pFL);
            ~Panel();

            bool isOk() { return status; }
            std::vector<strings::String> getPageFileNames();

        protected:
            PROJECT_T& getProject() { return Project; }
            FontList *getFontList() { return pFontLists; }
            Palette *getPalettes() { return pPalettes; }

        private:
            void readProject();
            void setVersionInfo(const strings::String& name, const strings::String& value);
            void setProjectInfo(const strings::String& name, const strings::String& value, const strings::String& attr);
            void setSupportFileList(const strings::String& name, const strings::String& value);
            void setPanelSetup(const strings::String& name, const strings::String& value);
            void setPageList(const strings::String& name, const strings::String& value);
            void setResourceList(const strings::String& name, const strings::String& value, const strings::String& attr);
            void setFwFeatureList(const strings::String& name, const strings::String& value);
            void setPaletteList(const strings::String& name, const strings::String& value);
            DateTime& getDate(const strings::String& dat, DateTime& dt);

            PROJECT_T Project;
            Palette *pPalettes;
            Icon *pIcons;
            FontList *pFontLists;
            bool status;
    };
}

#endif
