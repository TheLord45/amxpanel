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

#ifndef __WEBSOCKET_H__
#define __WEBSOCKET_H__

#include <websocketpp/config/asio.hpp>
#include <websocketpp/server.hpp>
#include "strings.h"

typedef websocketpp::server<websocketpp::config::asio_tls> server;

// pull out the type of messages sent by our config
typedef websocketpp::config::asio::message_type::ptr message_ptr;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;

namespace amx
{
	static server sock_server;
	static bool connected = false;
	static bool cbInit = false;
    static bool cbInitStop = false;
	static std::function<void(std::string&)> fcall;
	static std::function<void()> fcallStop;

	class WebSocket
	{
		public:
			WebSocket();
			~WebSocket();

			static void regCallback(std::function<void(std::string&)> func) { fcall = func; cbInit = true; }
			static void regCallbackStop(std::function<void()> func) { fcallStop = func; cbInitStop = true; }
			void run();
			bool send(strings::String& msg);
			static server& getServer() { return sock_server; }
			static bool getConStatus() { return connected; }
			static void setConStatus(bool s) { connected = s; }

			enum tls_mode
			{
				MOZILLA_INTERMEDIATE = 1,
				MOZILLA_MODERN = 2
			};

		private:
			static void on_http(server* s, websocketpp::connection_hdl hdl);
			static void on_fail(server* s, websocketpp::connection_hdl hdl);
			static void on_close(websocketpp::connection_hdl);
			static void on_message(server* s, websocketpp::connection_hdl hdl, message_ptr msg);
			static std::string getPassword();
			static context_ptr on_tls_init(tls_mode mode, websocketpp::connection_hdl hdl);
	};
}

#endif
