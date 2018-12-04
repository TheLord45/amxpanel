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

#include "regexp.h"

using namespace strings;

Regexp::Regexp(const String& re, RegCExtend ext)
{
    if (re.length() == 0)
        return;

    mRegstr = re;
    valid = false;
    exec = false;

    try
    {
        if (_compile(re.data(), ext) == ERR_NOERROR)
            valid = true;
    }
    catch (RegException&)
    {
        throw;
    }
}

Regexp::Regexp(const std::string& re, RegCExtend ext)
{
    if (re.length() == 0)
        return;
    
    mRegstr = re;
    valid = false;
    exec = false;

    try
    {
        if (_compile(re.c_str(), ext) == ERR_NOERROR)
            valid = true;
    }
    catch (RegException&)
    {
        throw;
    }
}

Regexp::Regexp(const char* re, RegCExtend ext)
{
    if (*re == 0)
        return;
    
    mRegstr = re;
    valid = false;
    exec = false;

    try
    {
        if (_compile(re, ext) == ERR_NOERROR)
            valid = true;
    }
    catch (RegException&)
    {
        throw;
    }
}

void Regexp::setRegexp(const String& re)
{
    mRegstr = re;
    valid = false;
    exec = false;
    mResult.clear();
}

void Regexp::setRegexp(const std::string& re)
{
    mRegstr = re;
    valid = false;
    exec = false;
    mResult.clear();
}

void Regexp::setRegexp(const char* re)
{
    mRegstr = re;
    valid = false;
    exec = false;
    mResult.clear();
}

bool Regexp::compile(RegCExtend ext)
{
    if (!valid || mRegstr.length() == 0)
        return false;

    try
    {
        if (_compile(mRegstr.data(), ext) == ERR_NOERROR)
        {
            valid = true;
            return true;
        }

        return false;
    }
    catch (RegException&)
    {
        throw;
    }
}

bool Regexp::compile(const String& re, RegCExtend ext)
{
    if (re.length() == 0)
        return false;

    mRegstr = re;

    try
    {
        if (_compile(mRegstr.data(), ext) == ERR_NOERROR)
        {
            valid = true;
            return true;
        }

        return false;
    }
    catch (RegException&)
    {
        throw;
    }
}

bool Regexp::compile(const std::string& re, RegCExtend ext)
{
    if (re.length() == 0)
        return false;

    mRegstr = re;

    try
    {
        if (_compile(mRegstr.data(), ext) == ERR_NOERROR)
        {
            valid = true;
            return true;
        }

        return false;
    }
    catch (RegException&)
    {
        throw;
    }
}

bool Regexp::compile(const char* re, RegCExtend ext)
{
    if (*re == 0)
        return false;

    mRegstr = re;

    try
    {
        if (_compile(mRegstr.data(), ext) == ERR_NOERROR)
        {
            valid = true;
            return true;
        }

        return false;
    }
    catch (RegException&)
    {
        throw;
    }
}

bool Regexp::execute(const String& str, RegEExtend ext)
{
    if (!valid || mRegstr.length() == 0)
        return false;

    regmatch_t matches[MAX_REGERG];
    RegError ret = (RegError)regexec(&regex, str.data(), MAX_REGERG, (regmatch_t*)&matches, ext);
    
    if (!ret)
    {
        mResult = _assignMatch(str, (regmatch_t*)&matches);
        exec = true;
        return true;
    }
    else if (ret == ERR_NOMATCH)
    {
        mResult.clear();
        exec = true;
        return false;
    }
    else
    {
        exec = false;
        mResult.clear();
        throw RegException(ret, regex);
    }
}

bool Regexp::execute(const std::string& str, RegEExtend ext)
{
    if (!valid || mRegstr.length() == 0)
        return false;
    
    regmatch_t matches[MAX_REGERG];
    RegError ret = (RegError)regexec(&regex, str.c_str(), MAX_REGERG, (regmatch_t*)&matches, ext);
    
    if (!ret)
    {
        String s(str);
        mResult = _assignMatch(s, (regmatch_t*)&matches);
        exec = true;
        return true;
    }
    else if (ret == ERR_NOMATCH)
    {
        exec = true;
        mResult.clear();
        return false;
    }
    else
    {
        exec = false;
        mResult.clear();
        throw RegException(ret, regex);
    }
}

