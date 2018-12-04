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

#include <iostream>
#include <memory>
#include <signal.h>
#include <utility>
#include "server.h"
#include "../common/syslog.h"
#include "hvl.h"

namespace http
{
	namespace server
	{
		Server::Server(const std::string& address, int port, const std::string& doc_root)
				: m_io_service(),
				m_signals(m_io_service),
				m_acceptor(m_io_service),
				m_connection_manager(),
				m_socket(m_io_service),
				m_request_handler(doc_root)
		{
			sysl->DebugMsg("Server::Server: Registering signals...");
			// Register to handle the signals that indicate when the server should exit.
			// It is safe to register for the same signal multiple times in a program,
			// provided all registration for the specified signal is made through Asio.
			m_signals.add(SIGINT);
			m_signals.add(SIGTERM);
		#if defined(SIGQUIT)
			m_signals.add(SIGQUIT);
		#endif // defined(SIGQUIT)
			sysl->DebugMsg("Server::Server: Signals registered");
			do_await_stop();
			// Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
			asio::ip::tcp::resolver resolver(m_io_service);
			asio::ip::tcp::endpoint endpoint = *resolver.resolve({address, std::to_string(port)});
			m_acceptor.open(endpoint.protocol());
			m_acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
			m_acceptor.bind(endpoint);
			m_acceptor.listen();
			do_accept();
			sysl->DebugMsg("Server::Server: Acceptor was started. Listening now.");
		}

		void Server::run()
		{
			sysl->DebugMsg("Server::run(): will be started.");
			m_io_service.run();
		}

		void Server::do_accept()
		{
			m_acceptor.async_accept(m_socket, [this](std::error_code ec)
			{
				sysl->DebugMsg("Server::do_accept: async_accept");
				// Check whether the server was stopped by a signal before this
				// completion handler had a chance to run.
				if (!m_acceptor.is_open())
				{
					sysl->DebugMsg("Server::do_accept: signal handler stopped server!");
					return;
				}

				if (!ec)
				{
					sysl->DebugMsg("Server::do_accept:: Starting connection manager.");
					m_connection_manager.start(std::make_shared<Connection>(std::move(m_socket), m_connection_manager, m_request_handler));
				}

				do_accept();
			});
		}

		void Server::do_await_stop()
		{
			m_signals.async_wait([this](std::error_code /*ec*/, int /*signo*/)
			{
				// The server is stopped by cancelling all outstanding asynchronous
				// operations. Once all operations have finished the io_service::run()
				// call will exit.
				sysl->DebugMsg("Server::do_await_stop: canceling all outstanding asynchronous operations.");
				m_acceptor.close();
				m_connection_manager.stop_all();
			});
		}
	} // namespace server
} // namespace http
