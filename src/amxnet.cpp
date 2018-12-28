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

#ifdef __APPLE__
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/write.hpp>
#else
#include <asio/buffer.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/read_until.hpp>
#include <asio/steady_timer.hpp>
#include <asio/write.hpp>
#endif
#include <functional>
#include <iostream>
#include <cstring>
#include <string>
#include "syslog.h"
#include "strings.h"
#include "config.h"
#include "amxnet.h"

#ifdef __APPLE__
using namespace boost;
#endif

extern Syslog *sysl;
extern Config *Configuration;

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
	readToken = RT_NONE;
	sendCounter = 0;
	write_busy = false;
	serial = "201812XTHE74201";
	version = "v1.00.00";
	manufacturer = "TheoSys";
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
#ifdef __APPLE__
	system::error_code ignored_error;
#else
	std::error_code ignored_error;
#endif
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
	deadline_.expires_after(std::chrono::seconds(60));
	protError = false;
	comm.clear();

	// Start an asynchronous operation to read a newline-delimited message.
	// Read the first byte. It should be 0x02
	readToken = RT_ID;
	asio::async_read(socket_, asio::dynamic_buffer(input_buffer_, 1), std::bind(&AMXNet::handle_read, this, _1, _2));
	readToken = RT_LEN;
	asio::async_read(socket_, asio::dynamic_buffer(input_buffer_, 2), std::bind(&AMXNet::handle_read, this, _1, _2));
	readToken = RT_SEP1;
	asio::async_read(socket_, asio::dynamic_buffer(input_buffer_, 1), std::bind(&AMXNet::handle_read, this, _1, _2));
	readToken = RT_TYPE;
	asio::async_read(socket_, asio::dynamic_buffer(input_buffer_, 1), std::bind(&AMXNet::handle_read, this, _1, _2));
	readToken = RT_WORD1;
	asio::async_read(socket_, asio::dynamic_buffer(input_buffer_, 2), std::bind(&AMXNet::handle_read, this, _1, _2));
	readToken = RT_DEVICE;
	asio::async_read(socket_, asio::dynamic_buffer(input_buffer_, 2), std::bind(&AMXNet::handle_read, this, _1, _2));
	readToken = RT_WORD2;
	asio::async_read(socket_, asio::dynamic_buffer(input_buffer_, 2), std::bind(&AMXNet::handle_read, this, _1, _2));
	readToken = RT_WORD3;
	asio::async_read(socket_, asio::dynamic_buffer(input_buffer_, 2), std::bind(&AMXNet::handle_read, this, _1, _2));
	readToken = RT_WORD4;
	asio::async_read(socket_, asio::dynamic_buffer(input_buffer_, 2), std::bind(&AMXNet::handle_read, this, _1, _2));
	readToken = RT_WORD5;
	asio::async_read(socket_, asio::dynamic_buffer(input_buffer_, 2), std::bind(&AMXNet::handle_read, this, _1, _2));
	readToken = RT_SEP2;
	asio::async_read(socket_, asio::dynamic_buffer(input_buffer_, 1), std::bind(&AMXNet::handle_read, this, _1, _2));
	readToken = RT_COUNT;
	asio::async_read(socket_, asio::dynamic_buffer(input_buffer_, 2), std::bind(&AMXNet::handle_read, this, _1, _2));
	readToken = RT_MC;
	asio::async_read(socket_, asio::dynamic_buffer(input_buffer_, 2), std::bind(&AMXNet::handle_read, this, _1, _2));
	readToken = RT_DATA;
	// Calculate the length of the data block. This is the rest of the total length
	size_t len = (comm.hlen + 3) - 0x0015;
	asio::async_read(socket_, asio::dynamic_buffer(input_buffer_, len), std::bind(&AMXNet::handle_read, this, _1, _2));
//	asio::async_read_until(socket_,
//				asio::dynamic_buffer(input_buffer_), '\n',
//				std::bind(&AMXNet::handle_read, this, _1, _2));
}

