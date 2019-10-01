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

#include <iostream>
#include "config.h"
#include "syslog.h"
#include "websocket.h"
#include "str.h"
#include "trace.h"

extern Config *Configuration;
extern Syslog *sysl;

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

using namespace amx;
using namespace std;

WebSocket::WebSocket()
{
	sysl->TRACE(Syslog::ENTRY, "WebSocket::WebSocket()");
	cbInit = false;
	cbInitStop = false;
	cbInitCon = false;
	cbInitRegister = false;
	websocketsLock = PTHREAD_RWLOCK_INITIALIZER;
}

void WebSocket::regCallback(function<void(string&, long)> func)
{
	DECL_TRACER("WebSocket::regCallback(function<void(string&, long)> func)");

    if (func == nullptr)
        return;

	fcall = func;
	cbInit = true;
}

void WebSocket::regCallbackStop(function<void()> func)
{
	DECL_TRACER("WebSocket::regCallbackStop(function<void()> func)");

    if (func == nullptr)
         return;

	fcallStop = func;
	cbInitStop = true;
}

void WebSocket::regCallbackConnected(function<void (bool, long)> func)
{
	DECL_TRACER("WebSocket::regCallbackConnected(function<void (bool, long)> func)");

    if (func == nullptr)
        return;

	fcallConn = func;
	cbInitCon = true;
}

void WebSocket::regCallbackRegister(function<void (long, int)> func)
{
	DECL_TRACER("WebSocket::regCallbackRegister(function<void (long, int)> func)");

    if (func == nullptr)
        return;

	fcallRegister = func;
	cbInitRegister = true;
}

void WebSocket::setConStatus(bool s, long pan)
{
	DECL_TRACER(string("WebSocket::setConStatus(bool s, long pan) [")+((s)?"TRUE":"FALSE")+"]");

	if (cbInitCon)
		fcallConn(s, pan);
	else
		sysl->warnlog("WebSocket::setConStatus: Callback function to indicate connection status was not set!");
}

void WebSocket::run()
{
	DECL_TRACER("WebSocket::run()");
	bool stopped = false;
	// Create a server endpoint

	try
	{
		if (Configuration->getWSStatus())
		{
			sysl->TRACE("WebSocket::run: Using encrypted communication.");
			// Set logging settings
			sock_server.set_access_channels(websocketpp::log::alevel::all);
			sock_server.clear_access_channels(websocketpp::log::alevel::frame_payload);
			// Initialize ASIO
			sock_server.init_asio();
			sock_server.set_reuse_addr(true);
			// Register our message handler
			sock_server.set_message_handler(bind(&WebSocket::on_message, this, &sock_server,::_1,::_2));
//			sock_server.set_http_handler(bind(&WebSocket::on_http, this, &sock_server, ::_1));
			sock_server.set_fail_handler(bind(&WebSocket::on_fail, this, &sock_server, ::_1));
			sock_server.set_close_handler(bind(&WebSocket::on_close, this, ::_1));
			sock_server.set_tls_init_handler(bind(&WebSocket::on_tls_init, this, MOZILLA_MODERN, ::_1));
			sock_server.set_tcp_post_init_handler(bind(&WebSocket::tcp_post_init, this, ::_1));
			// Listen on port 11012
			sock_server.listen(Configuration->getSidePort());
			// Start the server accept loop
			sock_server.start_accept();
			// Start the ASIO io_service run loop
			sock_server.run();
		}
		else
		{
			sysl->TRACE("WebSocket::run: Using plain communication!");
			// Set logging settings
			sock_server_ws.set_access_channels(websocketpp::log::alevel::all);
			sock_server_ws.clear_access_channels(websocketpp::log::alevel::frame_payload);
			// Initialize ASIO
			sock_server_ws.init_asio();
			sock_server_ws.set_reuse_addr(true);
			// Register our message handler
			sock_server_ws.set_message_handler(bind(&WebSocket::on_message_ws, this, &sock_server_ws, ::_1, ::_2));
//			sock_server_ws.set_http_handler(bind(&WebSocket::on_http_ws, this, &sock_server_ws, ::_1));
			sock_server_ws.set_fail_handler(bind(&WebSocket::on_fail_ws, this, &sock_server_ws, ::_1));
			sock_server_ws.set_close_handler(bind(&WebSocket::on_close, this, ::_1));
			sock_server_ws.set_tcp_post_init_handler(bind(&WebSocket::tcp_post_init, this, ::_1));
			// Listen on port 11012
			sock_server_ws.listen(Configuration->getSidePort());
			// Start the server accept loop
			sock_server_ws.start_accept();
			// Start the ASIO io_service run loop
			sock_server_ws.run();
		}
	}
	catch (websocketpp::exception const & e)
	{
		sysl->errlog(string("WebSocket::run: WEBSocketPP exception:")+e.what());
		stopped = true;

		if (cbInitStop)
			fcallStop();
	}
	catch (const exception & e)
	{
		sysl->errlog(string("WebSocket::run: ")+e.what());
		stopped = true;

		if (cbInitStop)
			fcallStop();
	}
	catch (...)
	{
		sysl->errlog("WebSocket::run: Other exception!");
		stopped = true;

		if (cbInitStop)
			fcallStop();
	}

	if (!stopped)
	{
		if (cbInitStop)
			fcallStop();
	}
}

