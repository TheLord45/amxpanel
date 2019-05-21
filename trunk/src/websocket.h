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
typedef websocketpp::server<websocketpp::config::asio> server_ws;

// pull out the type of messages sent by our config
typedef websocketpp::config::asio::message_type::ptr message_ptr;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;

namespace amx
{
	static server sock_server;
	static server_ws sock_server_ws;
	static websocketpp::connection_hdl server_hdl;
	static bool connected = false;
	static bool cbInit = false;
	static bool cbInitStop = false;
	static bool cbInitCon = false;
	static bool cbInitRegister = false;
	static std::function<void(std::string&)> fcall;
	static std::function<void()> fcallStop;
	static std::function<void(bool, websocketpp::connection_hdl&)> fcallConn;
	static std::function<void(websocketpp::connection_hdl&, int)> fcallRegister;
	static std::map<int, websocketpp::connection_hdl> __regs;

	class WebSocket
	{
		public:
			WebSocket();
			~WebSocket();

			static void regCallback(std::function<void(std::string&)> func);
			static void regCallbackStop(std::function<void()> func);
			static void regCallbackConnected(std::function<void(bool, websocketpp::connection_hdl&)> func);
			static void regCallbackRegister(std::function<void(websocketpp::connection_hdl&, int)> func);
			void run();
			bool send(strings::String& msg);
			static server& getServer() { return sock_server; }
			static server_ws& getServer_ws() { return sock_server_ws; }
			static bool getConStatus() { return connected; }
			static void setConStatus(bool s);
			static bool compareHdl(websocketpp::connection_hdl& hdl1, websocketpp::connection_hdl& hdl2);

			enum tls_mode
			{
				MOZILLA_INTERMEDIATE = 1,
				MOZILLA_MODERN = 2
			};

		private:
			static void on_http(server* s, websocketpp::connection_hdl hdl);
			static void on_fail(server* s, websocketpp::connection_hdl hdl);
			static void on_close (websocketpp::connection_hdl hdl);
			static void on_message(server* s, websocketpp::connection_hdl hdl, message_ptr msg);
			static void on_http_ws(server_ws* s, websocketpp::connection_hdl hdl);
			static void on_fail_ws(server_ws* s, websocketpp::connection_hdl hdl);
			static void on_message_ws(server_ws* s, websocketpp::connection_hdl hdl, message_ptr msg);
			static context_ptr on_tls_init(tls_mode mode, websocketpp::connection_hdl hdl);
			static void tcp_post_init(websocketpp::connection_hdl hdl);
			static std::string getPassword();
	};
}

#endif