void AMXNet::handle_read(const std::error_code& error, std::size_t n)
{
	if (stopped_)
		return;

	uint32_t dw;
	int val, pos;
	size_t len;
	bool ignore = false;

	if (!error)
	{
		// Extract the newline-delimited message from the buffer.
		std::string line(input_buffer_.substr(0, n));
		input_buffer_.erase(0, n);

		switch (readToken)
		{
			case RT_ID:
				if (line[0] != 0x02)
					protError = true;
				else
					comm.ID = line[0];
			break;

			case RT_LEN: 	comm.hlen = makeWord(line[0], line[1]); break;

			case RT_SEP1:
				if (line[0] != 0x02)
					protError = true;
				else
					comm.sep1 = line[0];
			break;

			case RT_TYPE: 	comm.type = line[0]; break;
			case RT_WORD1:	comm.unk1 = makeWord(line[0], line[1]); break;
			case RT_DEVICE:	comm.device1 = makeWord(line[0], line[1]); break;
			case RT_WORD2:	comm.port1 = makeWord(line[0], line[1]); break;
			case RT_WORD3:	comm.system = makeWord(line[0], line[1]); break;
			case RT_WORD4:	comm.device2 = makeWord(line[0], line[1]); break;
			case RT_WORD5:	comm.port2 = makeWord(line[0], line[1]); break;

			case RT_SEP2:
				if (line[0] != 0x0f)
					protError = true;
				else
					comm.unk6 = line[0];
			break;

			case RT_COUNT:	comm.count = makeWord(line[0], line[1]); break;
			case RT_MC:		comm.MC = makeWord(line[0], line[1]); break;

			case RT_DATA:
				if (protError)
					break;

				switch (comm.MC)
				{
					case 0x0001:	// ACK
					case 0x0002:	// NAK
						comm.checksum = line[0];
					break;

					case 0x0084:	// input channel ON
					case 0x0085:	// input channel OFF
					case 0x0006:	// output channel ON
					case 0x0086:	// output channel ON status
					case 0x0007:	// output channel OFF
					case 0x0087:	// output channel OFF status
					case 0x0088:	// input/output channel ON status
					case 0x0089:	// input/output channel OFF status
					case 0x0018:	// feedback channel ON
					case 0x0019:	// feedback channel OFF
						comm.data.chan_state.device = makeWord(line[0], line[1]);
						comm.data.chan_state.port = makeWord(line[2], line[3]);
						comm.data.chan_state.system = makeWord(line[4], line[5]);
						comm.data.chan_state.channel = makeWord(line[6], line[7]);
						comm.checksum = line[8];
					break;

					case 0x000a:	// level value change
					case 0x008a:
						comm.data.message_value.device = makeWord(line[0], line[1]);
						comm.data.message_value.port = makeWord(line[2], line[3]);
						comm.data.message_value.system = makeWord(line[4], line[5]);
						comm.data.message_value.value = makeWord(line[6], line[7]);
						comm.data.message_value.type = line[8];
						val = (int)line[8];

						switch (val)
						{
							case 0x010: comm.data.message_value.content.byte = line[9]; comm.checksum = line[10]; break;
							case 0x011: comm.data.message_value.content.ch = line[9]; comm.checksum = line[10]; break;
							case 0x020: comm.data.message_value.content.integer = makeWord(line[9], line[10]); comm.checksum = line[11]; break;
							case 0x021: comm.data.message_value.content.sinteger = makeWord(line[9], line[10]); comm.checksum = line[11]; break;
							case 0x040: comm.data.message_value.content.dword = makeDWord(line[9], line[10], line[11], line[12]); comm.checksum = line[13]; break;
							case 0x041: comm.data.message_value.content.sdword = makeDWord(line[9], line[10], line[11], line[12]); comm.checksum = line[13]; break;

							case 0x04f:
								dw = makeDWord(line[9], line[10], line[11], line[12]);
								memcpy(&comm.data.message_value.content.fvalue, &dw, 4);
								comm.checksum = line[13];
							break;

							case 0x08f: memcpy(&comm.data.message_value.content.dvalue, &line[9], 8); comm.checksum = line[17]; break;	// FIXME: wrong byte order on Intel CPU
						}
					break;

					case 0x000b:	// string value change
					case 0x008b:
					case 0x000c:	// command string
					case 0x008c:
						comm.data.message_string.device = makeWord(line[0], line[1]);
						comm.data.message_string.port = makeWord(line[2], line[3]);
						comm.data.message_string.system = makeWord(line[4], line[5]);
						comm.data.message_string.type = line[6];
						comm.data.message_string.length = makeWord(line[7], line[8]);
						len = (line[6] == 0x01) ? comm.data.message_string.length : comm.data.message_string.length * 2;
						memcpy(&comm.data.message_string.content[0], &line[9], len);
						pos = len + 10;
						comm.checksum = line[pos];
					break;

					case 0x000e:	// request level value
						comm.data.level.device = makeWord(line[0], line[1]);
						comm.data.level.port = makeWord(line[2], line[3]);
						comm.data.level.system = makeWord(line[4], line[5]);
						comm.data.level.level = makeWord(line[6], line[7]);
						comm.checksum = line[8];
					break;

					case 0x000f:	// request output channel status
						comm.data.channel.device = makeWord(line[0], line[1]);
						comm.data.channel.port = makeWord(line[2], line[3]);
						comm.data.channel.system = makeWord(line[4], line[5]);
						comm.data.channel.channel = makeWord(line[6], line[7]);
						comm.checksum = line[8];
					break;

					case 0x0010:	// request port count
					case 0x0017:	// request device info
						comm.data.reqPortCount.device = makeWord(line[0], line[1]);
						comm.data.reqPortCount.system = makeWord(line[2], line[3]);
						comm.checksum = line[4];
					break;

					case 0x0011:	// request output channel count
					case 0x0012:	// request level count
					case 0x0013:	// request string size
					case 0x0014:	// request command size
					case 0x0016:	// request status code
						comm.data.reqOutpChannels.device = makeWord(line[0], line[1]);
						comm.data.reqOutpChannels.port = makeWord(line[2], line[3]);
						comm.data.reqOutpChannels.system = makeWord(line[4], line[5]);
						comm.checksum = line[6];
					break;

					case 0x0015:	// request level size
						comm.data.reqLevels.device = makeWord(line[0], line[1]);
						comm.data.reqLevels.port = makeWord(line[2], line[3]);
						comm.data.reqLevels.system = makeWord(line[4], line[5]);
						comm.data.reqLevels.level = makeWord(line[6], line[7]);
						comm.checksum = line[8];
					break;

					case 0x0097:	// receive device info
						comm.data.srDeviceInfo.device = makeWord(line[0], line[1]);
						comm.data.srDeviceInfo.system = makeWord(line[2], line[3]);
						comm.data.srDeviceInfo.flag = makeWord(line[4], line[5]);
						comm.data.srDeviceInfo.objectID = line[6];
						comm.data.srDeviceInfo.parentID = line[7];
						comm.data.srDeviceInfo.herstID = makeWord(line[8], line[9]);
						comm.data.srDeviceInfo.deviceID = makeWord(line[10], line[11]);
						memcpy(comm.data.srDeviceInfo.serial, &line[12], 16);
						comm.data.srDeviceInfo.fwid = makeWord(line[28], line[29]);
						memcpy(comm.data.srDeviceInfo.info, &line[30], comm.hlen - 0x0015 - 29);
						comm.checksum = line[comm.hlen + 3];
					break;

					case 0x00a1:	// request status
						reqDevStatus = makeWord(line[0], line[1]);
						comm.checksum = line[2];
					break;
				}
			break;

			default:		// should never happen and is only to satisfy the compiler
				ignore = true;
		}

		sysl->TRACE(strings::String("AMXNet::handle_read: Received: ")+comm.MC);

		if (!ignore && !stopped_ && callback != 0)
			callback(comm);

		start_read();
	}
	else
	{
		sysl->errlog("AMXNet::handle_read: Error on receive: "+error.message());
		stop();
	}
}

