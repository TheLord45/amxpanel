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
#include "config.h"
#include "syslog.h"
#include "websocket.h"

extern Config *Configuration;
extern Syslog *sysl;

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

using namespace amx;

WebSocket::WebSocket()
{
	sysl->TRACE(Syslog::ENTRY, std::string("WebSocket::WebSocket()"));
	cbInit = false;
	cbInitStop = false;
	cbInitCon = false;
	cbInitRegister = false;
}

void WebSocket::regCallback(std::function<void(std::string&, long)> func)
{
	sysl->TRACE(std::string("WebSocket::regCallback(std::function<void(std::string&, long)> func)"));
	fcall = func;
	cbInit = true;
}

void WebSocket::regCallbackStop(std::function<void()> func)
{
	sysl->TRACE(std::string("WebSocket::regCallbackStop(std::function<void()> func)"));
	fcallStop = func;
	cbInitStop = true;
}

void WebSocket::regCallbackConnected(std::function<void (bool, long)> func)
{
	sysl->TRACE(std::string("WebSocket::regCallbackConnected(std::function<void (bool, long)> func)"));
	fcallConn = func;
	cbInitCon = true;
}

void WebSocket::regCallbackRegister(std::function<void (long, int)> func)
{
	sysl->TRACE(std::string("WebSocket::regCallbackRegister(std::function<void (long, int)> func)"));
	fcallRegister = func;
	cbInitRegister = true;
}

void WebSocket::setConStatus(bool s, long pan)
{
	sysl->TRACE(std::string("WebSocket::setConStatus(bool s, long pan) [")+((s)?"TRUE":"FALSE")+"]");

	if (cbInitCon)
		fcallConn(s, pan);
	else
		sysl->warnlog(std::string("WebSocket::setConStatus: Callback function to indicate connection status was not set!"));
}

