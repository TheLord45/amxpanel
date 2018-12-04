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

#ifndef HTTP_MIME_TYPES_HPP
#define HTTP_MIME_TYPES_HPP

#include <string>

namespace http
{
	namespace server
	{
		namespace mime_types
		{
			/// Convert a file extension into a MIME type.
			std::string extension_to_type(const std::string& extension);
		} // namespace mime_types
	} // namespace server
} // namespace http

#endif // HTTP_MIME_TYPES_HPP