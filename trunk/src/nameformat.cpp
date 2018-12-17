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

		if ((c < '0' || c > '9') && (c < 'a' || c > 'z') && (c < 'A' && c > 'Z'))
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
