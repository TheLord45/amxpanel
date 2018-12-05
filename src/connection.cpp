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

#include <utility>
#include <vector>
#include <memory>
#include <unistd.h>
#ifdef __APPLE__
#include <boost/asio.hpp>
#else
#include <asio.hpp>
#endif
#include "connection.h"
#include "connectionmanager.h"
#include "requesthandler.h"
#include "syslog.h"
#include "config.h"

using namespace std;
using namespace strings;

extern Config *Configuration;
extern string pName;
extern Syslog *sysl;

#ifdef __APPLE__
using namespace boost;
#endif

namespace http
{
	namespace server
	{
		Connection::Connection(asio::ip::tcp::socket socket, ConnectionManager& manager, RequestHandler& handler)
				: m_socket(std::move(socket)),
				m_connection_manager(manager),
				m_request_handler(handler)
		{
		}

		void Connection::start()
		{
			do_read();
		}

		void Connection::stop()
		{
			m_socket.close();
		}

		void Connection::do_read()
		{
			auto self(shared_from_this());
#ifdef __APPLE__
			m_socket.async_read_some(asio::buffer(m_buffer), [this, self](system::error_code ec, std::size_t bytes_transferred)
#else
			m_socket.async_read_some(asio::buffer(m_buffer), [this, self](std::error_code ec, std::size_t bytes_transferred)
#endif
			{
				if (!ec)
				{
					RequestParser::result_type result;

					if (bytes_transferred > BUFFER_MAX)
						bytes_transferred = BUFFER_MAX - 1;

					std::tie(result, std::ignore) = m_request_parser.parse(m_request, m_buffer.data(), m_buffer.data() + bytes_transferred);

					if (result == RequestParser::good)
					{
						// Find length of header
						int clen = 0;

						for (size_t i = 0; i < m_request.headers.size(); i++)
						{
							if (m_request.headers[i].name.compare("Content-Length") == 0)
							{
								clen = atoi(m_request.headers[i].value.c_str());
								break;
							}
						}

						std::string buffer;
						buffer.resize(m_buffer.size()+1);
						buffer.assign(m_buffer.data(), m_buffer.size());
						buffer.push_back(0);
						size_t hlen = bytes_transferred - clen;

						if (m_request.method.compare("POST") == 0 &&
								hlen != bytes_transferred && clen > 0 &&
								hlen < bytes_transferred)
						{
							m_request.content = buffer.substr(hlen, clen);
							sysl->DebugMsg("Connection::do_read: Content: "+m_request.content);
						}

						m_request_handler.HandleRequest(m_request, m_reply);
						do_write();
					}
					else if (result == RequestParser::bad)
					{
						m_reply = Reply::stock_reply(Reply::bad_request);
						do_write();
					}
					else
					{
						do_read();
					}
				}
				else if (ec != asio::error::operation_aborted)
				{
					m_connection_manager.stop(shared_from_this());
				}
			});
		}

		void Connection::do_write()
		{
			auto self(shared_from_this());
#ifdef __APPLE__
			asio::async_write(m_socket, m_reply.to_buffers(), [this, self](system::error_code ec, std::size_t)
#else
			asio::async_write(m_socket, m_reply.to_buffers(), [this, self](std::error_code ec, std::size_t)
#endif
			{
				if (!ec)
				{
					// Initiate graceful connection closure.
#ifdef __APPLE__
					system::error_code ignored_ec;
#else
					asio::error_code ignored_ec;
#endif
					m_socket.shutdown(asio::ip::tcp::socket::shutdown_both, ignored_ec);
				}

				if (ec != asio::error::operation_aborted)
				{
					m_connection_manager.stop(shared_from_this());
				}
			});
		}
	} // namespace server
} // namespace http
