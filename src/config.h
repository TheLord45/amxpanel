/*
 *   Copyright (C) 2018 by Andreas Theofilu (TheoSys) <andreas@theosys.at>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation version 3 of the License.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

// Which C++ version is supported?
#ifdef __SUNPRO_CC
    #define _CPP_STD 3
#elif defined(__GNUC__) && defined(__cplusplus)
    #if __cplusplus >= 201103L
        #define _CPP_STD 11
    #else
        #define _CPP_STD 3
    #endif
#elif defined(__cplusplus)
    #if __cplusplus == 199711L
        #define _CPP_STD 3
    #elif __cplusplus == 201103L
        #define _CPP_STD 11
    #else
        #define _CPP_STD 3
    #endif
#else
    #define _CPP_STD 3
#endif

// 32 or 64 bit environment?
// GCC
#if __GNUC__
    #if __x86_64__ || __ppc64__ || __amd64
        #define ENVIRONMENT64
    #else
        #define ENVIRONMENT32
    #endif
#endif
// Oracle/Sun developer studio
#ifdef __SUNPRO_CC
    #if defined(_ILP32) || defined(__i386)
        #define ENVIRONMENT32
    #elif _LP64 || __amd64
        #define ENVIRONMENT64
    #endif
#endif

#endif  // __CONFIG_H__
