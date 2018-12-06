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

#include <stdio.h>
#include "strings.h"
#include "regexp.h"

using namespace strings;

void SThrow(const String& err);
void SThrow(const std::string& err);
void SThrow(const char *err);

String::~String()
{
    clear();
}

String::String(const String& str)
{
    _len = _allocated = 0;
    _string = 0;
    _replacePos = 0;
    
    try
    {
        copyString(str._string, str._len);
    }
    catch(SCATCH(e))
    {
        throw;
    }
}

String::String(const String& str, size_t pos, size_t len)
{
    _len = _allocated = 0;
    _string = 0;
    _replacePos = 0;
    size_t l = std::string::npos;

    if (len != std::string::npos)
    {
        if (pos < str._len)
        {
            l = str._len - pos;
            
            if (l > len)
                l = len;
        }
    }
    else if (pos < str._len)
    {
        l = str._len - pos;
    }

    try
    {
        if (l != std::string::npos)
            copyString(str._string+pos, l);
    }
    catch(SCATCH(e))
    {
        throw;
    }
}

String::String(const char* s)
{
    _len = _allocated = 0;
    _string = 0;
    _replacePos = 0;
    // find length of string
    size_t len = 0;
    const char *p = s;
    
    while(*p != 0)
    {
        len++;
        p++;
    }

    try
    {
        copyString(s, len);
    }
    catch (SCATCH(e))
    {
        throw;
    }
}

String::String(const char* s, size_t n)
{
    _len = _allocated = 0;
    _string = 0;
    _replacePos = 0;
    
    try
    {
        copyString(s, n);
    }
    catch(SCATCH(e))
    {
        throw;
    }
}

String::String(size_t n, char c)
{
    _len = _allocated = 0;
    _string = 0;
    _replacePos = 0;
    
    try
    {
        getMemory(n);
    }
    catch (SCATCH(e))
    {
        throw;
    }

    for (size_t i = 0; i < n; i++)
        *(_string+i) = c;

    *(_string+n) = 0;
    _len = n;
}

String::String(const std::string& str)
{
    _len = _allocated = 0;
    _string = 0;
    _replacePos = 0;
    const char *s = str.c_str();
    size_t l = str.size();
    
    try
    {
        getMemory(l+1);
    }
    catch (SCATCH(e))
    {
        throw;
    }
    
    for (size_t i = 0; i < l; i++)
    {
        *(_string+i) = *s;
        s++;
    }
    
    *(_string+l) = 0;
    _len = l;
}

String::String(int i)
{
    _len = _allocated = 0;
    _string = 0;
    _replacePos = 0;
    char hv[64];
    int l = snprintf(hv, sizeof(hv), "%d", i);
    
    try
    {
        getMemory(l);
    }
    catch (SCATCH(e))
    {
        throw;
    }

    strncpy(_string, hv, l);
    *(_string+l) = 0;
    _len = l;
}

String::String(LONG l)
{
    _len = _allocated = 0;
    _string = 0;
    _replacePos = 0;
    char hv[64];
#ifdef ENVIRONMENT64
    int le = snprintf(hv, sizeof(hv), "%ld", l);
#else
    int le = snprintf(hv, sizeof(hv), "%lld", l);
#endif
    try
    {
        getMemory(le);
    }
    catch (SCATCH(e))
    {
        throw;
    }

    strncpy(_string, hv, le);
    *(_string+le) = 0;
    _len = le;
}

String::String(unsigned int i)
{
    _len = _allocated = 0;
    _string = 0;
    _replacePos = 0;
    char hv[64];
    int l = snprintf(hv, sizeof(hv), "%u", i);
    
    try
    {
        getMemory(l);
    }
    catch (SCATCH(e))
    {
        throw;
    }

    strncpy(_string, hv, l);
    *(_string+l) = 0;
    _len = l;
}

String::String(ULONG l)
{
    _len = _allocated = 0;
    _string = 0;
    _replacePos = 0;
    char hv[64];
#ifdef ENVIRONMENT64
    int le = snprintf(hv, sizeof(hv), "%lu", l);
#else
    int le = snprintf(hv, sizeof(hv), "%llu", l);
#endif
    try
    {
        getMemory(le);
    }
    catch (SCATCH(e))
    {
        throw;
    }

    strncpy(_string, hv, le);
    *(_string+le) = 0;
    _len = le;
}

String::String(float f)
{
    _len = _allocated = 0;
    _string = 0;
    _replacePos = 0;
    char hv[64];
    int l = snprintf(hv, sizeof(hv), "%f", f);
    
    try
    {
        getMemory(l);
    }
    catch (SCATCH(e))
    {
        throw;
    }

    strncpy(_string, hv, l);
    *(_string+l) = 0;
    _len = l;
}

String::String(double d)
{
    _len = _allocated = 0;
    _string = 0;
    _replacePos = 0;
    char hv[64];
    int l = snprintf(hv, sizeof(hv), "%lf", d);
    
    try
    {
        getMemory(l);
    }
    catch (SCATCH(e))
    {
        throw;
    }

    strncpy(_string, hv, l);
    *(_string+l) = 0;
    _len = l;
}

String::String(const Regexp& re)
{
    _len = _allocated = 0;
    _string = 0;
    _replacePos = 0;
    String s = re.getResult();
    swap(s);
}

bool strings::String::isAscii()
{
    if (_string == 0 || _len == 0)
        return false;

    for (size_t i = 0; i < _len; i++)
    {
        if (*(_string+i) < 0)
            return false;
    }

    return true;
}

bool strings::String::isAscii() const
{
    if (_string == 0 || _len == 0)
        return false;

    for (size_t i = 0; i < _len; i++)
    {
        if (*(_string+i) < 0)
            return false;
    }

    return true;
}

bool strings::String::isNumeric()
{
    if (_string == 0 || _len == 0)
        return false;

    bool dot = false;

    for (size_t i = 0; i < _len; i++)
    {
        if ((*(_string+i) >= '0' && *(_string+i) <= '9') || *(_string+i) == '.' ||
            (*(_string+i) == '-' && i == 0) || (*(_string+i) == '+' && i == 0))
        {
            if (*(_string+i) == '.' && !dot)
                dot = true;
            else if (*(_string+i) == '.')
                return false;

            continue;
        }
        else
            return false;
    }

    return true;
}

