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

#ifndef __FONTLIST_H__
#define __FONTLIST_H__

#include <vector>
#include "strings.h"

namespace amx
{
    typedef struct FONT
    {
        int number;
        strings::String file;
        int fileSize;
        int faceIndex;
        strings::String name;
        strings::String subfamilyName;
        strings::String fullName;
        int size;
        int usageCount;
    }FONT_T;

    class FontList
    {
        public:
            FontList(const strings::String& file);

        private:
            std::vector<FONT_T> fontList;
    };
}

#endif
