/*
 * Copyright (C) 2018 by Andreas Theofilu <andreas@theosys.at>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

#ifndef __WEBSOCKET_H__
#define __WEBSOCKET_H__

#include <websocketpp/config/asio.hpp>
#include <websocketpp/server.hpp>
#include <atomic>

typedef websocketpp::server<websocketpp::config::asio_tls> server;
typedef websocketpp::server<websocketpp::config::asio> server_ws;

// pull out the type of messages sent by our config
typedef websocketpp::config::asio::message_type::ptr message_ptr;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;

namespace amx
{
	typedef struct PAN_ID_T
	{
		int channel;
		long ID;
		std::string ip;

		PAN_ID_T& operator= (PAN_ID_T& pid) {
			channel = pid.channel;
			ID = pid.ID;
			ip = pid.ip;
			return pid;
		}
	}PAN_ID_T;

	typedef std::map<websocketpp::connection_hdl, PAN_ID_T, std::owner_less<websocketpp::connection_hdl> > REG_DATA_T;

	class WebSocket
	{
		public:
			WebSocket();
			~WebSocket();

			void regCallback(std::function<void(std::string&, long)> func);
			void regCallbackStop(std::function<void()> func);
			void regCallbackConnected(std::function<void(bool, long)> func);
			void regCallbackRegister(std::function<void(long, int)> func);
			void run();
			bool send(std::string& msg, long pan);
			server& getServer() { return sock_server; }
			server_ws& getServer_ws() { return sock_server_ws; }
			void setConStatus(bool s, long pan);
			std::string getIP(int pan);

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
			long getPanelID(websocketpp::connection_hdl hdl);
			std::string cutIpAddress(std::string& addr);

			server sock_server;
			server_ws sock_server_ws;
			websocketpp::connection_hdl server_hdl;
			pthread_rwlock_t websocketsLock;

			bool cbInit;
			bool cbInitStop;
			bool cbInitCon;
			bool cbInitRegister;
			std::function<void(std::string&, long)> fcall;
			std::function<void()> fcallStop;
			std::function<void(bool, long)> fcallConn;
			std::function<void(long, int)> fcallRegister;
			REG_DATA_T __regs;
	};
}

#endif
