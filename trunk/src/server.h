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

#ifndef __SERVER_H__
#define __SERVER_H__

#ifdef __APPLE__
   #include <boost/asio.hpp>
#else
   #include <asio.hpp>
#endif

#include <string>
#include "connectionmanager.h"
#include "requesthandler.h"
#include "../common/syslog.h"

#ifdef __APPLE__
using namespace boost;
#endif

namespace http
{
	namespace server
	{
		class Server
		{
			public:
				Server(const Server&) = delete;
				Server& operator= (const Server&) = delete;

				// Construct the server to listen on the specified TCP address and port.
				explicit Server(const std::string& address, int port, const std::string& doc_root);
				// Run the server's io_service loop
				void run();

			private:
				void do_accept();
				void do_await_stop();

				asio::io_service m_io_service;
				asio::signal_set m_signals;
				asio::ip::tcp::acceptor m_acceptor;
				ConnectionManager m_connection_manager;
				asio::ip::tcp::socket m_socket;
				RequestHandler m_request_handler;
		};
	} // namespace server
} // namespace http

#endif

