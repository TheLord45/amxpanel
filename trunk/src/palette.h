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

#ifndef __PALETTE_H__
#define __PALETTE_H__

#include <vector>
#include "strings.h"

namespace amx
{
    typedef struct PDATA
    {
        int index;
        strings::String name;
        unsigned long color;
        
        void clear()
        {
            index = 0;
            name.clear();
            color = 0;
        }
    }PDATA_T;

    class Palette
    {
        public:
            Palette(const strings::String& file);

            unsigned long getColor(size_t idx);
            unsigned long getColor(const strings::String& name);

        private:
            std::vector<PDATA_T> palette;
    };
}

#endif