WebSocket::~WebSocket()
{
	websocketpp::lib::error_code ec;

	if (Configuration->getWSStatus())
		sock_server.stop_listening(ec);
	else
		sock_server_ws.stop_listening(ec);

	if (ec)
	{
		sysl->errlog("WebSocket::~WebSocket: Error stopping listening: "+ec.message());
		sysl->TRACE(Syslog::EXIT, "WebSocket::~WebSocket()");
		return;
	}

	REG_DATA_T::iterator itr;
	string data = "Terminating connection...";
	size_t i = 0;

	for (itr = __regs.begin(); itr != __regs.end(); ++itr)
	{
		if (Configuration->getWSStatus())
			sock_server.close(itr->first, websocketpp::close::status::normal, data, ec);
		else
			sock_server_ws.close(itr->first, websocketpp::close::status::normal, data, ec);

		if (ec)
			sysl->errlog(string("WebSocket::~WebSocket: ")+ec.message());

		i++;

		if (i >= __regs.size())
			break;
	}

	if (Configuration->getWSStatus())
		sock_server.stop();
	else
		sock_server_ws.stop();

	sysl->TRACE(Syslog::EXIT, "WebSocket::~WebSocket()");
}

bool WebSocket::send(string& msg, long pan)
{
	DECL_TRACTHR("WebSocket::send(strings::String& msg, long pan)");

	REG_DATA_T::iterator itr;
	bool found = false;
	websocketpp::connection_hdl hdl;

	for (itr = __regs.begin(); itr != __regs.end(); ++itr)
	{
		if (itr->second.ID == pan)
		{
			hdl = itr->first;
			found = true;
			break;
		}
	}

	if (!found)
	{
		sysl->errlogThr("WebSocket::send: Unknown websocket handle!");
		return false;
	}

	try
	{
		if (Configuration->getWSStatus())
			sock_server.send(hdl, msg, websocketpp::frame::opcode::text);
		else
			sock_server_ws.send(hdl, msg, websocketpp::frame::opcode::text);
	}
	catch (websocketpp::exception const & e)
	{
		sysl->errlogThr(string("WebSocket::send: Error sending a message: ")+e.what());

		if (Configuration->getWSStatus())
			sock_server.close(hdl, 0, string(e.what()));
		else
			sock_server_ws.close(hdl, 0, string(e.what()));

		setConStatus(false, pan);
		return false;
	}

	return true;
}

void WebSocket::tcp_post_init(connection_hdl hdl)
{
	DECL_TRACTHR("WebSocket::tcp_post_init(websocketpp::connection_hdl hdl)");
	server_hdl = hdl;

	if (!cbInitRegister)
	{
		sysl->errlogThr("WebSocket::tcp_post_init: Callback function for registering web socket connection was not initialized!");
		return;
	}

	PAN_ID_T pid;
	pid.channel = 0;
	pid.ID = random();
	string ip;

	if (Configuration->getWSStatus())
		ip = sock_server.get_con_from_hdl(hdl)->get_remote_endpoint();
	else
		ip = sock_server_ws.get_con_from_hdl(hdl)->get_remote_endpoint();

	pid.ip = cutIpAddress(ip);

	__regs.insert(pair<websocketpp::connection_hdl, PAN_ID_T>(hdl, pid));
	sysl->DebugMsg("WebSocket::tcp_post_init: Registering pan "+to_string(pid.ID)+" for remote "+pid.ip, true);
	fcallRegister(pid.ID, 0);
}

