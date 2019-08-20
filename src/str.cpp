/*
 * Copyright (C) 2019 by Andreas Theofilu <andreas@theosys.at>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

#include "str.h"

using namespace std;

string& Str::replace(const string& old, const string& neu)
{
	size_t pos = mStr.find(old);

	while (pos != string::npos)
	{
		mStr.replace(pos, old.size(), neu);
		pos = mStr.find(old, pos + neu.size());
	}

	return mStr;
}

const string & Str::replace(const string& old, const string& neu) const
{
	Str *my = const_cast<Str *>(this);
	return my->replace(old, neu);
}

string Str::replace(string& str, const string& old, const string& neu)
{
	size_t pos = str.find(old);

	while (pos != string::npos)
	{
		str.replace(pos, old.size(), neu);
		pos = str.find(old, pos + neu.size());
	}

	return str;
}

vector<string> Str::split(char sep)
{
	string deli;
	deli.push_back(sep);
	return _split(mStr, deli, true);
}

vector<string> Str::split(const string& seps)
{
	return _split(mStr, seps, true);
}

const vector<string> Str::split(char sep) const
{
	Str *my = const_cast<Str *>(this);

	string deli;
	deli.push_back(sep);
	return my->_split(mStr, deli, true);
}

const vector<string> Str::split(const string& seps) const
{
	Str *my = const_cast<Str *>(this);
	return my->_split(mStr, seps, true);
}

vector<string> Str::split(const string& str, char sep)
{
	Str s;
	string deli;
	deli.push_back(sep);
	return s._split(str, deli, true);
}

vector<string> Str::split(const string& str, const string& seps)
{
	Str s;
	return s._split(str, seps, true);
}

int Str::caseCompare(const string& str)
{
	size_t i = 0;

	if (str.length() != mStr.length())
		return ((mStr.length() < str.length()) ? -1 : 1);

	for (const_iterator it = mStr.begin(); it != mStr.end(); ++it)
	{
		if (tolower(*it) != tolower(str.at(i)))
			return (int)(*it - str.at(i));

		i++;
	}

	return 0;
}

int Str::caseCompare(const string& str) const
{
	Str *my = const_cast<Str *>(this);
	return my->caseCompare(str);
}

int Str::caseCompare(const string& str1, const string& str2)
{
	size_t i = 0;

	for (const_iterator it = str1.begin(); it != str1.end(); ++it)
	{
		if (i >= str2.size())
			return -1;

		if (tolower(*it) != tolower(str2.at(i)))
			return (int)(*it - str2.at(i));

		i++;
	}

	return 0;
}

bool Str::isNumeric()
{
	for (iterator it = mStr.begin(); it != mStr.end(); ++it)
	{
		if (!isdigit(*it))
			return false;
	}

	return true;
}

bool Str::isNumeric() const
{
	Str *my = const_cast<Str *>(this);
	return my->isNumeric();
}

bool Str::isNumeric(const string &str)
{
	for (const_iterator it = str.begin(); it != str.end(); ++it)
	{
		if (!isdigit(*it))
			return false;
	}

	return true;
}

vector<string> Str::_split(const string& str, const string& seps, const bool trimEmpty)
{
	size_t pos = 0, mark = 0;
	vector<string> parts;

	for (const_iterator it = str.begin(); it != str.end(); ++it)
	{
		for (const_iterator sepIt = seps.begin(); sepIt != seps.end(); ++sepIt)
		{
			if (*it == *sepIt)
			{
				size_t len = pos - mark;
				parts.push_back(str.substr(mark, len));
				mark = pos + 1;
				break;
			}
		}

		pos++;
	}

	parts.push_back(str.substr(mark));

	if (trimEmpty)
	{
		vector<string> nparts;

		for (auto it = parts.begin(); it != parts.end(); ++it)
		{
			if (it->empty())
				continue;

			nparts.push_back(*it);
		}

		return nparts;
	}

	return parts;
}
