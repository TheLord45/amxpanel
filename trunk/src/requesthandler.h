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

#ifndef __REQUESTHANDLER_H__
#define __REQUESTHANDLER_H__

#include <string>

namespace http
{
	namespace server
	{
		struct Reply;
		struct Request;

		class RequestHandler
		{
			public:
				RequestHandler(const RequestHandler&) = delete;
				RequestHandler& operator=(const RequestHandler&) = delete;

				explicit RequestHandler(const std::string& doc_root);

				/// Handle a request and produce a reply.
				void HandleRequest(const Request& req, Reply& rep);

			private:
				bool testURL(const std::string& URL);
				/// The directory containing the files to be served.
				std::string m_doc_root;
				/// Perform URL-decoding on a string. Returns false if the encoding was
				/// invalid.
				std::vector<std::string> URIs;
				static bool url_decode(const std::string& in, std::string& out);
		};
	} // namespace server
} // namespace http

#endif

