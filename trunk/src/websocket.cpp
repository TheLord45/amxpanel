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

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
using namespace amx;

WebSocket::WebSocket()
{
	sysl->TRACE(Syslog::ENTRY, std::string("WebSocket::WebSocket()"));
	setConStatus(false);
}

void WebSocket::run()
{
	sysl->TRACE(std::string("WebSocket::run()"));
	// Create a server endpoint

	try
	{
		// Set logging settings
		sock_server.set_access_channels(websocketpp::log::alevel::all);
		sock_server.clear_access_channels(websocketpp::log::alevel::frame_payload);

		// Initialize ASIO
		sock_server.init_asio();
		sock_server.set_reuse_addr(true);

		// Register our message handler
		sock_server.set_message_handler(bind(&on_message,&sock_server,::_1,::_2));
		sock_server.set_http_handler(bind(&on_http,&sock_server,::_1));
		sock_server.set_fail_handler(bind(&on_fail,&sock_server,::_1));
		sock_server.set_close_handler(&on_close);
		sock_server.set_tls_init_handler(bind(&on_tls_init,MOZILLA_MODERN,::_1));

		// Listen on port 11012
		sock_server.listen(11012);

		// Start the server accept loop
		sock_server.start_accept();

		// Start the ASIO io_service run loop
		setConStatus(true);
		sock_server.run();
	}
	catch (websocketpp::exception const & e)
	{
		sysl->errlog(std::string("WebSocket::run: ")+e.what());
		setConStatus(false);

		if (cbInitStop)
			fcallStop();
	}
	catch (const std::exception & e)
	{
		sysl->errlog(std::string("WebSocket::run: ")+e.what());
		setConStatus(false);

		if (cbInitStop)
			fcallStop();
	}
	catch (...)
	{
		sysl->errlog(std::string("WebSocket::run: Other exception!"));
		setConStatus(false);

		if (cbInitStop)
			fcallStop();
	}

	setConStatus(false);

	if (cbInitStop)
		fcallStop();
}

WebSocket::~WebSocket()
{
	sysl->TRACE(Syslog::EXIT, std::string("WebSocket::WebSocket()"));

	websocketpp::lib::error_code ec;
	server& echo_server = getServer();
	echo_server.stop_listening(ec);
	
	if (ec)
	{
		sysl->errlog(std::string("WebSocket::~WebSocket: Error stopping listening: ")+ec.message());
		return;
	}

	echo_server.stop();
}

bool WebSocket::send(strings::String& msg)
{
	sysl->TRACE(std::string("WebSocket::send(strings::String& msg)"));

	if (!getConStatus())
		return false;

	server& echo_server = getServer();
	server::connection_ptr pcon = echo_server.get_connection();
	websocketpp::lib::error_code ec;
	echo_server.send(pcon, msg.toString(), websocketpp::frame::opcode::text, ec);

	if (ec)
	{
		sysl->errlog(std::string("WebSocket::send: Error sending a message: ")+ec.message());
		return false;
	}

	return true;
}

void WebSocket::on_http(server* s, websocketpp::connection_hdl hdl)
{
	sysl->TRACE(std::string("WebSocket::on_http(server* s, websocketpp::connection_hdl hdl)"));
	server::connection_ptr con = s->get_con_from_hdl(hdl);

	std::string res = con->get_request_body();

	std::stringstream ss;
	ss << "got HTTP request with " << res.size() << " bytes of body data.";

	con->set_body(ss.str());
	con->set_status(websocketpp::http::status_code::ok);
}


// Define a callback to handle incoming messages
void WebSocket::on_message(server* s, websocketpp::connection_hdl hdl, message_ptr msg)
{
	sysl->TRACE(std::string("WebSocket::on_message(server* s, websocketpp::connection_hdl hdl, message_ptr msg)"));

	std::string send = msg->get_payload();
	sysl->TRACE(std::string("WebSocket::on_message: Called with hdl: message: ")+send);

	if (!cbInit)
	{
		sysl->warnlog(std::string("WebSocket::on_message: No callback function registered!"));
		return;
	}

	fcall(send);
}

void WebSocket::on_fail(server* s, websocketpp::connection_hdl hdl)
{
	sysl->TRACE(std::string("WebSocket::on_fail(server* s, websocketpp::connection_hdl hdl)"));
	server::connection_ptr con = s->get_con_from_hdl(hdl);
	sysl->errlog(std::string("WebSocket::on_fail: Fail handler: ")+con->get_ec().message());
	setConStatus(false);
}

void WebSocket::on_close(websocketpp::connection_hdl)
{
	sysl->TRACE(std::string("WebSocket::on_close(websocketpp::connection_hdl)"));
	setConStatus(false);
}

context_ptr WebSocket::on_tls_init(tls_mode mode, websocketpp::connection_hdl)
{
	sysl->TRACE(std::string("WebSocket::on_tls_init(tls_mode mode, websocketpp::connection_hdl hdl)"));
	namespace asio = websocketpp::lib::asio;

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
			ctx->set_password_callback(bind(&getPassword));

		ctx->use_certificate_chain_file(Configuration->getSSHServerFile().toString());
		ctx->use_private_key_file(Configuration->getSSHServerFile().toString(), asio::ssl::context::pem);

		// Example method of generating this file:
		// `openssl dhparam -out dh.pem 2048`
		// Mozilla Intermediate suggests 1024 as the minimum size to use
		// Mozilla Modern suggests 2048 as the minimum size to use.
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