void WebSocket::run()
{
	sysl->TRACE(std::string("WebSocket::run()"));
	bool stopped = false;
	// Create a server endpoint

	try
	{
		if (Configuration->getWSStatus())
		{
			sysl->TRACE(std::string("WebSocket::run: Using encrypted communication."));
			// Set logging settings
			sock_server.set_access_channels(websocketpp::log::alevel::all);
			sock_server.clear_access_channels(websocketpp::log::alevel::frame_payload);
			// Initialize ASIO
			sock_server.init_asio();
			sock_server.set_reuse_addr(true);
			// Register our message handler
			sock_server.set_message_handler(bind(&WebSocket::on_message, this, &sock_server,::_1,::_2));
			sock_server.set_http_handler(bind(&WebSocket::on_http, this, &sock_server, ::_1));
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
			sysl->TRACE(std::string("WebSocket::run: Using plain communication!"));
			// Set logging settings
			sock_server_ws.set_access_channels(websocketpp::log::alevel::all);
			sock_server_ws.clear_access_channels(websocketpp::log::alevel::frame_payload);
			// Initialize ASIO
			sock_server_ws.init_asio();
			sock_server_ws.set_reuse_addr(true);
			// Register our message handler
			sock_server_ws.set_message_handler(bind(&WebSocket::on_message_ws, this, &sock_server_ws, ::_1, ::_2));
			sock_server_ws.set_http_handler(bind(&WebSocket::on_http_ws, this, &sock_server_ws, ::_1));
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
		sysl->errlog(std::string("WebSocket::run: WEBSocketPP exception:")+e.what());
		setConStatus(false, getPanelID(server_hdl));
		stopped = true;

		if (cbInitStop)
			fcallStop();
	}
	catch (const std::exception & e)
	{
		sysl->errlog(std::string("WebSocket::run: ")+e.what());
		setConStatus(false, getPanelID(server_hdl));
		stopped = true;

		if (cbInitStop)
			fcallStop();
	}
	catch (...)
	{
		sysl->errlog(std::string("WebSocket::run: Other exception!"));
		setConStatus(false, getPanelID(server_hdl));
		stopped = true;

		if (cbInitStop)
			fcallStop();
	}

	if (!stopped)
	{
		setConStatus(false, getPanelID(server_hdl));

		if (cbInitStop)
			fcallStop();
	}
}

WebSocket::~WebSocket()
{
	websocketpp::lib::error_code ec;

	if (Configuration->getWSStatus())
		getServer().stop_listening(ec);
	else
		getServer_ws().stop_listening(ec);

	if (ec)
	{
		sysl->errlog(std::string("WebSocket::~WebSocket: Error stopping listening: ")+ec.message());
		sysl->TRACE(Syslog::EXIT, std::string("WebSocket::~WebSocket()"));
		return;
	}

	if (Configuration->getWSStatus())
		getServer().stop();
	else
		getServer_ws().stop();

	sysl->TRACE(Syslog::EXIT, std::string("WebSocket::~WebSocket()"));
}

bool WebSocket::send(strings::String& msg, long pan)
{
	sysl->TRACE(std::string("WebSocket::send(strings::String& msg, long pan)"));

	REG_DATA_T::iterator itr;
	bool found = false;

	for (itr = __regs.begin(); itr != __regs.end(); ++itr)
	{
		if (itr->second.ID == pan)
		{
			server_hdl = itr->first;
			found = true;
			break;
		}
	}

	if (!found)
	{
		sysl->errlog(std::string("WebSocket::send: Unknown websocket handle!"));
		return false;
	}
	else
		sysl->DebugMsg(strings::String("WebSocket::send: Found id=%1 from pan %2").arg(itr->second.channel).arg(itr->second.ID));

	try
	{
		if (Configuration->getWSStatus())
			sock_server.send(itr->first, msg.toString(), websocketpp::frame::opcode::text);
		else
			sock_server_ws.send(itr->first, msg.toString(), websocketpp::frame::opcode::text);
	}
	catch (websocketpp::exception const & e)
	{
		sysl->errlog(std::string("WebSocket::send: Error sending a message: ")+e.what());

		if (Configuration->getWSStatus())
			sock_server.close(server_hdl, 0, std::string(e.what()));
		else
			sock_server_ws.close(server_hdl, 0, std::string(e.what()));

		setConStatus(false, pan);
		return false;
	}

	return true;
}

void WebSocket::tcp_post_init(connection_hdl hdl)
{
	sysl->TRACE(std::string("WebSocket::tcp_post_init(websocketpp::connection_hdl hdl)"));
	server_hdl = hdl;

	if (!cbInitRegister)
	{
		sysl->errlog(std::string("WebSocket::tcp_post_init: Callback function for registering web socket connection was not initialized!"));
		return;
	}

	PAN_ID_T pid;
	pid.channel = 0;
	pid.ID = random();
	
	if (Configuration->getWSStatus())
		pid.ip = sock_server.get_con_from_hdl(hdl)->get_remote_endpoint();
	else
		pid.ip = sock_server_ws.get_con_from_hdl(hdl)->get_remote_endpoint();

	__regs.insert(std::pair<websocketpp::connection_hdl, PAN_ID_T>(hdl, pid));
	sysl->DebugMsg(strings::String("WebSocket::tcp_post_init: Registering pan %1 for remote %2").arg(pid.ID).arg(pid.ip));
	fcallRegister(pid.ID, 0);
}

void WebSocket::on_http(server* s, connection_hdl hdl)
{
	sysl->TRACE(std::string("WebSocket::on_http(server* s, websocketpp::connection_hdl hdl)"));
	server_hdl = hdl;
	server::connection_ptr con = s->get_con_from_hdl(hdl);

	std::string res = con->get_request_body();

	std::stringstream ss;
	ss << "got HTTP request with " << res.size() << " bytes of body data.";

	con->set_body(ss.str());
	con->set_status(websocketpp::http::status_code::ok);
}

void WebSocket::on_http_ws(server_ws* s, connection_hdl hdl)
{
	sysl->TRACE(std::string("WebSocket::on_http_ws(server_ws* s, websocketpp::connection_hdl hdl)"));
	server_hdl = hdl;
	server_ws::connection_ptr con = s->get_con_from_hdl(hdl);

	std::string res = con->get_request_body();

	std::stringstream ss;
	ss << "got HTTP request with " << res.size() << " bytes of body data.";

	con->set_body(ss.str());
	con->set_status(websocketpp::http::status_code::ok);
}


// Define a callback to handle incoming messages
void WebSocket::on_message(server* s, connection_hdl hdl, message_ptr msg)
{
	sysl->TRACE(std::string("WebSocket::on_message(server* s, websocketpp::connection_hdl hdl, message_ptr msg)"));

	setConStatus(true, getPanelID(hdl));
	server_hdl = hdl;
	server::connection_ptr con = s->get_con_from_hdl(hdl);
	REG_DATA_T::iterator key;
	
	if ((key = __regs.find(hdl)) != __regs.end())
	{
		key->second.ip = con->get_remote_endpoint();
		sysl->DebugMsg(std::string("WebSocket::on_message: Endpoint: ")+key->second.ip);
	}

	std::string send = msg->get_payload();
	sysl->TRACE(std::string("WebSocket::on_message: Called with hdl: message: ")+send);
	std::vector<strings::String> parts = strings::String(send).split(":");
	int id = 0;
	long pan = 0;

	if (send.find("REGISTER:") == std::string::npos)
		id = atoi(parts[1].data());

	if (!cbInit)
	{
		sysl->warnlog(std::string("WebSocket::on_message: No callback function registered!"));
		return;
	}

	key = __regs.find(hdl);

	if (id >= 10000 && id <= 11000 && key == __regs.end())
	{
		PAN_ID_T reg;
		reg.channel = id;
		reg.ID = random();
		__regs.insert(std::pair<connection_hdl, PAN_ID_T>(hdl, reg));
		sysl->DebugMsg(strings::String("WebSocket::on_message: Registering id %1 for pan %2").arg(id).arg(reg.ID));
		fcallRegister(reg.ID, id);
		pan = reg.ID;
	}
	else if (id >= 10000 && id <= 11000 && key->second.channel == 0)
	{
		PAN_ID_T pid = key->second;
		pan = key->second.ID;
		pid.channel = id;
		replaceHdl(key, pid);
		sysl->DebugMsg(strings::String("WebSocket::on_message: Channel was set to %1 for pan %2").arg(id).arg(pan));
	}
	else
		pan = key->second.ID;

	if (send.find("PANEL:") != std::string::npos)
		return;

	fcall(send, pan);
}

void WebSocket::on_message_ws(server_ws* s, connection_hdl hdl, message_ptr msg)
{
	sysl->TRACE(std::string("WebSocket::on_message_ws(server_ws* s, websocketpp::connection_hdl hdl, message_ptr msg)"));

	setConStatus(true, getPanelID(hdl));
	server_hdl = hdl;
	server_ws::connection_ptr con = s->get_con_from_hdl(hdl);
	REG_DATA_T::iterator key;
	
	if ((key = __regs.find(hdl)) != __regs.end())
	{
		key->second.ip = con->get_remote_endpoint();
		sysl->DebugMsg(std::string("WebSocket::on_message: Endpoint: ")+key->second.ip);
	}
	
	std::string send = msg->get_payload();
	sysl->TRACE(std::string("WebSocket::on_message_ws: Called with hdl: message: ")+send);
	std::vector<strings::String> parts = strings::String(send).split(":");
	int id = 0;
	long pan = 0;

	if (send.find_first_of("REGISTER:") == std::string::npos)
		id = atoi(parts[1].data());

	if (!cbInit)
	{
		sysl->warnlog(std::string("WebSocket::on_message_ws: No callback function registered!"));
		return;
	}

	key = __regs.find(hdl);

	if (id >= 10000 && id <= 11000 && key == __regs.end())
	{
		PAN_ID_T reg;
		reg.channel = id;
		reg.ID = random();
		__regs.insert(std::pair<connection_hdl, PAN_ID_T>(hdl, reg));
		sysl->DebugMsg(strings::String("WebSocket::on_message_ws: Registering id %1 for pan %2").arg(id).arg(reg.ID));
		fcallRegister(reg.ID, id);
		pan = reg.ID;
	}
	else if (id >= 10000 && id <= 11000 && key->second.channel == 0)
	{
		PAN_ID_T pid = key->second;
		pan = key->second.ID;
		pid.channel = id;
		replaceHdl(key, pid);
		sysl->DebugMsg(strings::String("WebSocket::on_message_ws: Channel was set to %1 for pan %2").arg(id).arg(pan));
	}
	else
		pan = key->second.ID;

	if (send.find("PANEL:") != std::string::npos)
		return;

	fcall(send, pan);
}

void WebSocket::on_fail(server* s, connection_hdl hdl)
{
	sysl->TRACE(std::string("WebSocket::on_fail(server* s, websocketpp::connection_hdl hdl)"));
	server_hdl = hdl;
	server::connection_ptr con = s->get_con_from_hdl(hdl);
	sysl->errlog(std::string("WebSocket::on_fail: Fail handler: ")+con->get_ec().message());
	long pan = getPanelID(hdl);
	setConStatus(false, pan);
	fcallRegister(pan, -1);

	REG_DATA_T::iterator key;

	if ((key = __regs.find(hdl)) != __regs.end())
		__regs.erase(key);
}

void WebSocket::on_fail_ws(server_ws* s, connection_hdl hdl)
{
	sysl->TRACE(std::string("WebSocket::on_fail_ws(server_ws* s, websocketpp::connection_hdl hdl)"));
	server_hdl = hdl;
	server_ws::connection_ptr con = s->get_con_from_hdl(hdl);
	sysl->errlog(std::string("WebSocket::on_fail_ws: Fail handler: ")+con->get_ec().message());
	long pan = getPanelID(hdl);
	setConStatus(false, pan);
	fcallRegister(pan, -1);

	REG_DATA_T::iterator key;

	if ((key = __regs.find(hdl)) != __regs.end())
		__regs.erase(key);
}

void WebSocket::on_close(connection_hdl hdl)
{
	sysl->TRACE(std::string("WebSocket::on_close(websocketpp::connection_hdl)"));
	server_hdl = hdl;
	long pan = getPanelID(hdl);
	setConStatus(false, pan);
	fcallRegister(pan, -1);

	REG_DATA_T::iterator key;

	if ((key = __regs.find(hdl)) != __regs.end())
		__regs.erase(key);

	sysl->DebugMsg(std::string("WebSocket::on_close: AMX connection terminated."));
}

context_ptr WebSocket::on_tls_init(tls_mode mode, connection_hdl hdl)
{
	sysl->TRACE(std::string("WebSocket::on_tls_init(tls_mode mode, websocketpp::connection_hdl hdl)"));
	namespace asio = websocketpp::lib::asio;
	server_hdl = hdl;

	sysl->TRACE(std::string("WebSocket::on_tls_init: Using TLS mode: ")+(mode == MOZILLA_MODERN ? "Mozilla Modern" : "Mozilla Intermediate"));

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

		sysl->TRACE(std::string("WebSocket::on_tls_init: Reading certificate chain file: ")+Configuration->getSSHServerFile().toString());
		ctx->use_certificate_chain_file(Configuration->getSSHServerFile().toString());
		sysl->TRACE(std::string("WebSocket::on_tls_init: Reading private key file: ")+Configuration->getSSHServerFile().toString());
		ctx->use_private_key_file(Configuration->getSSHServerFile().toString(), asio::ssl::context::pem);

		// Example method of generating this file:
		// `openssl dhparam -out dh.pem 2048`
		// Mozilla Intermediate suggests 1024 as the minimum size to use
		// Mozilla Modern suggests 2048 as the minimum size to use.
		sysl->TRACE(std::string("WebSocket::on_tls_init: Reading DH parameter file: ")+Configuration->getSSHDHFile().toString());
		ctx->use_tmp_dh_file(Configuration->getSSHDHFile().toString());

		std::string ciphers;

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
			sysl->errlog(std::string("WebSocket::on_tls_init: Error setting cipher list"));
		}
	}
	catch (std::exception& e)
	{
		sysl->errlog(std::string("WebSocket::on_tls_init: Exception: ")+e.what());
	}

	return ctx;
}

