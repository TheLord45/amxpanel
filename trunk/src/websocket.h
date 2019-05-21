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

typedef std::map<websocketpp::connection_hdl, int, std::owner_less<websocketpp::connection_hdl> > REG_DATA_T;

namespace amx
{
	class WebSocket
	{
		public:
			WebSocket();
			~WebSocket();

			void regCallback(std::function<void(std::string&)> func);
			void regCallbackStop(std::function<void()> func);
			void regCallbackConnected(std::function<void(bool, websocketpp::connection_hdl&)> func);
			void regCallbackRegister(std::function<void(websocketpp::connection_hdl&, int)> func);
			void run();
			bool send(strings::String& msg);
			server& getServer() { return sock_server; }
			server_ws& getServer_ws() { return sock_server_ws; }
			bool getConStatus() { return connected; }
			void setConStatus(bool s);
			bool compareHdl(websocketpp::connection_hdl& hdl1, websocketpp::connection_hdl& hdl2);

			enum tls_mode
			{
				MOZILLA_INTERMEDIATE = 1,
				MOZILLA_MODERN = 2
			};

		private:
			void on_http(server* s, websocketpp::connection_hdl hdl);
			void on_fail(server* s, websocketpp::connection_hdl hdl);
			void on_close (websocketpp::connection_hdl hdl);
			void on_message(server* s, websocketpp::connection_hdl hdl, message_ptr msg);
			void on_http_ws(server_ws* s, websocketpp::connection_hdl hdl);
			void on_fail_ws(server_ws* s, websocketpp::connection_hdl hdl);
			void on_message_ws(server_ws* s, websocketpp::connection_hdl hdl, message_ptr msg);
			context_ptr on_tls_init(tls_mode mode, websocketpp::connection_hdl hdl);
			void tcp_post_init(websocketpp::connection_hdl hdl);
			std::string getPassword();

			server sock_server;
			server_ws sock_server_ws;
			websocketpp::connection_hdl server_hdl;
			bool connected = false;
			bool cbInit = false;
			bool cbInitStop = false;
			bool cbInitCon = false;
			bool cbInitRegister = false;
			std::function<void(std::string&)> fcall;
			std::function<void()> fcallStop;
			std::function<void(bool, websocketpp::connection_hdl&)> fcallConn;
			std::function<void(websocketpp::connection_hdl&, int)> fcallRegister;
			REG_DATA_T __regs;

	};
}

#endif
