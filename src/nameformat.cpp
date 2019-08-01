/*
 * Copyright (C) 2018 by Andreas Theofilu <andreas@theosys.at>
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

#include <string>
#include <string.h>
#include <ios>
#include <iomanip>
#include <iconv.h>
#include "syslog.h"
#include "nameformat.h"
#include "trace.h"
#include "str.h"

extern Syslog *sysl;

using namespace std;

NameFormat::NameFormat()
{
	sysl->TRACE(Syslog::ENTRY, string("NameFormat::NameFormat()"));
}

NameFormat::~NameFormat()
{
	sysl->TRACE(Syslog::EXIT, string("NameFormat::NameFormat()"));
}

string NameFormat::toValidName(string& str)
{
	DECL_TRACER("NameFormat::toValidName(string& str)");
	string ret;

	for (size_t i = 0; i < str.length(); i++)
	{
		char c = str.at(i);

		if (!(c >= '0' && c <= '9') && !(c >= 'a' && c <= 'z') && !(c >= 'A' && c <= 'Z'))
			c = '_';

		ret.push_back(c);
	}

	return ret;
}

string NameFormat::cutNumbers(string& str)
{
	DECL_TRACER("NameFormat::cutNumbers(String& str)");
	string ret;

	for (size_t i = 0; i < str.length(); i++)
	{
		char c = str.at(i);

		if (c < '0' || c > '9')
			continue;

		ret.push_back(c);
	}

	return ret;
}

string NameFormat::toShortName(string& str)
{
	DECL_TRACER("NameFormat::toShortName(string& str)");
	string ret;
	bool ignore = false;

	for (size_t i = 0; i < str.length(); i++)
	{
		char c = str.at(i);

		if (c == ' ')
			ignore = true;

		if (ignore && c == '.')
			ignore = false;

		if (!ignore)
			ret.push_back(c);
	}

	return ret;
}

string NameFormat::transFontName(string& str)
{
	DECL_TRACER("NameFormat::transFontName(string& str)");
	string ret;

	for (size_t i = 0; i < str.length(); i++)
	{
		char c = str.at(i);

		if (c == ' ' || c == '%')
			c = '_';

		ret.push_back(c);
	}

	Str::replace(ret, ".ttf", ".woff");
	return ret;
}

string NameFormat::toURL(string& str)
{
	DECL_TRACER("NameFormat::toURL(string& str)");
	string ret;

	for (size_t i = 0; i < str.length(); i++)
	{
		char c = str.at(i);

		if (!(c >= '0' && c <= '9') && !(c >= 'a' && c <= 'z') &&
			!(c >= 'A' && c <= 'Z') && c != '_' && c != '.' &&
			c != '-' && c != '/')
		{
			ret.append("%");
			std::stringstream stream;
			stream << std::setfill ('0') << std::setw(2) << std::hex << ((int)c & 0x000000ff);
			ret.append(stream.str());
		}
		else
			ret.push_back(c);
	}

	return ret;
}

char *NameFormat::EncodeTo(char* buf, size_t *len, const string& str, const string& from, const string& to)
{
	DECL_TRACER("NameFormat::EncodeTo(char* buf, size_t *len, const string& str, const string& from, const string& to)");
	if (!buf || str.empty())
		return 0;

	iconv_t cd = iconv_open(from.c_str(), to.c_str());

	if (cd == (iconv_t) -1)
	{
		sysl->errlog("NameFormat::EncodeTo: iconv_open failed!");
		return 0;
	}

	char *in_buf = (char *)str.c_str();
	size_t in_left = str.length() - 1;

	char *out_buf = buf;
	size_t out_left = *len - 1;
	size_t new_len;

	do
	{
		if ((new_len = iconv(cd, &in_buf, &in_left, &out_buf, &out_left)) == (size_t) -1)
		{
			sysl->errlog(string("NameFormat::EncodeTo: iconv failed: ")+strerror(errno));
			return 0;
		}
	}
	while (in_left > 0 && out_left > 0);

	*out_buf = 0;
	iconv_close(cd);
	*len = out_buf - buf;
	return buf;
}

string NameFormat::textToWeb(const string& txt)
{
	DECL_TRACER("NameFormat::textToWeb(const string& txt)");
	string out;

	if (txt.empty())
		return out;

	for (size_t i = 0; i < txt.length(); i++)
	{
		unsigned char c = txt.at(i);

		if (c == '\r')
			continue;

		if (c == '\n')
			out += "<br>";
		else if (c == '&')
			out += "&amp;";
		else if (c == ';')
			out += "&semi;";
		else if (c == '<')
			out += "&lt;";
		else if (c == '>')
			out += "&gt;";
		else if (c == '\t')
			out += "&tab;";
		else if (c == '!')
			out += "&excl;";
		else if (c == '"')
			out += "&quot;";
		else if (c == '#')
			out += "&num;";
		else if (c == '\'')
			out += "&apos;";
		else if (c == '=')
			out += "&equals;";
		else if (c == '-')
			out += "&dash;";
		else if (c == '~')
			out += "&tilde;";
		else if (c == ' ')
			out += "&nbsp;";
		else
			out.push_back(c);
	}

	return out;
}

string NameFormat::toHex(int num, int width)
{
	string ret;
	std::stringstream stream;
	stream << std::setfill ('0') << std::setw(width) << std::hex << num;
	ret = stream.str();
	return ret;
}

string NameFormat::strToHex(string str, int width, bool format)
{
	DECL_TRACER("NameFormat::strToHex(string str, int width, bool format)");
	int len = 0, pos = 0, old = 0;
	int w = (format) ? 1 : width;
	string out, left, right;

	for (size_t i = 0; i < str.length(); i++)
	{
		if (len >= w)
		{
			left.append(" ");
			len = 0;
		}

		if (format && i > 0 && (pos % width) == 0)
		{
			out += toHex(old, 4)+": "+left + " | " + right + "\n";
			left.clear();
			right.clear();
			old = pos;
		}

		char c = str.at(i);
		left.append(toHex((int)(c & 0x000000ff), 2));

		if (format)
		{
			if (std::isprint(c))
				right.push_back(c);
			else
				right.push_back('.');
		}

		len++;
		pos++;
	}

	if (!format)
		return left;
	else if (pos > 0)
	{
		for (int i = 0; i < (width - (pos % width)); i++)
			left.append("   ");

		out += toHex(old, 4)+": "+left + "  | " + right;
	}

	return out;
}

string NameFormat::latin1ToUTF8(const string& str)
{
	DECL_TRACER("NameFormat::latin1ToUTF8(const string& str)");
	string out;

	for (size_t i = 0; i < str.length(); i++)
	{
		uint8_t ch = str.at(i);

		if (ch < 0x80)
		{
			out.push_back(ch);
		}
		else
		{
			out.push_back(0xc0 | ch >> 6);
			out.push_back(0x80 | (ch & 0x3f));
		}
	}

	return out;
}

string NameFormat::cp1250ToUTF8(const string& str)
{
	DECL_TRACER("NameFormat::cp1250ToUTF8(const string& str)");

	string out;

	for (size_t j = 0; j < str.length(); j++)
	{
		int i = -1;
		unsigned char ch = str.at(j);
		short utf = -1;

		if (ch < 0x80)
		{
			do
			{
				i++;

				if (__cht[i].ch == ch)
				{
					utf = __cht[i].byte;
					break;
				}
			}
			while (__cht[i].ch != 0xff);

			if (utf < 0)
				utf = ch;
		}
		else
			utf = ch;

		if (utf > 0x00ff)
		{
			out.push_back((utf >> 8) & 0x00ff);
			out.push_back(utf & 0x00ff);
		}
		else if (ch > 0x7f)
		{
			out.push_back(0xc0 | ch >> 6);
			out.push_back(0x80 | (ch & 0x3f));
		}
		else
			out.push_back(ch);
	}

	return out;
}

string NameFormat::UTF8ToCp1250(const string& str)
{
	DECL_TRACER("NameFormat::UTF8ToCp1250(const strings::String& str)");
	char dst[1024];
	size_t srclen = 0;
	char* pIn, *pInSave;

	srclen = str.length();
	memset(&dst[0], 0, sizeof(dst));

	try
	{
		pIn = new char[srclen + 1];
		memcpy(pIn, str.c_str(), srclen);
		*(pIn+srclen) = 0;
		pInSave = pIn;
	}
	catch(std::exception& e)
	{
		sysl->errlog(string("NameFormat::UTF8ToCp1250: ")+e.what());
		return "";
	}

	size_t dstlen = sizeof(dst) - 1;
	char* pOut = (char *)dst;

	iconv_t conv = iconv_open("CP1250", "UTF-8");

	if (conv == (iconv_t)-1)
	{
		sysl->errlog("NameFormat::UTF8ToCp1250: Error opening iconv!");
		delete[] pInSave;
		return str;
	}

	size_t ret = iconv(conv, &pIn, &srclen, &pOut, &dstlen);
	iconv_close(conv);
	delete[] pInSave;

	if (ret == (size_t)-1)
	{
		sysl->errlog("NameFormat::UTF8ToCp1250: Error converting a string!");
		return str;
	}

	return string(dst);
}