std::string WebSocket::getPassword()
{
	sysl->TRACE(std::string("WebSocket::getPassword()"));
	return Configuration->getSSHPassword().toString();
}

bool WebSocket::compareHdl(connection_hdl hdl1, connection_hdl hdl2)
{
	sysl->TRACE(std::string("WebSocket::compareHdl(websocketpp::connection_hdl& hdl1, websocketpp::connection_hdl& hdl2)"));
	REG_DATA_T::iterator key1 = __regs.find(hdl1);
	REG_DATA_T::iterator key2 = __regs.find(hdl2);

	if (key1 != __regs.end() && key2 != __regs.end() && key1 == key2)
	{
		sysl->DebugMsg(std::string("WebSocket::compareHdl: Found matching handles!"));
		return true;
	}

	sysl->DebugMsg(std::string("WebSocket::compareHdl: Handles don't match!"));
	return false;
}

long WebSocket::getPanelID(websocketpp::connection_hdl hdl)
{
	REG_DATA_T::iterator key;

	if ((key = __regs.find(hdl)) != __regs.end())
		return key->second.ID;

	return 0;
}

bool WebSocket::replaceHdl(REG_DATA_T::iterator key, PAN_ID_T& pan)
{
	sysl->DebugMsg(std::string("WebSocket::replaceHdl(REG_DATA_T::iterator key, PAN_ID_T& pan)"));

	if (__regs.size() == 0)
		return false;

//	websocketpp::connection_hdl hdl = key->first;
//	__regs.erase(key);
//	__regs.insert(std::pair<websocketpp::connection_hdl, PAN_ID_T>(hdl, pan));
	key->second = pan;
	return true;
}