bool Regexp::execute(const char* str, RegEExtend ext)
{
    if (!valid || mRegstr.length() == 0)
        return false;
    
    regmatch_t matches[MAX_REGERG];
    RegError ret = (RegError)regexec(&regex, str, MAX_REGERG, (regmatch_t*)&matches, ext);
    
    if (!ret)
    {
        String s(str);
        mResult = _assignMatch(s, (regmatch_t*)&matches);
        exec = true;
        return true;
    }
    else if (ret == ERR_NOMATCH)
    {
        exec = true;
        mResult.clear();
        return false;
    }
    else
        throw RegException(ret, regex);
}

bool Regexp::execute(const String& re, const String& str, RegEExtend ext)
{
    if (re.length() == 0 || str.length() == 0)
        return false;
    
    try
    {
        if (_compile(str.data(), RC_NONE) != ERR_NOERROR)
            return false;

        RegError ret = (RegError)regexec(&regex, str.data(), 0, 0, ext);

        if (!ret)
            return true;
        else if (ret == ERR_NOMATCH)
            return false;
        else
            throw RegException(ret, regex);
    }
    catch (RegException&)
    {
        throw;
    }
}

bool Regexp::execute(const std::string& re, const std::string& str, RegEExtend ext)
{
    if (re.length() == 0 || str.length() == 0)
        return false;
    
    try
    {
        if (_compile(str.data(), RC_NONE) != ERR_NOERROR)
            return false;

        RegError ret = (RegError)regexec(&regex, str.c_str(), 0, 0, ext);

        if (!ret)
            return true;
        else if (ret == ERR_NOMATCH)
            return false;
        else
            throw RegException(ret, regex);
    }
    catch (RegException&)
    {
        throw;
    }
}

bool Regexp::execute(const char* re, const char* str, RegEExtend ext)
{
    if (*re == 0 || *str == 0)
        return false;
    
    try
    {
        if (_compile(str, RC_NONE) != ERR_NOERROR)
            return false;

        RegError ret = (RegError)regexec(&regex, str, 0, 0, ext);

        if (!ret)
            return true;
        else if (ret == ERR_NOMATCH)
            return false;
        else
            throw RegException(ret, regex);
    }
    catch (RegException&)
    {
        throw;
    }
}

String Regexp::match(const String& str, RegCExtend cext, RegEExtend eext)
{
    if (!valid)
        return "";
    
    try
    {
        if (_compile(mRegstr.data(), cext) != ERR_NOERROR)
            return "";
        
        regmatch_t matches[MAX_REGERG];
        RegError ret = (RegError)regexec(&regex, str.data(), MAX_REGERG, (regmatch_t*)&matches, eext);
        
        if (!ret)
        {
            mResult = _assignMatch(str, (regmatch_t*)&matches);
            exec = true;
            return mResult;
        }
        else if (ret == ERR_NOMATCH)
        {
            exec = true;
            mResult.clear();
            return "";
        }
        else
        {
            exec = false;
            mResult.clear();
            throw RegException(ret, regex);
        }
    }
    catch (RegException&)
    {
        throw;
    }
}

std::string Regexp::match(const std::string& str, RegCExtend cext, RegEExtend eext)
{
    if (!valid)
        return "";
    
    try
    {
        if (_compile(mRegstr.data(), cext) != ERR_NOERROR)
            return "";
        
        regmatch_t matches[MAX_REGERG];
        RegError ret = (RegError)regexec(&regex, str.c_str(), MAX_REGERG, (regmatch_t*)&matches, eext);
        
        if (!ret)
        {
            String s(str);
            mResult = _assignMatch(s, (regmatch_t*)&matches);
            exec = true;
            return mResult.toString();
        }
        else if (ret == ERR_NOMATCH)
        {
            exec = true;
            mResult.clear();
            return "";
        }
        else
        {
            exec = false;
            mResult.clear();
            throw RegException(ret, regex);
        }
    }
    catch (RegException&)
    {
        throw;
    }
}

