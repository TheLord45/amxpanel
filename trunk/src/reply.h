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

#ifndef __REPLY_H__
#define __REPLY_H__

#include <string>
#include <vector>
#ifdef __APPLE__
#include <boost/asio.hpp>
#else
#include <asio.hpp>
#endif
#include "header.h"

#ifdef __APPLE__
using namespace boost;
#endif

namespace http
{
	namespace server
	{
		struct Reply
		{
			/// The status of the reply.
			enum status_type
			{
				ok = 200,
				created = 201,
				accepted = 202,
				no_content = 204,
				multiple_choices = 300,
				moved_permanently = 301,
				moved_temporarily = 302,
				not_modified = 304,
				bad_request = 400,
				unauthorized = 401,
				forbidden = 403,
				not_found = 404,
				internal_server_error = 500,
				not_implemented = 501,
				bad_gateway = 502,
				service_unavailable = 503
			} status;

			/// The headers to be included in the reply.
			std::vector<header> headers;

			/// The content to be sent in the reply.
			std::string content;

			/// Convert the reply into a vector of buffers. The buffers do not own the
			/// underlying memory blocks, therefore the reply object must remain valid and
			/// not be changed until the write operation has completed.
			std::vector<asio::const_buffer> to_buffers();

			/// Get a stock reply.
			static Reply stock_reply(status_type status);
		};
	} // namespace server
} // namespace http

#endif

