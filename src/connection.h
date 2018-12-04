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

#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include <array>
#include <memory>
#ifdef __APPLE__
#include <boost/asio.hpp>
#else
#include <asio.hpp>
#endif
#include "request.h"
#include "reply.h"
#include "requesthandler.h"
#include "requestparser.h"

#ifdef __APPLE__
using namespace boost;
#endif

#define BUFFER_MAX  8192

namespace http
{
	namespace server
	{
		class ConnectionManager;

		class Connection : public std::enable_shared_from_this<Connection>
		{
			public:
				Connection(const Connection&) = delete;
				Connection& operator=(const Connection&) = delete;

				/// Construct a connection with the given socket.
				explicit Connection(asio::ip::tcp::socket socket, ConnectionManager& manager, RequestHandler& handler);

				/// Start the first asynchronous operation for the connection.
				void start();

				/// Stop all asynchronous operations associated with the connection.
				void stop();

			private:
				/// Perform an asynchronous read operation.
				void do_read();

				/// Perform an asynchronous write operation.
				void do_write();

				/// Socket for the connection.
				asio::ip::tcp::socket m_socket;

				/// The manager for this connection.
				ConnectionManager& m_connection_manager;

				/// The handler used to process the incoming request.
				RequestHandler& m_request_handler;

				/// Buffer for incoming data.
				std::array<char, BUFFER_MAX> m_buffer;

				/// The incoming request.
				Request m_request;

				/// The parser for the incoming request.
				RequestParser m_request_parser;

				/// The reply to be sent back to the client.
				Reply m_reply;
		};

		typedef std::shared_ptr<Connection> ConnectionPtr;
	} // namespace server
} // namespace http
#endif

