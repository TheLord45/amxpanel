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

#ifndef __NAMEFORMAT_H__
#define __NAMEFORMAT_H__

#include "strings.h"

class NameFormat
{
	public:
		NameFormat();
		~NameFormat();

		static strings::String toValidName(strings::String& str);
		static strings::String cutNumbers(strings::String& str);
		static strings::String toShortName(strings::String& str);
		static strings::String transFontName(strings::String& str);
		static strings::String toURL(strings::String& str);
};

#endif
