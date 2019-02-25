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

#include <string>
#include <ios>
#include <iomanip>
#include <iconv.h>
#include "syslog.h"
#include "nameformat.h"

extern Syslog *sysl;

using namespace strings;
using namespace std;

NameFormat::NameFormat()
{
	sysl->TRACE(Syslog::ENTRY, String("NameFormat::NameFormat()"));
}

NameFormat::~NameFormat()
{
	sysl->TRACE(Syslog::EXIT, String("NameFormat::NameFormat()"));
}

String NameFormat::toValidName(String& str)
{
	String ret;

	for (size_t i = 0; i < str.length(); i++)
	{
		char c = str.at(i);

		if (!(c >= '0' && c <= '9') && !(c >= 'a' && c <= 'z') && !(c >= 'A' && c <= 'Z'))
			c = '_';

		ret.append(c);
	}

	return ret;
}

String NameFormat::cutNumbers(String& str)
{
	String ret;

	for (size_t i = 0; i < str.length(); i++)
	{
		char c = str.at(i);

		if (c < '0' || c > '9')
			continue;

		ret.append(c);
	}

	return ret;
}

String NameFormat::toShortName(String& str)
{
	String ret;
	bool ignore = false;

	for (size_t i = 0; i < str.length(); i++)
	{
		char c = str.at(i);

		if (c == ' ')
			ignore = true;

		if (ignore && c == '.')
			ignore = false;

		if (!ignore)
			ret.append(c);
	}

	return ret;
}

String NameFormat::transFontName(String& str)
{
	String ret;

	for (size_t i = 0; i < str.length(); i++)
	{
		char c = str.at(i);

		if (c == ' ' || c == '%')
			c = '_';

		ret.append(c);
	}

	ret.replace(".ttf", ".woff", strings::LAST);
	return ret;
}

String NameFormat::toURL(String& str)
{
	String ret;

	for (size_t i = 0; i < str.length(); i++)
	{
		char c = str.at(i);

		if (!(c >= '0' && c <= '9') && !(c >= 'a' && c <= 'z') &&
			!(c >= 'A' && c <= 'Z') && c != '_' && c != '.' &&
			c != '-' && c != '/')
		{
			ret.append("%");
			std::stringstream stream;
			stream << std::setfill ('0') << std::setw(2) << std::hex << (int)c;
			ret.append(stream.str());
		}
		else
			ret.append(c);
	}

	return ret;
}

char *NameFormat::EncodeTo(char* buf, size_t *len, const String& str, const String& from, const String& to)
{
	if (!buf || str.empty())
		return 0;

	iconv_t cd = iconv_open(from.data(), to.data());

	if (cd == (iconv_t) -1)
	{
		sysl->errlog(String("NameFormat::EncodeTo: iconv_open failed!"));
		return 0;
	}

	char *in_buf = (char *)str.data();
	size_t in_left = str.length() - 1;

	char *out_buf = buf;
	size_t out_left = *len - 1;
	size_t new_len;

	do
	{
		if ((new_len = iconv(cd, &in_buf, &in_left, &out_buf, &out_left)) == (size_t) -1)
		{
			sysl->errlog(String("NameFormat::EncodeTo: iconv failed: ")+strerror(errno));
			return 0;
		}
	}
	while (in_left > 0 && out_left > 0);

	*out_buf = 0;
	iconv_close(cd);
	*len = out_buf - buf;
	return buf;
}

String NameFormat::textToWeb(const String& txt)
{
	String out;

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
		else
			out += (char)c;
	}

	return out;
}

String NameFormat::toHex(int num, int width)
{
	String ret;
	std::stringstream stream;
	stream << std::setfill ('0') << std::setw(width) << std::hex << num;
	ret = stream.str();
	return ret;
}

String NameFormat::strToHex(String str, int width, bool format)
{
	int len = 0, pos = 0, old = 0;
	int w = (format) ? 1 : width;
	String out, left, right;

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
				right.append(c);
			else
				right.append('.');
		}

		len++;
		pos++;
	}

	if (!format)
		return left;
	else if (pos > 0)
	{
		for (int i = 0; i < (pos % width); i++)
			left.append("   ");

		out += toHex(old, 4)+": "+left + "  | " + right;
	}

	return out;
}

String NameFormat::latin1ToUTF8(const String& str)
{
	sysl->TRACE(String("NameFormat::latin1ToUTF8(const String& str)"));
	String out;

	for (size_t i = 0; i < str.length(); i++)
	{
		uint8_t ch = str.at(i);

		if (ch < 0x80)
		{
			out.append((char)ch);
		}
		else
		{
			out.append((char)(0xc0 | ch >> 6));
			out.append((char)(0x80 | (ch & 0x3f)));
		}
	}

	return out;
}

String NameFormat::cp1250ToUTF8(const String& str)
{
	sysl->TRACE(String("NameFormat::cp1250ToUTF8(const String& str)"));

	String out;

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
			out.append((char)((utf >> 8) & 0x00ff));
			out.append((char)(utf & 0x00ff));
		}
		else if (ch > 0x7f)
		{
			out.append((char)(0xc0 | ch >> 6));
			out.append((char)(0x80 | (ch & 0x3f)));
		}
		else
			out.append((char)ch);
	}

	return out;
}