bool AMXNet::sendCommand (const ANET_SEND& s)
{
	int pos;
	bool status = false;
	ANET_COMMAND com;
	com.clear();
	com.MC = s.MC;
	com.device1 = 0;
	com.device2 = Configuration->getAMXChannel();
	com.port1 = 1;
	com.system = Configuration->getAMXSystem();
	com.port2 = s.port;
	sendCounter++;
	com.count = sendCounter;

	switch(s.MC)
	{
		case 0x0084:		// push button
			com.data.channel.device = com.device2;
			com.data.channel.port = s.port;
			com.data.channel.system = com.system;
			com.data.channel.channel = s.channel;
			com.hlen = 0x0016 - 0x0003 + sizeof(ANET_CHANNEL);
			comStack.push_back(com);
			status = true;
		break;

		case 0x0085:		// release button
			com.data.channel.device = com.device2;
			com.data.channel.port = s.port;
			com.data.channel.system = com.system;
			com.data.channel.channel = s.channel;
			com.hlen = 0x0016 - 0x0003 + sizeof(ANET_CHANNEL);
			comStack.push_back(com);
			status = true;
		break;

		case 0x008a:		// level value changed
		case 0x0095:		// level size
			com.data.message_value.device = com.device2;
			com.data.message_value.port = s.port;
			com.data.message_value.system = com.system;
			com.data.message_value.value = s.level;
			com.data.message_value.type = 0x20;		// unsigned iteger
			com.data.message_value.content.integer = s.value;
			com.hlen = 0x0016 - 0x0003 + 11;
			comStack.push_back(com);
			status = true;
		break;

		case 0x008b:		// string command
		case 0x008c:		// send command string
			com.data.message_string.device = com.device2;
			com.data.message_string.port = s.port;
			com.data.message_string.system = com.system;
			com.data.message_string.type = 0x01;	// char string
			int len;

			if (s.msg.length() >= sizeof(com.data.message_string.content))
				len = sizeof(com.data.message_string.content) - 1;
			else
				len = s.msg.length();

			strncpy((char *)&com.data.message_string.content[0], s.msg.data(), len);
			com.hlen = 0x0016 - 3 + 9 + len;
			comStack.push_back(com);
			status = true;
		break;

		case 0x0090:		// port count
			com.data.sendPortNumber.device = com.device2;
			com.data.sendPortNumber.system = com.system;
			com.data.sendPortNumber.pcount = s.value;
			com.hlen = 0x0016 - 3 + sizeof(ANET_APCOUNT);
			comStack.push_back(com);
			status = true;
		break;

		case 0x0091:		// output channel count
		case 0x0092:		// send level count
			com.data.sendOutpChannels.device = com.device2;
			com.data.sendOutpChannels.port = s.port;
			com.data.sendOutpChannels.system = com.system;
			com.data.sendOutpChannels.count = s.value;
			com.hlen = 0x0016 - 3 + sizeof(ANET_AOUTCHAN);
			comStack.push_back(com);
			status = true;
		break;

		case 0x0093:		// string size
		case 0x0094:		// command size
			com.data.sendSize.device = com.device2;
			com.data.sendSize.port = s.port;
			com.data.sendSize.system = com.system;
			com.data.sendSize.type = 0x01;
			com.data.sendSize.length = s.value;
			com.hlen = 0x0016 - 3 + sizeof(ANET_ASIZE);
			comStack.push_back(com);
			status = true;
		break;

		case 0x0097:		// device info
			com.data.srDeviceInfo.device = com.device2;
			com.data.srDeviceInfo.system = com.system;
			com.data.srDeviceInfo.flag = 0x0000;
			com.data.srDeviceInfo.objectID = 0;
			com.data.srDeviceInfo.parentID = 0;
			com.data.srDeviceInfo.herstID = 1;
			com.data.srDeviceInfo.deviceID = 0x0123;
			memcpy(com.data.srDeviceInfo.serial, serial.data(), 16);
			com.data.srDeviceInfo.fwid = 0x0135;
			unsigned char buf[128];
			memset(buf, 0, sizeof(buf));
			memcpy(buf, version.data(), version.length());
			pos = version.length() + 1;
			memcpy(buf+pos, Configuration->getAMXPanelType().data(), Configuration->getAMXPanelType().length());
			pos += Configuration->getAMXPanelType().length() + 1;
			memcpy(buf+pos, manufacturer.data(), manufacturer.length());
			pos += manufacturer.length()+1;
			*(buf+pos) = 0x02;	// type IP address
			pos++;
			*(buf+pos) = 0x04;	// field length: 4 bytes
			// Now the IP Address
			{
				strings::String addr = socket_.local_endpoint().address().to_string();
				std::vector<strings::String> parts = addr.split('.');

				for (size_t i = 0; i < parts.size(); i++)
				{
					pos++;
					*(buf+pos) = (unsigned char)atoi(parts[i].data());
				}
			}

			pos++;
			com.hlen = 0x0016 - 3 + 31 + pos;
			comStack.push_back(com);
			status = true;
		break;

		case 0x0098:
			com.data.reqPortCount.device = com.device2;
			com.data.reqPortCount.system = com.system;
			com.hlen = 0x0016 - 3 + 4;
			comStack.push_back(com);
			status = true;
		break;
	}

	return status;
}