bool strings::String::isNumeric() const
{
    if (_string == 0 || _len == 0)
        return false;

    bool dot = false;

    for (size_t i = 0; i < _len; i++)
    {
        if ((*(_string+i) >= '0' && *(_string+i) <= '9') || *(_string+i) == '.' ||
            (*(_string+i) == '-' && i == 0) || (*(_string+i) == '+' && i == 0))
        {
            if (*(_string+i) == '.' && !dot)
                dot = true;
            else if (*(_string+i) == '.')
                return false;

            continue;
        }
        else
            return false;
    }

    return true;
}

std::istream& String::readFile(std::istream& stream)
{
    clear();

    try
    {
        for (; ;)
        {
            size_t al = 0;
            char *buf = getMemory(1, al, buf);
            stream.read(buf, al);
            appendString(buf, stream.gcount());
            delete[] buf;
            
            if (!stream.good())
                break;
        }
    }
    catch (SCATCH(e))
    {
        throw;
    }

    return stream;
}

void String::resize(size_t n)
{
    char *np = new char[n+1];

    for (size_t i = 0; i < n; i++)
    {
        if (i < _len)
            *(np+i) = *(_string+i);
    }

    *(np+n) = 0;
    
    clear();
    _string = np;
    _len = n;
    _allocated = n + 1;
}

void String::resize(size_t n, char c)
{
    char *np = new char[n+1];

    for (size_t i = 0; i < n; i++)
    {
        if (i < _len)
            *(np+i) = *(_string+i);
        else
            *(np+i) = c;
    }

    *(np+n) = 0;
    
    clear();
    _string = np;
    _len = n;
    _allocated = n + 1;
}

void String::clear()
{
    if (_string != 0 && _allocated > 0)
        delete[] _string;

    _string = 0;
    _len = 0;
    _allocated = 0;
    _replacePos = 0;
}

void String::push_back(char c)
{
    try
    {
        getMemory(_len+2);
    }
    catch (SCATCH(e))
    {
        throw;
    }

    *(_string+_len) = c;
    *(_string+_len+1) = 0;
    _len++;
}

void String::pop_back()
{
    if (_len == 0)
        return;

    _len--;
    *(_string+_len) = 0;
}

String& String::toLower()
{
    for (size_t i = 0; i < _len; i++)
        *(_string+i) = tolower(*(_string+i));

    return *this;
}

String& String::toUpper()
{
    for (size_t i = 0; i < _len; i++)
        *(_string+i) = toupper(*(_string+i));

    return *this;
}

String & String::assign(const String& str)
{
    try
    {
        copyString(str._string, str._len);
    }
    catch(SCATCH(e))
    {
        throw;
    }

    return *this;
}

std::istream& String::readToDelim(std::istream& stream, char delim)
{
    return _rdToDelim(stream, delim);
}

std::istream& String::readLine(std::istream& stream)
{
    return _rdToDelim(stream, '\n');
}

std::istream& String::readString(std::istream& stream)
{
    return _rdToDelim(stream, '\0');
}

String& String::remove(size_t pos)
{
    assertElement(pos);

    if (_string != 0 && pos < _len)
    {
        *(_string+pos) = 0;
        _len = pos;
    }

    return *this;
}

String & String::assign(const String& str, size_t pos, size_t len)
{
    size_t l = std::string::npos;
    assertElement(pos);

    if (len != std::string::npos)
    {
        if (pos < str._len)
        {
            l = str._len - pos;
            
            if (l > len)
                l = len;
        }
    }
    else if (pos < str._len)
    {
        l = str._len - pos;
    }

    try
    {
        if (l != std::string::npos)
            copyString(str._string+pos, l);
        else
            clear();
    }
    catch(SCATCH(e))
    {
        throw;
    }

    return *this;
}

String & String::assign(const char* s)
{
    // find length of string
    size_t len = 0;
    const char *p = s;

    if (s == 0)
        return *this;

    while(*p != 0)
    {
        len++;
        p++;
    }

    try
    {
        copyString(s, len);
    }
    catch(SCATCH(e))
    {
        throw;
    }

    return *this;
}

String & String::assign(const char* s, size_t n)
{
    try
    {
        copyString(s, n);
    }
    catch(SCATCH(e))
    {
        throw;
    }

    return *this;
}

String& String::assign(const std::string& str)
{
    size_t l = str.size();
    const char *s = str.c_str();
    
    try
    {
        copyString(s, l);
    }
    catch(SCATCH(e))
    {
        throw;
    }

    return *this;
}

String & String::assign(int i)
{
    clear();
    _string = new char[64];
    _len = snprintf(_string, 64, "%d", i);
    return *this;
}

String & String::assign(LONG l)
{
    clear();
    _string = new char[64];
    _len = snprintf(_string, 64, "%ld", l);
    return *this;
}

String & String::assign(unsigned int i)
{
    clear();
    _string = new char[64];
    _len = snprintf(_string, 64, "%u", i);
    return *this;
}

String & String::assign(ULONG l)
{
    clear();
    _string = new char[64];
    _len = snprintf(_string, 64, "%lu", l);
    return *this;
}

String & String::assign(float f)
{
    clear();
    _string = new char[64];
    _len = snprintf(_string, 64, "%f", f);
    return *this;
}

String & String::assign(double d)
{
    _string = new char[64];
    _len = snprintf(_string, 64, "%lf", d);
    return *this;
}

String& String::insert(size_t pos, const String& str)
{
    assertElement(pos);
    _insert(pos, str._string, str._len);
    return *this;
}

String& String::insert(size_t pos, const std::string& str)
{
    assertElement(pos);
    _insert(pos, str.c_str(), str.size());
    return *this;
}

String& String::insert(size_t pos, const char* s)
{
    assertElement(pos);
    _insert(pos, s, strlen(s));
    return *this;
}

String& String::arg(const String& str)
{
    size_t pos;
    String find = "%";
    _replacePos++;
    find.append(_replacePos);

    while ((pos = findOf(find)) != std::string::npos)
        replace(find, str);
    
    return *this;
}

String& String::arg(const std::string& str)
{
    return arg(String(str));
}

String& String::arg(const char *str)
{
    return arg(String(str));
}

String& String::arg(const char c)
{
    return arg(String(c));
}

String& String::arg(const int i)
{
    return arg(String(i));
}

String& String::arg(const unsigned int i)
{
    return arg(String(i));
}

