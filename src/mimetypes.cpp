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

#include "mimetypes.h"

namespace http
{
	namespace server
	{
		namespace mime_types
		{
			struct mapping
			{
				const char* extension;
				const char* mime_type;
			} mappings[] =
			{
				{ "gif", "image/gif" },
				{ "htm", "text/html" },
				{ "html", "text/html" },
				{ "inc", "text/html" },
				{ "jpg", "image/jpeg" },
				{ "png", "image/png" },
				{ "xml", "text/xml" },
				{ "css", "text/css" },
				{ "js", "text/javascript" },
				{ "json", "text/json" }
			};

			std::string extension_to_type(const std::string& extension)
			{
				for (mapping m: mappings)
				{
					if (m.extension == extension)
						return m.mime_type;
				}

				return "text/plain";
			}

		} // namespace mime_types
	} // namespace server
} // namespace http