void AMXNet::start_write()
{
	if (stopped_)
		return;

	if (write_busy)
		return;

	write_busy = true;

	while (comStack.size() > 0)
	{
		send = comStack.at(0);
		comStack.erase(comStack.begin());	// delete oldest element
		unsigned char *buf = makeBuffer(send);

		if (buf == 0)
		{
			sysl->errlog(strings::String("AMXNet::start_write: Error creating a buffer! Magic number: ")+send.MC);
			continue;
		}

		asio::async_write(socket_, asio::buffer(buf, send.hlen + 3), std::bind(&AMXNet::handle_write, this, _1));
		delete[] buf;
	}

	write_busy = false;
	// Start an asynchronous operation to send a heartbeat message.
//	asio::async_write(socket_, asio::buffer("\n", 1),
//			std::bind(&AMXNet::handle_write, this, _1));
}

void AMXNet::handle_write(const std::error_code& error)
{
	if (stopped_)
		return;

	if (!error)
	{
		while (comStack.size() == 0)
			heartbeat_timer_.expires_after(std::chrono::microseconds(150));

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
	word = ((w << 8) & 0xff00) | ((w >> 8) & 0x00ff);
	return word;
}

uint32_t AMXNet::swapDWord(uint32_t dw)
{
	uint32_t dword = 0;
	dword = ((dw << 24) & 0xff000000) | ((dw << 8) & 0x00ff0000) | ((dw >> 8) & 0x0000ff00) | ((dw >> 24) & 0x000000ff);
	return dword;
}

unsigned char AMXNet::calcChecksum(const unsigned char* buffer, size_t len)
{
	unsigned long sum;

	for (size_t i = 0; i < len; i++)
		sum += (unsigned long)*(buffer+i);

	sum &= 0x000000ff;
	return (unsigned char)sum;
}

uint16_t AMXNet::makeWord ( unsigned char b1, unsigned char b2 )
{
	return ((b1 << 8) & 0xff00) | b2;
}

uint32_t AMXNet::makeDWord ( unsigned char b1, unsigned char b2, unsigned char b3, unsigned char b4 )
{
	return ((b1 << 24) & 0xff000000) | ((b2 << 16) & 0x00ff0000) | ((b3  << 8) & 0x0000ff00) | b4;
}

unsigned char *AMXNet::makeBuffer (const ANET_COMMAND& s)
{
	int pos = 0;
	bool valid = false;
	unsigned char *buf = new unsigned char[s.hlen+4];
	*buf = s.ID;
	*(buf+1) = s.hlen >> 8;
	*(buf+2) = s.hlen;
	*(buf+3) = s.sep1;
	*(buf+4) = s.type;
	*(buf+5) = s.unk1 >> 8;
	*(buf+6) = s.unk1;
	*(buf+7) = s.device1 >> 8;
	*(buf+8) = s.device1;
	*(buf+9) = s.port1 >> 8;
	*(buf+10) = s.port1;
	*(buf+11) = s.system >> 8;
	*(buf+12) = s.system;
	*(buf+13) = s.device2 >> 8;
	*(buf+14) = s.device2;
	*(buf+15) = s.port2 >> 8;
	*(buf+16) = s.port2;
	*(buf+17) = s.unk6;
	*(buf+18) = s.count >> 8;
	*(buf+19) = s.count;
	*(buf+20) = s.MC >> 8;
	*(buf+21) = s.MC;

	// Here the fixed block is complete. The data are following.
	switch(s.MC)
	{
		case 0x0006:
		case 0x0007:
		case 0x0018:
		case 0x0019:
		case 0x0084:
		case 0x0085:
		case 0x0086:
		case 0x0087:
		case 0x0088:
		case 0x0089:
			*(buf+22) = s.data.chan_state.device >> 8;
			*(buf+23) = s.data.chan_state.device;
			*(buf+24) = s.data.chan_state.port >> 8;
			*(buf+25) = s.data.chan_state.port;
			*(buf+26) = s.data.chan_state.system >> 8;
			*(buf+27) = s.data.chan_state.system;
			*(buf+28) = s.data.chan_state.channel >> 8;
			*(buf+29) = s.data.chan_state.channel;
			*(buf+30) = calcChecksum(buf, 30);
			valid = true;
		break;

		case 0x000a:
		case 0x008a:
		case 0x0095:
			*(buf+22) = s.data.message_value.device >> 8;
			*(buf+23) = s.data.message_value.device;
			*(buf+24) = s.data.message_value.port >> 8;
			*(buf+25) = s.data.message_value.port;
			*(buf+26) = s.data.message_value.system >> 8;
			*(buf+27) = s.data.message_value.system;
			*(buf+28) = s.data.message_value.value >> 8;
			*(buf+29) = s.data.message_value.value;
			*(buf+30) = s.data.message_value.type;
			pos = 31;

			switch(s.data.message_value.type)
			{
				case 0x10: *(buf+pos) = s.data.message_value.content.byte; break;
				case 0x11: *(buf+pos) = s.data.message_value.content.ch; break;
				case 0x20:
					*(buf+pos) = s.data.message_value.content.integer >> 8;
					pos++;
					*(buf+pos) = s.data.message_value.content.integer;
				break;
				case 0x21:
					*(buf+pos) = s.data.message_value.content.sinteger >> 8;
					pos++;
					*(buf+pos) = s.data.message_value.content.sinteger;
				break;
				case 0x40:
					*(buf+pos) = s.data.message_value.content.dword >> 24;
					pos++;
					*(buf+pos) = s.data.message_value.content.dword >> 16;
					pos++;
					*(buf+pos) = s.data.message_value.content.dword >> 8;
					pos++;
					*(buf+pos) = s.data.message_value.content.dword;
				break;
				case 0x41:
					*(buf+pos) = s.data.message_value.content.sdword >> 24;
					pos++;
					*(buf+pos) = s.data.message_value.content.sdword >> 16;
					pos++;
					*(buf+pos) = s.data.message_value.content.sdword >> 8;
					pos++;
					*(buf+pos) = s.data.message_value.content.sdword;
				break;
				case 0x4f:
					memcpy(buf+pos, &s.data.message_value.content.fvalue, 4);
					pos += 3;
				break;
				case 0x8f:
					memcpy(buf+pos, &s.data.message_value.content.fvalue, 8);
					pos += 3;
				break;
			}

			pos++;
			*(buf+pos) = calcChecksum(buf, pos);
			valid = true;
		break;

		case 0x000b:
		case 0x000c:
		case 0x008b:
		case 0x008c:
			*(buf+22) = s.data.message_string.device >> 8;
			*(buf+23) = s.data.message_string.device;
			*(buf+24) = s.data.message_string.port >> 8;
			*(buf+25) = s.data.message_string.port;
			*(buf+26) = s.data.message_string.system >> 8;
			*(buf+27) = s.data.message_string.system;
			*(buf+28) = s.data.message_string.type;
			*(buf+29) = s.data.message_string.length >> 8;
			*(buf+30) = s.data.message_string.length;
			pos = 31;
			memcpy(buf+pos, s.data.message_string.content, s.data.message_string.length);
			pos += s.data.message_string.length;
			*(buf+pos) = calcChecksum(buf, pos);
			valid = true;
		break;

		case 0x0090:
			*(buf+22) = s.data.sendPortNumber.device >> 8;
			*(buf+23) = s.data.sendPortNumber.device;
			*(buf+24) = s.data.sendPortNumber.system >> 8;
			*(buf+25) = s.data.sendPortNumber.system;
			*(buf+26) = s.data.sendPortNumber.pcount >> 8;
			*(buf+27) = s.data.sendPortNumber.pcount;
			*(buf+28) = calcChecksum(buf, 28);
			valid = true;
		break;

		case 0x0091:
		case 0x0092:
			*(buf+22) = s.data.sendOutpChannels.device >> 8;
			*(buf+23) = s.data.sendOutpChannels.device;
			*(buf+24) = s.data.sendOutpChannels.port >> 8;
			*(buf+25) = s.data.sendOutpChannels.port;
			*(buf+26) = s.data.sendOutpChannels.system >> 8;
			*(buf+27) = s.data.sendOutpChannels.system;
			*(buf+28) = s.data.sendOutpChannels.count >> 8;
			*(buf+29) = s.data.sendOutpChannels.count;
			*(buf+30) = calcChecksum(buf, 30);
			valid = true;
		break;

		case 0x0093:
		case 0x0094:
			*(buf+22) = s.data.sendSize.device >> 8;
			*(buf+23) = s.data.sendSize.device;
			*(buf+24) = s.data.sendSize.port >> 8;
			*(buf+25) = s.data.sendSize.port;
			*(buf+26) = s.data.sendSize.system >> 8;
			*(buf+27) = s.data.sendSize.system;
			*(buf+28) = s.data.sendSize.type;
			*(buf+29) = s.data.sendSize.length >> 8;
			*(buf+30) = s.data.sendSize.length;
			*(buf+31) = calcChecksum(buf, 31);
			valid = true;
		break;

		case 0x0096:
			*(buf+22) = s.data.sendStatusCode.device >> 8;
			*(buf+23) = s.data.sendStatusCode.device;
			*(buf+24) = s.data.sendStatusCode.port >> 8;
			*(buf+25) = s.data.sendStatusCode.port;
			*(buf+26) = s.data.sendStatusCode.system >> 8;
			*(buf+27) = s.data.sendStatusCode.system;
			*(buf+28) = s.data.sendStatusCode.status >> 8;
			*(buf+29) = s.data.sendStatusCode.status;
			*(buf+30) = s.data.sendStatusCode.type;
			*(buf+31) = s.data.sendStatusCode.length >> 8;
			*(buf+32) = s.data.sendStatusCode.length;
			pos = 33;
			memcpy(buf+pos, s.data.sendStatusCode.str, s.data.sendStatusCode.length);
			pos += s.data.sendStatusCode.length;
			*(buf+pos) = calcChecksum(buf, pos);
			valid = true;
		break;

		case 0x0097:
			*(buf+22) = s.data.srDeviceInfo.device >> 8;
			*(buf+23) = s.data.srDeviceInfo.device;
			*(buf+24) = s.data.srDeviceInfo.system >> 8;
			*(buf+25) = s.data.srDeviceInfo.system;
			*(buf+26) = s.data.srDeviceInfo.flag >> 8;
			*(buf+27) = s.data.srDeviceInfo.flag;
			*(buf+28) = s.data.srDeviceInfo.objectID;
			*(buf+29) = s.data.srDeviceInfo.parentID;
			*(buf+30) = s.data.srDeviceInfo.herstID >> 8;
			*(buf+31) = s.data.srDeviceInfo.herstID;
			*(buf+32) = s.data.srDeviceInfo.deviceID >> 8;
			*(buf+33) = s.data.srDeviceInfo.deviceID;
			pos = 34;
			memcpy(buf+pos, s.data.srDeviceInfo.serial, 16);
			pos += 16;
			*(buf+pos) = s.data.srDeviceInfo.fwid >> 8;
			pos++;
			*(buf+pos) = s.data.srDeviceInfo.fwid;
			pos++;
			memcpy(buf+pos, s.data.srDeviceInfo.info, s.hlen + 3 - pos);
			*(buf+pos) = calcChecksum(buf, s.hlen + 3);
			valid = true;
		break;

		case 0x0098:
			*(buf+22) = s.data.reqPortCount.device >> 8;
			*(buf+23) = s.data.reqPortCount.device;
			*(buf+24) = s.data.reqPortCount.system >> 8;
			*(buf+25) = s.data.reqPortCount.system;
			*(buf+26) = calcChecksum(buf, 26);
			valid = true;
		break;
	}

	if (!valid)
	{
		delete[] buf;
		return 0;
	}

	return buf;
}