String& String::arg(const LONG l)
{
    return arg(String(l));
}

String& String::arg(const ULONG l)
{
    return arg(String(l));
}

String& String::arg(const float f)
{
    return arg(String(f));
}

String& String::arg(const double f)
{
    return arg(String(f));
}

String& String::append(const String& str)
{
    try
    {
        appendString(str._string, str._len);
    }
    catch(SCATCH(e))
    {
        throw;
    }

    return *this;
}

String & String::append(const String& str) const
{
    try
    {
        appendString(str._string, str._len);
    }
    catch(SCATCH(e))
    {
        throw;
    }

    return *const_cast<String*>(this);
}

String& String::append(const String& str, size_t pos, size_t len)
{
    size_t l = std::string::npos;
    assertElement(pos);

    if (len != std::string::npos)
    {
        if (pos < str._len)
        {
            l = str._len - pos;
            
            if (l > len)
                l = len;
        }
    }
    else if (pos < str._len)
    {
        l = str._len - pos;
    }

    try
    {
        if (l != std::string::npos)
            appendString(str._string+pos, l);
        else if (_string != 0 && _len != 0)
        {
            delete _string;
            _len = 0;
        }
    }
    catch(SCATCH(e))
    {
        throw;
    }
    
    return *this;
}

String& String::append(const String& str, size_t pos, size_t len) const
{
    size_t l = std::string::npos;
    assertElement(pos);
    String *o = const_cast<String*>(this);

    if (len != std::string::npos)
    {
        if (pos < str._len)
        {
            l = str._len - pos;
            
            if (l > len)
                l = len;
        }
    }
    else if (pos < str._len)
    {
        l = str._len - pos;
    }

    try
    {
        if (l != std::string::npos)
            appendString(str._string+pos, l);
        else if (_string != 0 && _len != 0)
        {
            delete _string;
            o->_len = 0;
        }
    }
    catch(SCATCH(e))
    {
        throw;
    }
    
    return *const_cast<String*>(this);
}

String& String::append(const char* s)
{
    // find length of string
    size_t len = 0;
    const char *p = s;
    
    while(*p != 0)
    {
        len++;
        p++;
    }

    try
    {
        appendString(s, len);
    }
    catch(SCATCH(e))
    {
        throw;
    }

    return *this;
}

String& String::append(const char* s) const
{
    // find length of string
    size_t len = 0;
    const char *p = s;
    
    while(*p != 0)
    {
        len++;
        p++;
    }

    try
    {
        appendString(s, len);
    }
    catch(SCATCH(e))
    {
        throw;
    }

    return *const_cast<String*>(this);
}

String& String::append(const char* s, size_t n)
{
    try
    {
        appendString(s, n);
    }
    catch(SCATCH(e))
    {
        throw;
    }

    return *this;
}

String& String::append(const char* s, size_t n) const
{
    try
    {
        appendString(s, n);
    }
    catch(SCATCH(e))
    {
        throw;
    }

    return *const_cast<String*>(this);
}

String& String::append(const std::string& str)
{
    size_t l = str.size();
    const char *s = str.c_str();
    
    try
    {
        appendString(s, l);
    }
    catch(SCATCH(e))
    {
        throw;
    }

    return *this;
}

String& String::append(const std::string& str) const
{
    size_t l = str.size();
    const char *s = str.c_str();
    
    try
    {
        appendString(s, l);
    }
    catch(SCATCH(e))
    {
        throw;
    }

    return *const_cast<String*>(this);
}

String& String::append(char c)
{
    char np[2];
    int len = 1;
    np[0] = c;
    np[1] = 0;
    
    try
    {
        appendString(np, len);
    }
    catch(SCATCH(e))
    {
        throw;
    }

    return *this;
}

String& String::append(char c) const
{
    char np[2];
    int len = 1;
    np[0] = c;
    np[1] = 0;
    
    try
    {
        appendString(np, len);
    }
    catch(SCATCH(e))
    {
        throw;
    }

    String *o = const_cast<String*>(this);
    return *o;
}

String & String::append(int i)
{
    char *np = new char[64];
    int len = snprintf(np, 64, "%d", i);
    
    try
    {
        appendString(np, len);
    }
    catch (SCATCH(e))
    {
        throw;
    }

    delete[] np;
    return *this;
}

String & String::append(int i) const
{
    char *np = new char[64];
    String *o = const_cast<String*>(this);
    int len = snprintf(np, 64, "%d", i);
    
    try
    {
        appendString(np, len);
    }
    catch (SCATCH(e))
    {
        throw;
    }

    delete[] np;
    return *o;
}

String & String::append(LONG l)
{
    char *np = new char[64];
    int len = snprintf(np, 64, "%ld", l);
    
    try
    {
        appendString(np, len);
    }
    catch(SCATCH(e))
    {
        throw;
    }

    delete[] np;
    return *this;
}

String & String::append(LONG l) const
{
    char *np = new char[64];
    String *o = const_cast<String*>(this);
    int len = snprintf(np, 64, "%ld", l);
    
    try
    {
        appendString(np, len);
    }
    catch(SCATCH(e))
    {
        throw;
    }

    delete[] np;
    return *o;
}

String & String::append(unsigned int i)
{
    char *np = new char[64];
    int len = snprintf(np, 64, "%u", i);
    
    try
    {
        appendString(np, len);
    }
    catch(SCATCH(e))
    {
        throw;
    }

    delete[] np;
    return *this;
}

String & String::append(unsigned int i) const
{
    char *np = new char[64];
    String *o = const_cast<String*>(this);
    int len = snprintf(np, 64, "%u", i);
    
    try
    {
        appendString(np, len);
    }
    catch(SCATCH(e))
    {
        throw;
    }

    delete[] np;
    return *o;
}

String & String::append(ULONG l)
{
    char *np = new char[64];
    int len = snprintf(np, 64, "%lu", l);
    
    try
    {
        appendString(np, len);
    }
    catch(SCATCH(e))
    {
        throw;
    }

    delete[] np;
    return *this;
}

String & String::append(ULONG l) const
{
    char *np = new char[64];
    String *o = const_cast<String*>(this);
    int len = snprintf(np, 64, "%lu", l);
    
    try
    {
        appendString(np, len);
    }
    catch(SCATCH(e))
    {
        throw;
    }

    delete[] np;
    return *o;
}