String Regexp::match(const String& re, const String& str, RegCExtend cext, RegEExtend eext)
{
    if (re.length() == 0)
        return "";
    
    try
    {
        if (_compile(re.data(), cext) != ERR_NOERROR)
            return "";
        
        regmatch_t matches[MAX_REGERG];
        RegError ret = (RegError)regexec(&regex, str.data(), MAX_REGERG, (regmatch_t*)&matches, eext);
        
        if (!ret)
            return _assignMatch(str, (regmatch_t*)&matches);
        else if (ret == ERR_NOMATCH)
            return "";
        else
            throw RegException(ret, regex);
    }
    catch (RegException&)
    {
        throw;
    }
}

std::string Regexp::match(const std::string& re, const std::string& str, RegCExtend cext, RegEExtend eext)
{
    if (re.length() == 0)
        return "";
    
    try
    {
        if (_compile(re.c_str(), cext) != ERR_NOERROR)
            return "";
        
        regmatch_t matches[MAX_REGERG];
        RegError ret = (RegError)regexec(&regex, str.c_str(), MAX_REGERG, (regmatch_t*)&matches, eext);
        
        if (!ret)
        {
            String s(str);
            s = _assignMatch(s, (regmatch_t*)&matches);
            return s.toString();
        }
        else if (ret == ERR_NOMATCH)
            return "";
        else
            throw RegException(ret, regex);
    }
    catch (RegException&)
    {
        throw;
    }
}

Regexp& Regexp::getResult(const String& str, RegCExtend cext, RegEExtend eext)
{
    if (!valid)
        return *this;
    
    try
    {
        if (_compile(mRegstr.data(), cext) != ERR_NOERROR)
            return *this;
        
        regmatch_t matches[MAX_REGERG];
        RegError ret = (RegError)regexec(&regex, str.data(), MAX_REGERG, (regmatch_t*)&matches, eext);
        
        if (!ret)
        {
            mResult = _assignMatch(str, (regmatch_t*)&matches);
            exec = true;
            return *this;
        }
        else if (ret == ERR_NOMATCH)
        {
            exec = true;
            mResult.clear();
            return *this;
        }
        else
            throw RegException(ret, regex);
    }
    catch (RegException&)
    {
        throw;
    }
}

Regexp& Regexp::getResult(const String& re, const String& str, RegCExtend cext, RegEExtend eext)
{
    try
    {
        if (_compile(re.data(), cext) != ERR_NOERROR)
            return *this;
        
        regmatch_t matches[MAX_REGERG];
        RegError ret = (RegError)regexec(&regex, str.data(), MAX_REGERG, (regmatch_t*)&matches, eext);
        
        if (!ret)
        {
            mResult = _assignMatch(str, (regmatch_t*)&matches);
            exec = true;
            return *this;
        }
        else if (ret == ERR_NOMATCH)
        {
            exec = true;
            mResult.clear();
            return *this;
        }
        else
            throw RegException(ret, regex);
    }
    catch (RegException&)
    {
        throw;
    }
}

String& Regexp::getResult()
{
    return mResult;
}

String& Regexp::getResult() const
{
    Regexp *r = const_cast<Regexp*>(this);
    return r->mResult;
}

Regexp::~Regexp()
{
    if (valid)
        regfree(&regex);
}

RegError Regexp::_compile(const char* re, RegCExtend ext)
{
    RegError err;

    if (!(err = (RegError)regcomp(&regex, re, (int)ext)))
        return ERR_NOERROR;
    else
        throw RegException(err, regex);
}

String Regexp::_assignMatch(const String& str, regmatch_t match[])
{
    int i = 1;
    String erg;
    
    while (match[i].rm_so != -1 && i < MAX_REGERG)
    {
        erg = str.substring((size_t)match[i].rm_so, (size_t)(match[i].rm_eo - match[i].rm_so));
        i++;
    }

    return erg;
}
