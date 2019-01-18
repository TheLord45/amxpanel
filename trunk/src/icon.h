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

#ifndef __ICON_H__
#define __ICON_H__

#include <vector>
#include "strings.h"

namespace amx
{
    typedef struct ICON
    {
        int index;
        strings::String file;
    }ICON_T;

    class Icon
    {
        public:
            Icon(const strings::String& file);
            ~Icon();

            bool isOk() { return status; }
            strings::String getFileName(size_t idx);
            strings::String getFileFromID(int id);

        private:
            std::vector<ICON_T> icons;
            bool status;
    };
}

#endif