String & String::append(float f)
{
    char *np = new char[64];
    int len = snprintf(np, 64, "%f", f);
    
    try
    {
        appendString(np, len);
    }
    catch(SCATCH(e))
    {
        throw;
    }

    delete[] np;
    return *this;
}

String & String::append(float f) const
{
    char *np = new char[64];
    String *o = const_cast<String*>(this);
    int len = snprintf(np, 64, "%f", f);
    
    try
    {
        appendString(np, len);
    }
    catch(SCATCH(e))
    {
        throw;
    }

    delete[] np;
    return *o;
}

String & String::append(double d)
{
    char *np = new char[64];
    int len = snprintf(np, 64, "%lf", d);
    
    try
    {
        appendString(np, len);
    }
    catch(SCATCH(e))
    {
        throw;
    }

    delete[] np;
    return *this;
}

String & String::append(double d) const
{
    char *np = new char[64];
    String *o = const_cast<String*>(this);
    int len = snprintf(np, 64, "%lf", d);
    
    try
    {
        appendString(np, len);
    }
    catch(SCATCH(e))
    {
        throw;
    }

    delete[] np;
    return *o;
}

int String::compare(const String& str)
{
    return _comp(_string, str._string, _max(str._len, _len));
}

int String::compare(const String& str) const
{
    return _comp(_string, str._string, _max(str._len, _len));
}

int String::compare(const String& str, size_t len)
{
    return _comp(_string, str._string, len);
}

int String::compare(const String& str, size_t len) const
{
    return _comp(_string, str._string, len);
}

int String::compare(const char* s, size_t n)
{
    return _comp(_string, (char *)s, n);
}

int String::compare(const char* s, size_t n) const
{
    return _comp(_string, (char *)s, n);
}

int String::compare(const char* s)
{
    return _comp(_string, (char *)s, _max(_len, strlen(s)));
}

int String::compare(const char* s) const
{
    return _comp(_string, (char *)s, _max(_len, strlen(s)));
}

int String::compare(const std::string& str)
{
    const char *s = str.c_str();
    return _comp(_string, (char *)s, _max(_len, str.size()));
}

int String::compare(const std::string& str) const
{
    const char *s = str.c_str();
    return _comp(_string, (char *)s, _max(_len, str.size()));
}

int String::caseCompare(const String& str)
{
    return _compN(_string, str._string, _max(_len, str._len));
}

int String::caseCompare(const String& str) const
{
    return _compN(_string, str._string, _max(_len, str._len));
}

int String::caseCompare(const String& str, size_t len)
{
    return _compN(_string, str._string, len);
}

int String::caseCompare(const String& str, size_t len) const
{
    return _compN(_string, str._string, len);
}

int String::caseCompare(const char* s, size_t n)
{
    return _compN(_string, (char *)s, n);
}

int String::caseCompare(const char* s, size_t n) const
{
    return _compN(_string, (char *)s, n);
}

int String::caseCompare(const char* s)
{
    return _compN(_string, (char *)s, _max(_len, strlen(s)));
}

int String::caseCompare(const char* s) const
{
    return _compN(_string, (char *)s, _max(_len, strlen(s)));
}

int String::caseCompare(const std::string& str)
{
    const char *s = str.c_str();
    return _compN(_string, (char *)s, _max(_len, str.size()));
}

int String::caseCompare(const std::string& str) const
{
    const char *s = str.c_str();
    return _compN(_string, (char *)s, _max(_len, str.size()));
}

size_t String::findOf(const String& str, size_t pos)
{
    if (pos >= _len)
        return std::string::npos;

    return _find(_string+pos, str._string, str._len)+pos;
}

size_t String::findOf(const char* s, size_t pos)
{
    if (pos >= _len)
        return std::string::npos;

    size_t i = 0;
    const char *p = s;

    while (*p != 0)
    {
        p++;
        i++;
    }

    return _find(_string+pos, s, i)+pos;
}

size_t strings::String::findOf(char s, size_t pos)
{
    if (pos >= _len)
        return std::string::npos;

    return _find(_string+pos, &s, 1)+pos;
}

size_t String::findOf(const std::string& str, size_t pos)
{
    if (pos >= _len)
        return std::string::npos;

    return _find(_string+pos, str.c_str(), str.size())+pos;
}

size_t String::findOf(const String& str, size_t pos) const
{
    if (pos >= _len)
        return std::string::npos;

    return _find(_string+pos, str._string, str._len)+pos;
}

size_t String::findOf(const char* s, size_t pos) const
{
    if (pos >= _len)
        return std::string::npos;

    size_t i = 0;
    const char *p = s;

    while (*p != 0)
    {
        p++;
        i++;
    }

    return _find(_string+pos, s, i)+pos;
}

size_t strings::String::findOf(char s, size_t pos) const
{
    if (pos >= _len)
        return std::string::npos;

    return _find(_string+pos, &s, 1)+pos;
}

size_t String::findOf(const std::string& str, size_t pos) const
{
    if (pos >= _len)
        return std::string::npos;

    return _find(_string+pos, str.c_str(), str.size())+pos;
}

size_t String::findFirstOf(const String& str)
{
    return _find(_string, str._string, str._len);
}

size_t String::findFirstOf(const char* s)
{
    size_t i = 0;
    const char *p = s;

    while (*p != 0)
    {
        p++;
        i++;
    }

    return _find(_string, s, i);
}

size_t strings::String::findFirstOf(char s)
{
    return _find(_string, &s, 1);
}


size_t String::findFirstOf(const std::string& str)
{
    return _find(_string, str.c_str(), str.size());
}

size_t String::findFirstNotOf(const String& str)
{
    return _findNot(_string, str._string, _len);
}

size_t String::findFirstNotOf(const char* s)
{
    return _findNot(_string, s, _len);
}

size_t strings::String::findFirstNotOf(char s)
{
    char hv[2];
    hv[0] = s;
    hv[1] = 0;
    return _findNot(_string, &hv[0], _len);
}


size_t String::findFirstNotOf(const std::string& str)
{
    return _findNot(_string, str.c_str(), _len);
}

size_t String::findLastOf(const String& str)
{
    return _find_last(_string, str._string, str._len);
}

size_t String::findLastOf(const char* s)
{
    size_t i = 0;
    const char *p = s;

    while (*p != 0)
    {
        p++;
        i++;
    }

    return _find_last(_string, s, i);
}

