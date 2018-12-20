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

#include "asio/buffer.hpp"
#include "asio/io_context.hpp"
#include "asio/ip/tcp.hpp"
#include "asio/read_until.hpp"
#include "asio/steady_timer.hpp"
#include "asio/write.hpp"
#include <functional>
#include <iostream>
#include <string>
#include "syslog.h"
#include "strings.h"
#include "amxnet.h"

#ifdef __APPLE__
using namespace boost;
#endif

extern Syslog *sysl;

using asio::steady_timer;
using asio::ip::tcp;
using std::placeholders::_1;
using std::placeholders::_2;
using namespace amx;

AMXNet::AMXNet(asio::io_context& io_context)
				: socket_(io_context),
				  deadline_(io_context),
				  heartbeat_timer_(io_context)
{
	sysl->TRACE(Syslog::ENTRY, std::string("AMXNet::AMXNet(asio::io_context& io_context)"));
}

AMXNet::~AMXNet()
{
	sysl->TRACE(Syslog::EXIT, std::string("AMXNet::AMXNet(...)"));
	callback = 0;
}

void AMXNet::start(asio::ip::tcp::resolver::results_type endpoints)
{
	endpoints_ = endpoints;
	start_connect(endpoints_.begin());	
	deadline_.async_wait(std::bind(&AMXNet::check_deadline, this));
}

void AMXNet::stop()
{
	sysl->TRACE(std::string("AMXNet::stop: Stopping the client..."));
	stopped_ = true;
	std::error_code ignored_error;
	socket_.close(ignored_error);
	deadline_.cancel();
	heartbeat_timer_.cancel();
}

void AMXNet::start_connect(asio::ip::tcp::resolver::results_type::iterator endpoint_iter)
{
	if (endpoint_iter != endpoints_.end())
	{
		sysl->TRACE(std::string("AMXNet::start_connect: Trying ")+endpoint_iter->endpoint().address().to_string()+" ...\n");

		// Set a deadline for the connect operation.
		deadline_.expires_after(std::chrono::seconds(60));

		// Start the asynchronous connect operation.
		socket_.async_connect(endpoint_iter->endpoint(),
			std::bind(&AMXNet::handle_connect,
			this, _1, endpoint_iter));
	}
	else
	{
		// There are no more endpoints to try. Shut down the client.
		stop();
	}
}

void AMXNet::handle_connect(const std::error_code& error, asio::ip::tcp::resolver::results_type::iterator endpoint_iter)
{
	if (stopped_)
		return;

	// The async_connect() function automatically opens the socket at the start
	// of the asynchronous operation. If the socket is closed at this time then
	// the timeout handler must have run first.
	if (!socket_.is_open())
	{
		sysl->TRACE(std::string("Connect timed out"));

		// Try the next available endpoint.
		start_connect(++endpoint_iter);
    }

	// Check if the connect operation failed before the deadline expired.
	else if (error)
	{
		sysl->errlog("AMXNet::handle_connect: Connect error: "+error.message());

		// We need to close the socket used in the previous connection attempt
		// before starting a new one.
		socket_.close();

		// Try the next available endpoint.
		start_connect(++endpoint_iter);
	}
	else
	{
		sysl->log(Syslog::INFO, "AMXNet::handle_connect: Connected to "+endpoint_iter->endpoint().address().to_string());

		// Start the input actor.
		start_read();

		// Start the heartbeat actor.
		start_write();
	}
}

void AMXNet::start_read()
{
	// Set a deadline for the read operation.
	deadline_.expires_after(std::chrono::seconds(30));

	// Start an asynchronous operation to read a newline-delimited message.
	asio::async_read_until(socket_,
				asio::dynamic_buffer(input_buffer_), '\n',
				std::bind(&AMXNet::handle_read, this, _1, _2));
}

void AMXNet::handle_read(const std::error_code& error, std::size_t n)
{
	if (stopped_)
		return;

	if (!error)
	{
		// Extract the newline-delimited message from the buffer.
		std::string line(input_buffer_.substr(0, n - 1));
		input_buffer_.erase(0, n);

		// Empty messages are heartbeats and so ignored.
		if (!line.empty())
		{
			sysl->TRACE("AMXNet::handle_read: Received: "+line);
			// FIXME! Send the received text to the interpreter
			if (callback != 0)
				callback(strings::String(line));
		}

		start_read();
	}
	else
	{
		sysl->errlog("AMXNet::handle_read: Error on receive: "+error.message());
		stop();
	}
}

void AMXNet::start_write()
{
	if (stopped_)
		return;

	// Start an asynchronous operation to send a heartbeat message.
	asio::async_write(socket_, asio::buffer("\n", 1),
			std::bind(&AMXNet::handle_write, this, _1));
}

void AMXNet::handle_write(const std::error_code& error)
{
	if (stopped_)
		return;

	if (!error)
	{
		// Wait 10 seconds before sending the next heartbeat.
		heartbeat_timer_.expires_after(std::chrono::seconds(10));
		heartbeat_timer_.async_wait(std::bind(&AMXNet::start_write, this));
	}
	else
	{
		sysl->errlog("AMXNet::handle_write: Error on heartbeat: "+error.message());
		stop();	
	}
}

void AMXNet::check_deadline()
{
	if (stopped_)
		return;

	// Check whether the deadline has passed. We compare the deadline against
	// the current time since a new asynchronous operation may have moved the
	// deadline before this actor had a chance to run.
	if (deadline_.expiry() <= steady_timer::clock_type::now())
	{
		// The deadline has passed. The socket is closed so that any outstanding
		// asynchronous operations are cancelled.
		socket_.close();

		// There is no longer an active deadline. The expiry is set to the
		// maximum time point so that the actor takes no action until a new
		// deadline is set.
		deadline_.expires_at(steady_timer::time_point::max());
	}

	// Put the actor back to sleep.
	deadline_.async_wait(std::bind(&AMXNet::check_deadline, this));
}

uint16_t AMXNet::swapWord(uint16_t w)
{
    uint16_t word = 0;
    word = ((w << 8) & 0xff00) & ((w >> 8) & 0x00ff);
    return word;
}

uint32_t AMXNet::swapDWord(uint32_t dw)
{
    uint32_t dword = 0;
    dword = ((dw << 24) & 0xff000000) & ((dw << 8) & 0x00ff0000) & ((dw >> 8) & 0x0000ff00) & ((dw >> 24) & 0x000000ff);
    return dword;
}