// Define a callback to handle incoming messages
void WebSocket::on_message(server* s, connection_hdl hdl, message_ptr msg)
{
	DECL_TRACTHR("WebSocket::on_message(server* s, websocketpp::connection_hdl hdl, message_ptr msg)");

	REG_DATA_T::iterator key;
	bool validKey = false;
	key = __regs.find(hdl);

	if (key != __regs.end())
		validKey = true;

	if (validKey)
		setConStatus(true, key->second.ID);

	server_hdl = hdl;
	server::connection_ptr con = s->get_con_from_hdl(hdl);

	if (validKey && key->second.ip.length() == 0)
	{
		string ip = con->get_remote_endpoint();
		key->second.ip = cutIpAddress(ip);
		sysl->DebugMsg("WebSocket::on_message: Endpoint: "+key->second.ip, true);
	}

	string send = msg->get_payload();
	sysl->TRACE("WebSocket::on_message: Called with hdl: message: "+send, true);
	int id = 0;
	long pan = 0;

	if (send.find("REGISTER:") == string::npos)
	{
		vector<string> parts = Str::split(send, ":");

		if (parts.size() > 0)
			id = atoi(parts[1].c_str());
	}

	if (!cbInit)
	{
		sysl->warnlogThr("WebSocket::on_message: No callback function registered!");
		return;
	}

	if (!validKey && id >= 10000 && id <= 11000)
	{
		PAN_ID_T reg;
		reg.channel = id;
		reg.ID = random();
		__regs.insert(pair<connection_hdl, PAN_ID_T>(hdl, reg));
		sysl->DebugMsg("WebSocket::on_message: Registering id "+to_string(id)+" for pan "+to_string(reg.ID), true);
		fcallRegister(reg.ID, id);
		pan = reg.ID;
	}
	else if (validKey && id >= 10000 && id <= 11000 && key->second.channel == 0)
	{
		pan = key->second.ID;
		key->second.channel = id;
		sysl->DebugMsg("WebSocket::on_message: Channel was set to "+to_string(id)+" for pan "+to_string(pan), true);
	}
	else if (validKey)
		pan = key->second.ID;

	if (send.find("PANEL:") != string::npos)
		return;

	fcall(send, pan);
}

void WebSocket::on_message_ws(server_ws* s, connection_hdl hdl, message_ptr msg)
{
	DECL_TRACTHR("WebSocket::on_message_ws(server_ws* s, websocketpp::connection_hdl hdl, message_ptr msg)");

	REG_DATA_T::iterator key;
	bool validKey = false;
	key = __regs.find(hdl);

	if (key != __regs.end())
		validKey = true;

	if (validKey)
		setConStatus(true, key->second.ID);

	server_hdl = hdl;
	server_ws::connection_ptr con = s->get_con_from_hdl(hdl);

	if (validKey && key->second.ip.length() == 0)
	{
		string ip = con->get_remote_endpoint();
		key->second.ip = cutIpAddress(ip);
		sysl->DebugMsg("WebSocket::on_message_ws: Endpoint: "+key->second.ip, true);
	}

	string send = msg->get_payload();
	sysl->TRACE("WebSocket::on_message_ws: Called with hdl: message: "+send, true);
	int id = 0;
	long pan = 0;

	if (send.find_first_of("REGISTER:") == string::npos)
	{
		vector<string> parts = Str::split(send, ":");

		if (parts.size() > 0)
			id = atoi(parts[1].c_str());
	}

	if (!cbInit)
	{
		sysl->warnlogThr("WebSocket::on_message_ws: No callback function registered!");
		return;
	}

	if (id >= 10000 && id <= 11000 && !validKey)
	{
		PAN_ID_T reg;
		reg.channel = id;
		reg.ID = random();
		__regs.insert(pair<connection_hdl, PAN_ID_T>(hdl, reg));
		sysl->DebugMsg("WebSocket::on_message_ws: Registering id "+to_string(id)+" for pan "+to_string(reg.ID), true);
		fcallRegister(reg.ID, id);
		pan = reg.ID;
	}
	else if (validKey && id >= 10000 && id <= 11000 && key->second.channel == 0)
	{
		pan = key->second.ID;
		key->second.channel = id;
		sysl->DebugMsg("WebSocket::on_message_ws: Channel was set to "+to_string(id)+" for pan "+to_string(pan), true);
	}
	else if (validKey)
		pan = key->second.ID;

	if (send.find("PANEL:") != string::npos)
		return;

	fcall(send, pan);
}