size_t strings::String::findLastOf(char s)
{
    return _find_last(_string, &s, 1);
}


size_t String::findLastOf(const std::string& str)
{
    return _find_last(_string, str.c_str(), str.size());
}

String String::substring(size_t start, size_t len)
{
    size_t l;

    if (start >= _len)
        return *this;

    if (len == 0)
        l = _len - start;
    else if (len > (_len - start))
        l = _len;
    else
        l = len;

    return String(_string+start, l);
}

String& String::substring(String& str, size_t start, size_t len)
{
    size_t l;

    if (start >= _len)
        return *this;

    if (len == 0)
        l = _len - start;
    else if (len > (_len - start))
        l = _len;
    else
        l = len;

    str.assign(_string+start, l);
    return str;
}

String String::substring(const String& pat, size_t len)
{
    size_t pos = _find(_string, pat._string, pat._len);
    
    if (pos != std::string::npos)
        return substring(pos, len);

    return *this;
}

String String::substring(const char *pat, size_t len)
{
    size_t l = strlen(pat);
    size_t pos = _find(_string, pat, l);
    
    if (pos != std::string::npos)
        return substring(pos, len);

    return *this;
}

String String::substring(const std::string& pat, size_t len)
{
    size_t pos = _find(_string, pat.c_str(), pat.size());
    
    if (pos != std::string::npos)
        return substring(pos, len);

    return *this;
}

String String::substring(size_t start, size_t len) const
{
    size_t l;

    if (start >= _len)
    {
        String *st = const_cast<String *>(this);
        return *st;
    }

    if (len == 0)
        l = _len - start;
    else if (len > (_len - start))
        l = _len;
    else
        l = len;

    return String(_string+start, l);
}

String& String::substring(String& str, size_t start, size_t len) const
{
    size_t l;

    if (start >= _len)
    {
        String *st = const_cast<String *>(this);
        return *st;
    }

    if (len == 0)
        l = _len - start;
    else if (len > (_len - start))
        l = _len;
    else
        l = len;

    str.assign(_string+start, l);
    return str;
}

String String::substring(const String& pat, size_t len) const
{
    size_t pos = _find(_string, pat._string, pat._len);
    
    if (pos != std::string::npos)
        return substring(pos, len);

    String *s = const_cast<String*>(this);
    return *s;
}

String String::substring(const char *pat, size_t len) const
{
    size_t l = strlen(pat);
    size_t pos = _find(_string, pat, l);
    
    if (pos != std::string::npos)
        return substring(pos, len);

    String *s = const_cast<String*>(this);
    return *s;
}

String String::substring(const std::string& pat, size_t len) const
{
    size_t pos = _find(_string, pat.c_str(), pat.size());
    
    if (pos != std::string::npos)
        return substring(pos, len);

    String *s = const_cast<String*>(this);
    return *s;
}

std::vector<String> strings::String::split(char seperator)
{
    char hv0[2];
    hv0[0] = seperator;
    hv0[1] = 0;
    std::vector<String> output = _split(*this, &hv0[0]);
    return output;
}

std::vector<String> strings::String::split(char seperator) const
{
    char hv0[2];
    hv0[0] = seperator;
    hv0[1] = 0;
    std::vector<String> output = _split(*this, &hv0[0]);
    return output;
}

std::vector<String> strings::String::split(int seperator)
{
    char hv0[2];
    hv0[0] = seperator;
    hv0[1] = 0;
    std::vector<String> output = _split(*this, &hv0[0]);
    return output;
}

std::vector<String> strings::String::split(int seperator) const
{
    char hv0[2];
    hv0[0] = seperator;
    hv0[1] = 0;
    std::vector<String> output = _split(*this, &hv0[0]);
    return output;
}

std::vector<String> strings::String::split(const char* seps)
{
    std::vector<String> output = _split(*this, seps);
    return output;
}

std::vector<String> strings::String::split(const char* seps) const
{
    std::vector<String> output = _split(*this, seps);
    return output;
}

std::vector<String> strings::String::split(const std::string& seps)
{
    std::vector<String> output = _split(*this, seps.c_str());
    return output;
}

std::vector<String> strings::String::split(const std::string& seps) const
{
    std::vector<String> output = _split(*this, seps.c_str());
    return output;
}

std::vector<String> strings::String::split(const strings::String& seps)
{
    const char *d = seps.data();
    std::vector<String> output = _split(*this, d);
    return output;
}

std::vector<String> strings::String::split(const strings::String& seps) const
{
    const char *d = seps.data();
    std::vector<String> output = _split(*this, d);
    return output;
}

std::vector<String> String::split(const String& s, char seperator)
{
    char hv0[2];
    hv0[0] = seperator;
    hv0[1] = 0;
    std::vector<String> output = _split(s, &hv0[0]);
    return output;
}

std::vector<String> String::split(const String& s, char seperator) const
{
    char hv0[2];
    hv0[0] = seperator;
    hv0[1] = 0;
    std::vector<String> output = _split(s, &hv0[0]);
    return output;
}

std::vector<String> strings::String::split(const strings::String& s, const strings::String& seps)
{
    std::vector<String> output = _split(s, seps._string);
    return output;
}

std::vector<String> strings::String::split(const strings::String& s, const strings::String& seps) const
{
    std::vector<String> output = _split(s, seps._string);
    return output;
}

std::vector<String> strings::String::split(const char* s, char seperator)
{
    char hv0[2];
    hv0[0] = seperator;
    hv0[1] = 0;
    String str(s);
    std::vector<String> output = _split(str, &hv0[0]);
    return output;
}

std::vector<String> strings::String::split(const char* s, char seperator) const
{
    char hv0[2];
    hv0[0] = seperator;
    hv0[1] = 0;
    String str(s);
    std::vector<String> output = _split(str, &hv0[0]);
    return output;
}

std::vector<String> strings::String::split(const char* s, const char* seps)
{
    String str(s);
    std::vector<String> output = _split(str, seps);
    return output;
}

std::vector<String> strings::String::split(const char* s, const char* seps) const
{
    String str(s);
    std::vector<String> output = _split(str, seps);
    return output;
}

std::vector<String> strings::String::split(const std::string& s, char seperator)
{
    char hv0[2];
    hv0[0] = seperator;
    hv0[1] = 0;
    String str(s);
    std::vector<String> output = _split(str, &hv0[0]);
    return output;
}