void WebSocket::on_fail(server* s, connection_hdl hdl)
{
	DECL_TRACTHR("WebSocket::on_fail(server* s, websocketpp::connection_hdl hdl)");
	server_hdl = hdl;
	server::connection_ptr con = s->get_con_from_hdl(hdl);
	sysl->errlogThr(string("WebSocket::on_fail: Fail handler: ")+con->get_ec().message());
	long pan = getPanelID(hdl);
	setConStatus(false, pan);
	websocketpp::lib::error_code ec;
	sock_server.close(hdl, websocketpp::close::status::normal, "Terminating connection ...", ec);

	if (ec)
		sysl->errlogThr(string("WebSocket::on_fail: ")+ec.message());

	REG_DATA_T::iterator key = __regs.find(hdl);

	if (key != __regs.end())
		__regs.erase(key);

	fcallRegister(pan, -1);
}

void WebSocket::on_fail_ws(server_ws* s, connection_hdl hdl)
{
	DECL_TRACTHR("WebSocket::on_fail_ws(server_ws* s, websocketpp::connection_hdl hdl)");
	server_hdl = hdl;
	server_ws::connection_ptr con = s->get_con_from_hdl(hdl);
	sysl->errlogThr(string("WebSocket::on_fail_ws: Fail handler: ")+con->get_ec().message());
	long pan = getPanelID(hdl);
	setConStatus(false, pan);
	websocketpp::lib::error_code ec;
	sock_server_ws.close(hdl, websocketpp::close::status::normal, "Terminating connection ...", ec);

	if (ec)
		sysl->errlogThr(string("WebSocket::on_fail_ws: ")+ec.message());

	REG_DATA_T::iterator key = __regs.find(hdl);

	if (key != __regs.end())
		__regs.erase(key);

	fcallRegister(pan, -1);
}

void WebSocket::on_close(connection_hdl hdl)
{
	DECL_TRACTHR("WebSocket::on_close(websocketpp::connection_hdl)");
	server_hdl = hdl;
	long pan = getPanelID(hdl);
	setConStatus(false, pan);
	websocketpp::lib::error_code ec;

	if (Configuration->getWSStatus())
		sock_server.close(hdl, websocketpp::close::status::normal, "Terminating TLS connection ...", ec);
	else
		sock_server_ws.close(hdl, websocketpp::close::status::normal, "Terminating connection ...", ec);

	if (ec)
		sysl->errlog(string("WebSocket::on_close: ")+ec.message());

	REG_DATA_T::iterator key = __regs.find(hdl);

	if (key != __regs.end())
		__regs.erase(key);

	fcallRegister(pan, -1);
	sysl->TRACE("WebSocket::on_close: Connection for pan "+to_string(pan)+" terminated.", true);
}

context_ptr WebSocket::on_tls_init(tls_mode mode, connection_hdl hdl)
{
	DECL_TRACTHR("WebSocket::on_tls_init(tls_mode mode, websocketpp::connection_hdl hdl)");
	namespace asio = websocketpp::lib::asio;
	server_hdl = hdl;

	sysl->TRACE(string("WebSocket::on_tls_init: Using TLS mode: ")+(mode == MOZILLA_MODERN ? "Mozilla Modern" : "Mozilla Intermediate"), true);

	context_ptr ctx = websocketpp::lib::make_shared<asio::ssl::context>(asio::ssl::context::sslv23);

	try
	{
		if (mode == MOZILLA_MODERN)
		{
			// Modern disables TLSv1
			ctx->set_options(asio::ssl::context::default_workarounds |
//			asio::ssl::context::no_sslv2 |
//			asio::ssl::context::no_sslv3 |
			asio::ssl::context::no_tlsv1 |
			asio::ssl::context::single_dh_use);
		}
		else
		{
			ctx->set_options(asio::ssl::context::default_workarounds |
//			asio::ssl::context::no_sslv2 |
//			asio::ssl::context::no_sslv3 |
			asio::ssl::context::single_dh_use);
		}

		if (!Configuration->getSSHPassword().empty())
			ctx->set_password_callback(bind(&WebSocket::getPassword, this));

		sysl->TRACE("WebSocket::on_tls_init: Reading certificate chain file: "+Configuration->getSSHServerFile(), true);
		ctx->use_certificate_chain_file(Configuration->getSSHServerFile());
		sysl->TRACE("WebSocket::on_tls_init: Reading private key file: "+Configuration->getSSHServerFile(), true);
		ctx->use_private_key_file(Configuration->getSSHServerFile(), asio::ssl::context::pem);

		// Example method of generating this file:
		// `openssl dhparam -out dh.pem 2048`
		// Mozilla Intermediate suggests 1024 as the minimum size to use
		// Mozilla Modern suggests 2048 as the minimum size to use.
		sysl->TRACE("WebSocket::on_tls_init: Reading DH parameter file: "+Configuration->getSSHDHFile(), true);
		ctx->use_tmp_dh_file(Configuration->getSSHDHFile());

		string ciphers;

		if (mode == MOZILLA_MODERN)
		{
			ciphers = "ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-DSS-AES128-GCM-SHA256:kEDH+AESGCM:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:DHE-DSS-AES256-SHA:DHE-RSA-AES256-SHA:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!3DES:!MD5:!PSK";
		}
		else
		{
			ciphers = "ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-DSS-AES128-GCM-SHA256:kEDH+AESGCM:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:DHE-DSS-AES256-SHA:DHE-RSA-AES256-SHA:AES128-GCM-SHA256:AES256-GCM-SHA384:AES128-SHA256:AES256-SHA256:AES128-SHA:AES256-SHA:AES:CAMELLIA:DES-CBC3-SHA:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!MD5:!PSK:!aECDH:!EDH-DSS-DES-CBC3-SHA:!EDH-RSA-DES-CBC3-SHA:!KRB5-DES-CBC3-SHA";
		}

		if (SSL_CTX_set_cipher_list(ctx->native_handle() , ciphers.c_str()) != 1)
		{
			sysl->errlogThr("WebSocket::on_tls_init: Error setting cipher list");
		}
	}
	catch (exception& e)
	{
		sysl->errlogThr(string("WebSocket::on_tls_init: Exception: ")+e.what());
	}

	return ctx;
}

string WebSocket::getPassword()
{
	DECL_TRACTHR("WebSocket::getPassword()");
	return Configuration->getSSHPassword();
}

long WebSocket::getPanelID(websocketpp::connection_hdl hdl)
{
	DECL_TRACTHR("WebSocket::getPanelID(websocketpp::connection_hdl hdl)");

	if (hdl.expired())
		return 0;

	REG_DATA_T::iterator key;

	if ((key = __regs.find(hdl)) != __regs.end())
		return key->second.ID;

	return 0;
}

string WebSocket::getIP(int pan)
{
	DECL_TRACTHR("WebSocket::getIP(int pan)");

	REG_DATA_T::iterator key;

	for (key = __regs.begin(); key != __regs.end(); ++key)
	{
		if (key->second.ID == pan)
			return key->second.ip;
	}

	return "";
}

string WebSocket::cutIpAddress(string& addr)
{
	DECL_TRACTHR("WebSocket::cutIpAddress(strings::String& addr)");
	size_t pos1 = addr.find("[");
	size_t pos2 = addr.find("]");
	string ip;

	if (pos1 != string::npos && pos2 != string::npos && pos1 < pos2)
		ip = addr.substr(pos1 + 1, pos2 - pos1 - 1);

	return ip;
}