std::vector<String> strings::String::split(const std::string& s, char seperator) const
{
    char hv0[2];
    hv0[0] = seperator;
    hv0[1] = 0;
    String str(s);
    std::vector<String> output = _split(str, &hv0[0]);
    return output;
}

std::vector<String> String::split(const std::string& s, const std::string& seps)
{
    String str(s);
    std::vector<String> output = _split(str, seps.c_str());
    return output;
}

std::vector<String> String::split(const std::string& s, const std::string& seps) const
{
    String str(s);
    std::vector<String> output = _split(str, seps.c_str());
    return output;
}

String& String::replace(const String& str, const String& what, REPLACE rep)
{
    _replace(str._string, str._len, what._string, what._len, rep);
    return *this;
}

String& String::replace(const char* str, const char* what, strings::REPLACE rep)
{
    size_t len1 = 0, len2 = 0;
    
    while (*(str+len1) != 0)
        len1++;
    
    while (*(what+len2) != 0)
        len2++;
    
    _replace((char *)str, len1, (char *)what, len2, rep);
    return *this;
}

String& String::replace(const std::string& str, const std::string& what, strings::REPLACE rep)
{
    _replace((char *)str.c_str(), str.length(), (char *)what.c_str(), what.length(), rep);
    return *this;
}

String& String::ltrim()
{
    char *p = _string;
    size_t pos = 0;

    if (_string == 0 || _allocated == 0 || _len == 0)
        return *this;

    while ((*p == ' ' || *p == '\t') && pos < _len)
    {
        p++;
        pos++;
    }

    if (pos == 0)
        return *this;

    size_t nl = _len - pos;
    size_t al = 0;
    char *buf = 0;
    
    try
    {
        buf = getMemory(nl, al, buf);
    }
    catch (SCATCH(e))
    {
        throw;
    }

    size_t i = 0;
    
    while (pos < _len)
    {
        *(buf+i) = *(_string+pos);
        i++;
        pos++;
    }
    
    *(buf+i) = 0;
    delete[] _string;
    _string = buf;
    _len = nl;
    _allocated = al;
    return *this;
}

String& String::rtrim()
{
    char *p = _string+_len-1;
    size_t pos = _len-1;

    if (_string == 0 || _allocated == 0 || _len == 0)
        return *this;

    while ((*p == ' ' || *p == '\t') && pos > 0)
    {
        p--;
        pos--;
    }

    if (pos == (_len-1))
        return *this;

    size_t nl = pos + 1;
    *(_string+nl) = 0;
    _len = nl;
    return *this;
}

String& String::ltrim() const
{
    char *p = _string;
    size_t pos = 0;
    String *ori = const_cast<String*>(this);

    if (_string == 0 || _allocated == 0 || _len == 0)
        return *ori;

    while ((*p == ' ' || *p == '\t') && pos < _len)
    {
        p++;
        pos++;
    }

    if (pos == 0)
        return *ori;

    size_t nl = _len - pos;
    size_t al = 0;
    char *buf = 0;
    
    try
    {
        buf = getMemory(nl, al, buf);
    }
    catch (SCATCH(e))
    {
        throw;
    }

    size_t i = 0;
    
    while (pos < _len)
    {
        *(buf+i) = *(_string+pos);
        i++;
        pos++;
    }
    
    *(buf+i) = 0;
    delete[] _string;
    ori->_string = buf;
    ori->_len = nl;
    ori->_allocated = al;
    return *ori;
}

String& String::rtrim() const
{
    char *p = _string+_len-1;
    size_t pos = _len-1;
    String *ori = const_cast<String*>(this);

    if (_string == 0 || _allocated == 0 || _len == 0)
        return *ori;

    while ((*p == ' ' || *p == '\t') && pos > 0)
    {
        p--;
        pos--;
    }

    if (pos == (_len-1))
        return *ori;

    size_t nl = pos + 1;
    *(_string+nl) = 0;
    ori->_len = nl;
    return *ori;
}

void String::swap(String& str)
{
    char *o = _string;
    size_t ol = _len;
    size_t oa = _allocated;
    _string = str._string;
    _len = str._len;
    _allocated = str._allocated;
    str._string = o;
    str._len = ol;
    str._allocated = oa;
}

/************** Internal used functions ********************/

void String::assertElement(size_t i) const
{
    if (i == std::string::npos)
        SThrow(ERR_INVALID_INDEX);
    else if (i >= _len)
        SThrow(ERR_OUT_OF_BOUNDS);
}

void String::assertMemory(size_t i) const
{
    if (i == std::string::npos)
        SThrow(ERR_INVALID_INDEX);
    else if (i > MAX_BLOCK)
        SThrow(ERR_OUT_OF_MEMORY);
}

void String::copyString(const char* s, size_t len)
{
    try
    {
        getMemory(len);
    }
    catch(SCATCH(e))
    {
        throw;
    }

    *_string = 0;
    size_t pos = 0;

    for (size_t i = 0; i < len; i++)
    {
        if (*(s+i) != 0)
        {
            *(_string+i) = *(s+i);
            pos++;
        }
        else
        {
            *(_string+i) = 0;
            break;
        }
    }

    *(_string+pos) = 0;
    _len = pos;
}

void String::appendString(const char* s, size_t len)
{
    try
    {
        getMemory(len);
    }
    catch (SCATCH(e))
    {
        throw;
    }

    // Now append the new string
    int x = 0;

    for (size_t i = _len; i < (_len+len); i++)
    {
        *(_string+i) = *(s+x);
        x++;
    }
    // mark the end
    _len += len;
    *(_string+_len) = 0;
}

void String::appendString(const char* s, size_t len) const
{
    try
    {
        getMemory(len);
    }
    catch (SCATCH(e))
    {
        throw;
    }

    // Now append the new string
    int x = 0;

    for (size_t i = _len; i < (_len+len); i++)
    {
        *(_string+i) = *(s+x);
        x++;
    }
    // mark the end
    String *o = const_cast<String*>(this);
    o->_len += len;
    *(_string+_len) = 0;
}

String String::appendVirtual(const String& s)
{
    String neu(_string);
    neu.append(s);
    return neu;
}

String String::appendVirtual(const std::string& s)
{
    String neu(_string);
    neu.append(s);
    return neu;
}

String String::appendVirtual(const char *s)
{
    String neu(_string);
    neu.append(s);
    return neu;
}

String String::appendVirtual(const int i)
{
    String neu(_string);
    neu.append(i);
    return neu;
}

String String::appendVirtual(const LONG l)
{
    String neu(_string);
    neu.append(l);
    return neu;
}

String String::appendVirtual(const char c)
{
    String neu(_string);
    neu.append(c);
    return neu;
}

String String::appendVirtual(const unsigned int i)
{
    String neu(_string);
    neu.append(i);
    return neu;
}

String String::appendVirtual(const ULONG l)
{
    String neu(_string);
    neu.append(l);
    return neu;
}

String String::appendVirtual(const float f)
{
    String neu(_string);
    neu.append(f);
    return neu;
}

String String::appendVirtual(const double d)
{
    String neu(_string);
    neu.append(d);
    return neu;
}

String String::appendVirtual(const String& s) const
{
    String neu(_string);
    neu.append(s);
    return neu;
}

String String::appendVirtual(const std::string& s) const
{
    String neu(_string);
    neu.append(s);
    return neu;
}

String String::appendVirtual(const char *s) const
{
    String neu(_string);
    neu.append(s);
    return neu;
}

String String::appendVirtual(const int i) const
{
    String neu(_string);
    neu.append(i);
    return neu;
}

String String::appendVirtual(const LONG l) const
{
    String neu(_string);
    neu.append(l);
    return neu;
}

String String::appendVirtual(const char c) const
{
    String neu(_string);
    neu.append(c);
    return neu;
}

String String::appendVirtual(const unsigned int i) const
{
    String neu(_string);
    neu.append(i);
    return neu;
}

String String::appendVirtual(const ULONG l) const
{
    String neu(_string);
    neu.append(l);
    return neu;
}

String String::appendVirtual(const float f) const
{
    String neu(_string);
    neu.append(f);
    return neu;
}

String String::appendVirtual(const double d) const
{
    String neu(_string);
    neu.append(d);
    return neu;
}

char *String::getMemory(size_t l)
{
    if (l >= _allocated)
    {
        size_t fact = l / MEM_BLOCK + 1;    // The multiplication factor to get the amount of memory needed
        size_t nl = fact * MEM_BLOCK;       // The amount of memory needed
        assertMemory(nl);                   // Throw an exception if memory is exhausted
        char *np = new char[nl];            // Allocate the memory
        // Copy the content
        if (_string != 0 && _len > 0)
        {
            for (size_t i = 0; i < _len; i++)
                *(np+i) = *(_string+i);

            delete[] _string;
        }

        _string = np;
        _allocated = nl;
    }

    return _string;
}

char *String::getMemory(size_t l) const
{
    if (l >= _allocated)
    {
        size_t fact = l / MEM_BLOCK + 1;    // The multiplication factor to get the amount of memory needed
        size_t nl = fact * MEM_BLOCK;       // The amount of memory needed
        assertMemory(nl);                   // Throw an exception if memory is exhausted
        char *np = new char[nl];            // Allocate the memory
        // Copy the content
        if (_string != 0 && _len > 0)
        {
            for (size_t i = 0; i < _len; i++)
                *(np+i) = *(_string+i);

            delete[] _string;
        }

        String *o = const_cast<String*>(this);
        o->_string = np;
        o->_allocated = nl;
    }

    return _string;
}

/*
 * @param l
 * Amount of memory to allocate
 * 
 * @param a
 * Amount of allready allocated memory
 * 
 * @param p
 * Pointer to the allocated memory
 */
char *String::getMemory(size_t l, size_t& a, char *p)
{
    size_t nl = 0;

    if (l >= a)
    {
        size_t fact = l / MEM_BLOCK + 1;    // The multiplication factor to get the amount of memory needed
        nl = fact * MEM_BLOCK;              // The amount of memory needed
        assertMemory(nl);                   // Throw an exception if memory is exhausted
        char *np = new char[nl];            // Allocate the memory
        // Copy the content
        if (p != 0 && a > 0)
        {
            for (size_t i = 0; i < a; i++)
                *(np+i) = *(p+i);

            *(np+a) = 0;
            delete[] p;
        }

        p = np;
        a = nl;
    }

    return p;
}

char *String::getMemory(size_t l, size_t& a, char *p) const
{
    size_t nl = 0;

    if (l >= a)
    {
        size_t fact = l / MEM_BLOCK + 1;    // The multiplication factor to get the amount of memory needed
        nl = fact * MEM_BLOCK;              // The amount of memory needed
        assertMemory(nl);                   // Throw an exception if memory is exhausted
        char *np = new char[nl];            // Allocate the memory
        // Copy the content
        if (p != 0 && a > 0)
        {
            for (size_t i = 0; i < a; i++)
                *(np+i) = *(p+i);

            *(np+a) = 0;
            delete[] p;
        }

        p = np;
        a = nl;
    }

    return p;
}

int String::_comp(char* s1, char* s2, size_t l)
{
    for (size_t i = 0; i < l; i++)
    {
        if (*(s1+i) != *(s2+i))
            return (int)(*(s1+i) - *(s2+i));
    }

    return 0;
}

int String::_comp(char* s1, char* s2, size_t l) const
{
    String *my = const_cast<String *>(this);
    return my->_comp(s1, s2, l);
}

int String::_compN(char* s1, char* s2, size_t l)
{
    for (size_t i = 0; i < l; i++)
    {
        if (toupper(*(s1+i)) != toupper(*(s2+i)))
            return (int)(toupper(*(s1+i)) - toupper(*(s2+i)));
    }

    return 0;
}

int String::_compN(char* s1, char* s2, size_t l) const
{
    String *my = const_cast<String *>(this);
    return my->_compN(s1, s2, l);

    return 0;
}

size_t String::_find(const char *s, const char *what, size_t len)
{
    if (len > _len)
        return std::string::npos;

    size_t shift = _len - len;
    
    for (size_t i = 0; i <= shift; i++)
    {
        if (_comp((char *)s+i, (char *)what, len) == 0)
            return i;
    }

    return std::string::npos;
}

size_t String::_find(const char *s, const char *what, size_t len) const
{
    String *my = const_cast<String *>(this);
    return my->_find(s, what, len);
}

size_t String::_findNot(const char *s, const char *what, size_t len)
{
    char *str = (char *)s;
    char *sep = (char *)what;
    size_t pos = 0;

    while (*str != 0 && pos < len)
    {
        while (*sep != 0)
        {
            if (*s != *sep)
                return pos;

            sep++;
        }

        str++;
        sep = (char *)what;
        pos++;
    }

    return std::string::npos;
}

size_t String::_find_last(const char *s, const char *what, size_t len)
{
    if (len > _len)
        return std::string::npos;

    size_t shift = _len - len;
    
    for (long i = shift; i >= 0; i--)
    {
        if (_comp((char *)s+i, (char *)what, len) == 0)
            return i;
    }

    return std::string::npos;
}

std::vector<String> String::_split(const String& str, const char *seps)
{
    std::vector<String> output;
    size_t prev_pos = 0, pos = 0;

    while((pos = _isPart(_string, seps, pos)) != std::string::npos)
    {
            String substring(str.substring(prev_pos, pos - prev_pos));
            output.push_back(substring);
            prev_pos = ++pos;
    }

    output.push_back(str.substring(prev_pos, _len - prev_pos));
    return output;
}

std::vector<String> String::_split(const String& str, const char *seps) const
{
    String *my = const_cast<String *>(this);
    return my->_split(str, seps);
}

char *String::_replace(char *pattern, size_t patlen, char *replacement, size_t replen, REPLACE rep)
{
    size_t patcnt = 0;
    const char *oriptr;
    const char *patloc;

    if (_string == 0 || _len == 0)
        return _string;

    if (strstr(_string, pattern) == 0)
        return _string;

    // find how many times the pattern occurs in the original string
    for (oriptr = _string; (patloc = strstr(oriptr, pattern)) != 0; oriptr = patloc + patlen)
        patcnt++;

    // allocate memory for the new string
    size_t retlen = _len + patcnt * (replen - patlen);
    size_t na = 0;
    char *returned = 0;
    
    try
    {
        returned = getMemory(retlen, na, returned);
    }
    catch (SCATCH(e))
    {
        throw;
    }

    if (returned != 0)
    {
        // copy the original string, 
        // replacing all the instances of the pattern
        char *retptr = returned;

        if (rep == ALL)
        {
            for (oriptr = _string; (patloc = strstr(oriptr, pattern)) != 0; oriptr = patloc + patlen)
            {
                size_t const skplen = patloc - oriptr;
                // copy the section until the occurence of the pattern
                strncpy(retptr, oriptr, skplen);
                retptr += skplen;
                // copy the replacement 
                strncpy(retptr, replacement, replen);
                retptr += replen;
            }
            // copy the rest of the string.
            strcpy(retptr, oriptr);
        }
        else if (rep == FIRST)
        {
            oriptr = _string;
            patloc = strstr(oriptr, pattern);
            size_t const skplen = patloc - oriptr;
            // copy the section until the occurence of the pattern
            strncpy(retptr, oriptr, skplen);
            retptr += skplen;
            // copy the replacement 
            strncpy(retptr, replacement, replen);
            retptr += replen;
            oriptr = patloc + patlen;
            strcpy(retptr, oriptr);
            retlen = _len - patlen + replen;
        }
        else
        {
            oriptr = _string;
            // Find last occurence
            while ((patloc = strstr(oriptr, pattern)) != 0)
                oriptr = patloc + patlen;

            size_t const skplen = oriptr - _string - patlen;
            // copy the section until the occurence of the pattern
            strncpy(retptr, _string, skplen);
            retptr += skplen;
            // copy the replacement 
            strncpy(retptr, replacement, replen);
            retptr += replen;
            strcpy(retptr, oriptr);
            retlen = _len - patlen + replen;
        }
    }
    else
        return _string;

    delete[] _string;
    _string = returned;
    _len = retlen;
    _allocated = na;
    return _string;
}

size_t String::_min(size_t a1, size_t a2)
{
    if (a1 > a2)
        return a2;

    return a1;
}

size_t String::_max(size_t a1, size_t a2)
{
    if (a1 > a2)
        return a1;

    return a2;
}

size_t String::_min(size_t a1, size_t a2) const
{
    if (a1 > a2)
        return a2;

    return a1;
}

size_t String::_max(size_t a1, size_t a2) const
{
    if (a1 > a2)
        return a1;

    return a2;
}

size_t String::_isPart(const char* str, const char* p, size_t position)
{
    char *s = (char *)str + position;
    char *sep = (char *)p;
    size_t pos = 0;

    while (*s != 0)
    {
        while (*sep != 0)
        {
            if (*s == *sep)
                return pos + position;

            sep++;
        }

        s++;
        sep = (char *)p;
        pos++;
    }

    return std::string::npos;
}

size_t String::_isPart(const char* str, const char* p, size_t position) const
{
    char *s = (char *)str + position;
    char *sep = (char *)p;
    size_t pos = 0;

    while (*s != 0)
    {
        while (*sep != 0)
        {
            if (*s == *sep)
                return pos + position;

            sep++;
        }

        s++;
        sep = (char *)p;
        pos++;
    }

    return std::string::npos;
}

bool strings::String::_insert(size_t pos, const char* s, size_t len)
{
    if (pos > _len || _string == 0)
        return false;

    size_t al = 0, ptr = 0;
    char *buf = 0;
    
    try
    {
        buf = getMemory(_len+len, al, buf);
    }
    catch (SCATCH(e))
    {
        throw;
    }

    if (buf == 0)
        return false;

    for (size_t i = 0; i < pos; i++)
        *(buf+i) = *(_string+i);

    ptr = pos;

    for (size_t i = 0; i < len; i++)
    {
        *(buf+ptr) = *(s+i);
        ptr++;
    }
    
    for (size_t i = pos; i < _len; i++)
    {
        *(buf+ptr) = *(_string+i);
        ptr++;
    }
    
    *(buf+ptr) = 0;
    delete[]_string;
    _string = buf;
    _len = ptr;
    _allocated = al;
    return true;
}

std::istream & strings::String::_rdToDelim(std::istream& stream, char delim)
{
    clear();
    std::string buf;
    std::getline(stream, buf, delim);
    
    try
    {
        copyString(buf.c_str(), buf.size());
    }
    catch(SCATCH(e))
    {
        throw;
    }

    return stream;
}

/*********************************************************/

void strings::SThrow(const String& err)
{
    STHROW(err.data());
}

void strings::SThrow(const std::string& err)
{
    STHROW(err.c_str());
}

void strings::SThrow(const char *err)
{
    STHROW(err);
}
