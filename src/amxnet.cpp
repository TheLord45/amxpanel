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

#include <sys/utsname.h>
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
#include <asio/read.hpp>
#include "nameformat.h"

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
	callback = 0;
	cbWebConn = 0;
    init();
}

AMXNet::~AMXNet()
{
	sysl->TRACE(Syslog::EXIT, std::string("AMXNet::AMXNet(...)"));
	callback = 0;
}

void AMXNet::init()
{
	sendCounter = 0;
	initSend = false;
	ready = false;
	write_busy = false;
	strings::String version = "v1.00.00";
	// Initialize the devive info structure
	DEVICE_INFO di;
	// Answer to MC = 0x0017 --> MC = 0x0097
	di.objectID = 0;
	di.parentID = 0;
	di.manufacturerID = 1;
	di.deviceID = 0x0149;
	memset(di.serialNum, 0x20, sizeof(di.serialNum));
	memcpy(di.serialNum, "201903XTHE74201", 15);
	di.firmwareID = 0x0310;
	memset(di.versionInfo, 0, sizeof(di.versionInfo));
	strncpy(di.versionInfo, version.data(), version.length());
	memset(di.deviceInfo, 0, sizeof(di.deviceInfo));
	strncpy(di.deviceInfo, Configuration->getAMXPanelType().data(), ((Configuration->getAMXPanelType().length() < sizeof(di.deviceInfo))?Configuration->getAMXPanelType().length():(sizeof(di.deviceInfo)-1)));
	memset(di.manufacturerInfo, 0, sizeof(di.manufacturerInfo));
	strncpy(di.manufacturerInfo, "TheoSys", 7);
	di.format = 2;
	di.len = 4;
	memset(di.addr, 0, sizeof(di.addr));
	devInfo.push_back(di);
	// Kernel info
	di.objectID = 2;
	di.firmwareID = 0x0311;
	memset(di.serialNum, 0x20, sizeof(di.serialNum));
	memcpy(di.serialNum, "N/A", 3);
	memset(di.deviceInfo, 0, sizeof(di.deviceInfo));
	strncpy(di.deviceInfo, "Kernel", 6);
	memset(di.versionInfo, 0, sizeof(di.versionInfo));
#ifdef __linux__
	struct utsname kinfo;
	uname(&kinfo);
	strncpy (di.versionInfo, kinfo.release, sizeof(di.versionInfo));
#else
	strncpy(di.versionInfo, "2.16.00", 7);
#endif
	devInfo.push_back(di);
	memset(di.versionInfo, 0, sizeof(di.versionInfo));
	strncpy(di.versionInfo, version.data(), version.length());
	// Root file system
	di.objectID = 3;
	di.firmwareID = 0x0312;
	memset(di.deviceInfo, 0, sizeof(di.deviceInfo));
	strncpy(di.deviceInfo, "Root File System", 16);
	devInfo.push_back(di);
	// Bootrom
	di.objectID = 4;
	di.firmwareID = 0x0313;
	memset(di.deviceInfo, 0, sizeof(di.deviceInfo));
	strncpy(di.deviceInfo, "Bootrom", 7);
	devInfo.push_back(di);
	// File System
	di.objectID = 6;
	di.firmwareID = 0x0315;
	memset(di.deviceInfo, 0, sizeof(di.deviceInfo));
	strncpy(di.deviceInfo, "File System", 11);
	devInfo.push_back(di);
}

void AMXNet::start(asio::ip::tcp::resolver::results_type endpoints)
{
	sysl->TRACE(std::string("AMXNet::start(asio::ip::tcp::resolver::results_type endpoints)"));
	endpoints_ = endpoints;
	start_connect(endpoints_.begin());
	deadline_.async_wait(std::bind(&AMXNet::check_deadline, this));
}

bool AMXNet::isConnected()
{
	sysl->TRACE(std::string("AMXNet::isConnected() --> ")+((socket_.is_open())?"TRUE":"FALSE"));
	return socket_.is_open();
}

void AMXNet::stop()
{
	sysl->TRACE(std::string("AMXNet::stop: Stopping the client..."));

	if (stopped_)
		return;

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
	sysl->TRACE(std::string("AMXNet::start_connect(asio::ip::tcp::resolver::results_type::iterator endpoint_iter)"));

	if (endpoint_iter != endpoints_.end())
	{
		sysl->TRACE(strings::String("AMXNet::start_connect: Trying ")+endpoint_iter->endpoint().address().to_string()+":"+endpoint_iter->endpoint().port()+" ...\n");

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
	sysl->TRACE(std::string("handle_connect(const std::error_code& error, asio::ip::tcp::resolver::results_type::iterator endpoint_iter)"));

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
		sysl->errlog(std::string("AMXNet::handle_connect: Connect error: ")+error.message());

		// We need to close the socket used in the previous connection attempt
		// before starting a new one.
		socket_.close();

		// Try the next available endpoint.
		start_connect(++endpoint_iter);
	}
	else
	{
		sysl->log(Syslog::INFO, strings::String("AMXNet::handle_connect: Connected to ")+endpoint_iter->endpoint().address().to_string()+":"+endpoint_iter->endpoint().port());

		while (!stopped_ && !killed)
		{
			if (cbWebConn && !cbWebConn())
			{
				stop();
				break;
			}

			// Start the input actor.
			start_read();

			// Start the heartbeat actor.
			start_write();
		}

		if (!stopped_ && killed)
			stop();
	}
}

void AMXNet::start_read()
{
	sysl->TRACE(std::string("start_read()"));

	if (stopped_)
		return;

	// Set a deadline for the read operation.
	deadline_.expires_after(std::chrono::seconds(60));
//	input_buffer_.clear();
	protError = false;
	comm.clear();
	asio::error_code error;

	// Start an asynchronous operation to read a newline-delimited message.
	// Read the first byte. It should be 0x02
	if (asio::read(socket_, asio::buffer(buff_, 1), asio::transfer_exactly(1), error) == 1)
		handle_read(error, 1, RT_ID);
	else
		throw std::invalid_argument(error.message());

	if (asio::read(socket_, asio::buffer(buff_, 2), asio::transfer_exactly(2), error) == 2)
		handle_read(error, 2, RT_LEN);
	else
		throw std::invalid_argument(error.message());

	if (asio::read(socket_, asio::buffer(buff_, 1), asio::transfer_exactly(1), error) == 1)
		handle_read(error, 1, RT_SEP1);
	else
		throw std::invalid_argument(error.message());

	if (asio::read(socket_, asio::buffer(buff_, 1), asio::transfer_exactly(1), error) == 1)
		handle_read(error, 1, RT_TYPE);
	else
		throw std::invalid_argument(error.message());

	if (asio::read(socket_, asio::buffer(buff_, 2), asio::transfer_exactly(2), error) == 2)
		handle_read(error, 2, RT_WORD1);
	else
		throw std::invalid_argument(error.message());

	if (asio::read(socket_, asio::buffer(buff_, 2), asio::transfer_exactly(2), error) == 2)
		handle_read(error, 2, RT_DEVICE);
	else
		throw std::invalid_argument(error.message());

	if (asio::read(socket_, asio::buffer(buff_, 2), asio::transfer_exactly(2), error) == 2)
		handle_read(error, 2, RT_WORD2);
	else
		throw std::invalid_argument(error.message());

	if (asio::read(socket_, asio::buffer(buff_, 2), asio::transfer_exactly(2), error) == 2)
		handle_read(error, 2, RT_WORD3);
	else
		throw std::invalid_argument(error.message());

	if (asio::read(socket_, asio::buffer(buff_, 2), asio::transfer_exactly(2), error) == 2)
		handle_read(error, 2, RT_WORD4);
	else
		throw std::invalid_argument(error.message());

	if (asio::read(socket_, asio::buffer(buff_, 2), asio::transfer_exactly(2), error) == 2)
		handle_read(error, 2, RT_WORD5);
	else
		throw std::invalid_argument(error.message());

	if (asio::read(socket_, asio::buffer(buff_, 1), asio::transfer_exactly(1), error) == 1)
		handle_read(error, 1, RT_SEP2);
	else
		throw std::invalid_argument(error.message());

	if (asio::read(socket_, asio::buffer(buff_, 2), asio::transfer_exactly(2), error) == 2)
		handle_read(error, 2, RT_COUNT);
	else
		throw std::invalid_argument(error.message());

	if (asio::read(socket_, asio::buffer(buff_, 2), asio::transfer_exactly(2), error) == 2)
		handle_read(error, 2, RT_MC);
	else
		throw std::invalid_argument(error.message());

	// Calculate the length of the data block. This is the rest of the total length
	size_t len = (comm.hlen + 3) - 0x0015;

	if (len >= 2048)
	{
		sysl->errlog(strings::String("AMXnet::start_read: Length to read is ")+len+" bytes, but the buffer is only 2048 bytes!");
		return;
	}

	if (asio::read(socket_, asio::buffer(buff_, len), asio::transfer_exactly(len), error) == len)
		handle_read(error, len, RT_DATA);
	else
		throw std::invalid_argument(error.message());
}

void AMXNet::handle_read(const asio::error_code& error, size_t n, R_TOKEN tk)
{
	sysl->TRACE(std::string("handle_read(const std::error_code& error, std::size_t n, R_TOKEN tk)"));

	if (stopped_)
		return;

	if ((cbWebConn && !cbWebConn()) || killed)
	{
		stop();
		return;
	}

	uint32_t dw;
	int val, pos;
	size_t len;
	bool ignore = false;
	ANET_SEND s;		// Used to answer system requests

	if (!error)
	{
		len = (n < 2048) ? n : 2047;
		input_buffer_.assign((char *)&buff_[0], len);

		if (len >= 8)
			sysl->DebugMsg(strings::String("AMXNet::handle_read: read:\n")+NameFormat::strToHex(input_buffer_, 8, true)+"\n\t\t\t\t\tToken: "+tk+", "+len+" bytes");
		else
			sysl->DebugMsg(strings::String("AMXNet::handle_read: read: ")+NameFormat::strToHex(input_buffer_, 1)+", Token: "+tk+", "+len+" bytes");

		switch (tk)
		{
			case RT_ID:
				if (buff_[0] != 0x02)
					protError = true;
				else
					comm.ID = buff_[0];
			break;

			case RT_LEN: 	comm.hlen = makeWord(buff_[0], buff_[1]); break;

			case RT_SEP1:
				if (buff_[0] != 0x02)
					protError = true;
				else
					comm.sep1 = buff_[0];
			break;

			case RT_TYPE: 	comm.type = buff_[0]; break;
			case RT_WORD1:	comm.unk1 = makeWord(buff_[0], buff_[1]); break;
			case RT_DEVICE:	comm.device1 = makeWord(buff_[0], buff_[1]); break;
			case RT_WORD2:	comm.port1 = makeWord(buff_[0], buff_[1]); break;
			case RT_WORD3:	comm.system = makeWord(buff_[0], buff_[1]); break;
			case RT_WORD4:	comm.device2 = makeWord(buff_[0], buff_[1]); break;
			case RT_WORD5:	comm.port2 = makeWord(buff_[0], buff_[1]); break;

			case RT_SEP2:
				if (buff_[0] != 0x0f)
					protError = true;
				else
					comm.unk6 = buff_[0];
			break;

			case RT_COUNT:	comm.count = makeWord(buff_[0], buff_[1]); break;
			case RT_MC:		comm.MC = makeWord(buff_[0], buff_[1]); break;

			case RT_DATA:
				if (protError || stopped_ || killed)
					break;

				sysl->TRACE(strings::String("AMXNet::handle_read: Received message type: 0x")+NameFormat::toHex(comm.MC, 4));

				switch (comm.MC)
				{
					case 0x0001:	// ACK
					case 0x0002:	// NAK
						comm.checksum = buff_[0];
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
						comm.data.chan_state.device = makeWord(buff_[0], buff_[1]);
						comm.data.chan_state.port = makeWord(buff_[2], buff_[3]);
						comm.data.chan_state.system = makeWord(buff_[4], buff_[5]);
						comm.data.chan_state.channel = makeWord(buff_[6], buff_[7]);
						comm.checksum = buff_[8];

						s.channel = comm.data.chan_state.channel;
						s.level = 0;
						s.port = comm.data.chan_state.port;
						s.value = 0;

						switch(comm.MC)
						{
							case 0x0006: s.MC = 0x0086; break;
							case 0x0007: s.MC = 0x0087; break;
						}

						if (comm.MC < 0x0020)
						{
							if (callback)
								callback(comm);
						}
						else
							sendCommand(s);
					break;

					case 0x000a:	// level value change
					case 0x008a:
						comm.data.message_value.device = makeWord(buff_[0], buff_[1]);
						comm.data.message_value.port = makeWord(buff_[2], buff_[3]);
						comm.data.message_value.system = makeWord(buff_[4], buff_[5]);
						comm.data.message_value.value = makeWord(buff_[6], buff_[7]);
						comm.data.message_value.type = buff_[8];
						val = (int)buff_[8];

						switch (val)
						{
							case 0x010: comm.data.message_value.content.byte = buff_[9]; comm.checksum = buff_[10]; break;
							case 0x011: comm.data.message_value.content.ch = buff_[9]; comm.checksum = buff_[10]; break;
							case 0x020: comm.data.message_value.content.integer = makeWord(buff_[9], buff_[10]); comm.checksum = buff_[11]; break;
							case 0x021: comm.data.message_value.content.sinteger = makeWord(buff_[9], buff_[10]); comm.checksum = buff_[11]; break;
							case 0x040: comm.data.message_value.content.dword = makeDWord(buff_[9], buff_[10], buff_[11], buff_[12]); comm.checksum = buff_[13]; break;
							case 0x041: comm.data.message_value.content.sdword = makeDWord(buff_[9], buff_[10], buff_[11], buff_[12]); comm.checksum = buff_[13]; break;

							case 0x04f:
								dw = makeDWord(buff_[9], buff_[10], buff_[11], buff_[12]);
								memcpy(&comm.data.message_value.content.fvalue, &dw, 4);
								comm.checksum = buff_[13];
							break;

							case 0x08f:
								memcpy(&comm.data.message_value.content.dvalue, &buff_[9], 8);	// FIXME: wrong byte order on Intel CPU?
								comm.checksum = buff_[17];
							break;
						}

						if (callback)
							callback(comm);
					break;

					case 0x000b:	// string value change
					case 0x008b:
					case 0x000c:	// command string
					case 0x008c:
						comm.data.message_string.device = makeWord(buff_[0], buff_[1]);
						comm.data.message_string.port = makeWord(buff_[2], buff_[3]);
						comm.data.message_string.system = makeWord(buff_[4], buff_[5]);
						comm.data.message_string.type = buff_[6];
						comm.data.message_string.length = makeWord(buff_[7], buff_[8]);
						len = (buff_[6] == 0x01) ? comm.data.message_string.length : comm.data.message_string.length * 2;
						memcpy(&comm.data.message_string.content[0], &buff_[9], len);
						pos = len + 10;
						comm.checksum = buff_[pos];

						if (callback)
							callback(comm);
					break;

					case 0x000e:	// request level value
						comm.data.level.device = makeWord(buff_[0], buff_[1]);
						comm.data.level.port = makeWord(buff_[2], buff_[3]);
						comm.data.level.system = makeWord(buff_[4], buff_[5]);
						comm.data.level.level = makeWord(buff_[6], buff_[7]);
						comm.checksum = buff_[8];

						if (callback)
							callback(comm);
					break;

					case 0x000f:	// request output channel status
						comm.data.channel.device = makeWord(buff_[0], buff_[1]);
						comm.data.channel.port = makeWord(buff_[2], buff_[3]);
						comm.data.channel.system = makeWord(buff_[4], buff_[5]);
						comm.data.channel.channel = makeWord(buff_[6], buff_[7]);
						comm.checksum = buff_[8];

						if (callback)
							callback(comm);
					break;

					case 0x0010:	// request port count
					case 0x0017:	// request device info
						comm.data.reqPortCount.device = makeWord(buff_[0], buff_[1]);
						comm.data.reqPortCount.system = makeWord(buff_[2], buff_[3]);
						comm.checksum = buff_[4];
						s.channel = false;
						s.level = 0;
						s.port = 0;
						s.value = 0x0015;
						s.MC = (comm.MC == 0x0010) ? 0x0090 : 0x0097;

						if (s.MC == 0x0097)
						{
							comm.data.srDeviceInfo.device = comm.device2;
							comm.data.srDeviceInfo.system = comm.system;
							comm.data.srDeviceInfo.flag = 0x0000;
							comm.data.srDeviceInfo.parentID = 0;
							comm.data.srDeviceInfo.herstID = 1;
							msg97fill(&comm);
						}
						else
							sendCommand(s);
					break;

					case 0x0011:	// request output channel count
					case 0x0012:	// request level count
					case 0x0013:	// request string size
					case 0x0014:	// request command size
						comm.data.reqOutpChannels.device = makeWord(buff_[0], buff_[1]);
						comm.data.reqOutpChannels.port = makeWord(buff_[2], buff_[3]);
						comm.data.reqOutpChannels.system = makeWord(buff_[4], buff_[5]);
						comm.checksum = buff_[6];
						s.channel = false;
						s.level = 0;
						s.port = comm.data.reqOutpChannels.port;
						s.value = 0;

						switch (comm.MC)
						{
							case 0x0011:
								s.MC = 0x0091;
								s.value = 0x0f75;	// # channels
							break;
							case 0x0012:
								s.MC = 0x0092;
								s.value = 0x000d;	// # levels
							break;
							case 0x0013:
								s.MC = 0x0093;
								s.value = 0x00c7;	// string size
							break;
							case 0x0014:
								s.MC = 0x0094;
								s.value = 0x00c7;	// command size
							break;
						}

						sendCommand(s);
					break;

					case 0x0015:	// request level size
						comm.data.reqLevels.device = makeWord(buff_[0], buff_[1]);
						comm.data.reqLevels.port = makeWord(buff_[2], buff_[3]);
						comm.data.reqLevels.system = makeWord(buff_[4], buff_[5]);
						comm.data.reqLevels.level = makeWord(buff_[6], buff_[7]);
						comm.checksum = buff_[8];
						s.channel = false;
						s.level = comm.data.reqLevels.level;
						s.port = comm.data.reqLevels.port;
						s.value = 0;
						s.MC = 0x0095;
						sendCommand(s);
					break;

					case 0x0016:	// request status code
						comm.data.sendStatusCode.device = makeWord(buff_[0], buff_[1]);
						comm.data.sendStatusCode.port = makeWord(buff_[2], buff_[3]);
						comm.data.sendStatusCode.system = makeWord(buff_[4], buff_[5]);

						if (callback)
							callback(comm);
					break;

					case 0x0097:	// receive device info
						comm.data.srDeviceInfo.device = makeWord(buff_[0], buff_[1]);
						comm.data.srDeviceInfo.system = makeWord(buff_[2], buff_[3]);
						comm.data.srDeviceInfo.flag = makeWord(buff_[4], buff_[5]);
						comm.data.srDeviceInfo.objectID = buff_[6];
						comm.data.srDeviceInfo.parentID = buff_[7];
						comm.data.srDeviceInfo.herstID = makeWord(buff_[8], buff_[9]);
						comm.data.srDeviceInfo.deviceID = makeWord(buff_[10], buff_[11]);
						memcpy(comm.data.srDeviceInfo.serial, &buff_[12], 16);
						comm.data.srDeviceInfo.fwid = makeWord(buff_[28], buff_[29]);
						memcpy(comm.data.srDeviceInfo.info, &buff_[30], comm.hlen - 0x0015 - 29);
						comm.checksum = buff_[comm.hlen + 3];
						// Prepare answer
						s.channel = false;
						s.level = 0;
						s.port = 0;
						s.value = 0;

						if (!initSend)
						{
							s.MC = 0x0097;
							initSend = true;
						}
						else if (!ready)
						{
							// Send counts
							s.MC = 0x0090;
							s.value = 0x0015;	// # ports
							sendCommand(s);
							s.MC = 0x0091;
							s.value = 0x0f75;	// # channels
							sendCommand(s);
							s.MC = 0x0092;
							s.value = 0x000d;	// # levels
							sendCommand(s);
							s.MC = 0x0093;
							s.value = 0x00c7;	// string size
							sendCommand(s);
							s.MC = 0x0094;
							s.value = 0x00c7;	// command size
							sendCommand(s);
							s.MC = 0x0098;
							ready = true;
						}
						else
							break;

						sendCommand(s);

						sysl->TRACE(strings::String("AMXNet::handle_read: S/N: ")+(char *)&comm.data.srDeviceInfo.serial[0]+" | "+(char *)&comm.data.srDeviceInfo.info[0]);
					break;

					case 0x00a1:	// request status
						reqDevStatus = makeWord(buff_[0], buff_[1]);
						comm.checksum = buff_[2];
					break;

					case 0x0501:    // ping
						comm.data.chan_state.device = makeWord(buff_[0], buff_[1]);
						comm.data.chan_state.system = makeWord(buff_[2], buff_[3]);
						s.channel = 0;
						s.level = 0;
						s.port = 0;
						s.value = 0;
						s.MC = 0x0581;
						sendCommand(s);
					break;
				}
			break;

			default:		// should never happen and is only to satisfy the compiler
				ignore = true;
		}
	}
	else
	{
		sysl->errlog(std::string("AMXNet::handle_read: Error on receive: ")+error.message());
		stop();
	}
}

bool AMXNet::sendCommand (const ANET_SEND& s)
{
	sysl->TRACE(std::string("AMXNet::sendCommand (const ANET_SEND& s)"));

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

		case 0x0086:
			com.data.channel.device = com.device2;
			com.data.channel.port = s.port;
			com.data.channel.system = com.system;
			com.data.channel.channel = s.channel;
			com.hlen = 0x0016 - 0x0003 + sizeof(ANET_CHANNEL);
			comStack.push_back(com);
			status = true;
		break;

		case 0x0087:
			com.data.channel.device = com.device2;
			com.data.channel.port = s.port;
			com.data.channel.system = com.system;
			com.data.channel.channel = s.channel;
			com.hlen = 0x0016 - 0x0003 + sizeof(ANET_CHANNEL);
			comStack.push_back(com);
			status = true;
			break;

		case 0x008a:		// level value changed
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

		case 0x0095:		// suported level types
			com.data.sendLevSupport.device = com.device2;
			com.data.sendLevSupport.port = s.port;
			com.data.sendLevSupport.system = com.system;
			com.data.sendLevSupport.level = s.level;
			com.data.sendLevSupport.num = 6;
			com.data.sendLevSupport.types[0] = 0x10;
			com.data.sendLevSupport.types[1] = 0x11;
			com.data.sendLevSupport.types[2] = 0x20;
			com.data.sendLevSupport.types[3] = 0x21;
			com.data.sendLevSupport.types[4] = 0x40;
			com.data.sendLevSupport.types[5] = 0x41;
			com.hlen = 0x0016 - 0x0003 + sizeof(ANET_LEVSUPPORT);
			comStack.push_back(com);
		break;

		case 0x0096:		// Status code
			com.data.sendStatusCode.device = com.device2;
			com.data.sendStatusCode.port = s.port;
			com.data.sendStatusCode.system = com.system;
			com.data.sendStatusCode.status = 0;
			com.data.sendStatusCode.type = 0x11;
			com.data.sendStatusCode.length = 2;
			com.data.sendStatusCode.str[0] = 'O';
			com.data.sendStatusCode.str[1] = 'K';
			com.hlen = 0x0016 - 3 + 13;
			comStack.push_back(com);
		break;

		case 0x0097:		// device info
			com.data.srDeviceInfo.device = com.device2;
			com.data.srDeviceInfo.system = com.system;
			com.data.srDeviceInfo.flag = 0x0000;
			com.data.srDeviceInfo.objectID = 0;
			com.data.srDeviceInfo.parentID = 0;
			com.data.srDeviceInfo.herstID = 1;
			msg97fill(&com);
			status = true;
		break;

		case 0x0098:
			com.data.reqPortCount.device = com.device2;
			com.data.reqPortCount.system = com.system;
			com.hlen = 0x0016 - 3 + 4;
			comStack.push_back(com);
			status = true;
		break;

		case 0x0581:		// Pong
			com.data.srDeviceInfo.device = Configuration->getAMXChannel();
			com.data.srDeviceInfo.system = Configuration->getAMXSystem();
			com.data.srDeviceInfo.herstID = devInfo[0].manufacturerID;
			com.data.srDeviceInfo.deviceID = devInfo[0].deviceID;
			com.data.srDeviceInfo.info[0] = 2;	// Type: IPv4 address
			com.data.srDeviceInfo.info[1] = 4;	// length of following data

			{
				strings::String addr = socket_.local_endpoint().address().to_string();
				std::vector<strings::String> parts = addr.split('.');

				for (size_t i = 0; i < parts.size(); i++)
					com.data.srDeviceInfo.info[i+2] = (unsigned char)atoi(parts[i].data());
			}

			com.hlen = 0x0016 - 3 + 14;
			comStack.push_back(com);
			status = true;
		break;
	}

	if (status)
		start_write();

	return status;
}

int AMXNet::msg97fill(ANET_COMMAND *com)
{
	int pos;
	unsigned char buf[128];

	for (size_t i = 0; i < devInfo.size(); i++)
	{
		pos = 0;

		if (i == 0)
			com->sep1 = 0x12;
		else
			com->sep1 = 0x02;

		memset(buf, 0, sizeof(buf));
		com->data.srDeviceInfo.objectID = devInfo[i].objectID;
		com->data.srDeviceInfo.parentID = devInfo[i].parentID;
		com->data.srDeviceInfo.herstID = devInfo[i].manufacturerID;
		com->data.srDeviceInfo.deviceID = devInfo[i].deviceID;
		memcpy(com->data.srDeviceInfo.serial, devInfo[i].serialNum, 16);
		com->data.srDeviceInfo.fwid = devInfo[i].firmwareID;
		memcpy(buf, devInfo[i].versionInfo, strlen(devInfo[i].versionInfo));
		pos = strlen(devInfo[i].versionInfo) + 1;
		memcpy(buf+pos, devInfo[i].deviceInfo, strlen(devInfo[i].deviceInfo));
		pos += strlen(devInfo[i].deviceInfo) + 1;
		memcpy(buf+pos, devInfo[i].manufacturerInfo, strlen(devInfo[i].manufacturerInfo));
		pos += strlen(devInfo[i].manufacturerInfo)+1;
		*(buf+pos) = 0x02;	// type IP address
		pos++;
		*(buf+pos) = 0x04;	// field length: 4 bytes
		// Now the IP Address
		strings::String addr = socket_.local_endpoint().address().to_string();
		std::vector<strings::String> parts = addr.split('.');

		for (size_t i = 0; i < parts.size(); i++)
		{
			pos++;
			*(buf+pos) = (unsigned char)atoi(parts[i].data());
		}

		pos++;
		com->data.srDeviceInfo.len = pos;
		memcpy(com->data.srDeviceInfo.info, buf, pos);
		com->hlen = 0x0016 - 3 + 31 + pos - 1;
		comStack.push_back(*com);
	}

	return pos;
}

void AMXNet::start_write()
{
	sysl->TRACE(std::string("AMXNet::start_write()"));

	if (stopped_ || killed)
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
			sysl->errlog(strings::String("AMXNet::start_write: Error creating a buffer! Token number: ")+send.MC);
			continue;
		}

		asio::async_write(socket_, asio::buffer(buf, send.hlen + 4), std::bind(&AMXNet::handle_write, this, _1));
		delete[] buf;
	}

	write_busy = false;
}

void AMXNet::handle_write(const std::error_code& error)
{
	sysl->TRACE(std::string("handle_write(const std::error_code& error)"));

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
	sysl->TRACE(std::string("check_deadline()"));

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
	sysl->TRACE(std::string("AMXNet::calcChecksum(const unsigned char* buffer, size_t len)"));
	unsigned long sum = 0;

	for (size_t i = 0; i < len; i++)
		sum += (unsigned long)(*(buffer+i)) & 0x000000ff;

	sum &= 0x000000ff;
	sysl->TRACE(strings::String("AMXNet::calcChecksum: Checksum=")+NameFormat::toHex((int)sum, 2)+", "+len+" bytes.");
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
	sysl->TRACE(std::string("AMXNet::makeBuffer (const ANET_COMMAND& s)"));

	int pos = 0;
	bool valid = false;
	unsigned char *buf;

	try
	{
		buf = new unsigned char[s.hlen+5];
		memset(buf, 0, s.hlen+5);
	}
	catch(std::exception& e)
	{
		sysl->errlog(std::string("AMXNet::makeBuffer: Error allocating memory: ")+e.what());
		return 0;
	}

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

		case 0x0095:
			*(buf+22) = s.data.sendLevSupport.device >> 8;
			*(buf+23) = s.data.sendLevSupport.device;
			*(buf+24) = s.data.sendLevSupport.port >> 8;
			*(buf+25) = s.data.sendLevSupport.port;
			*(buf+26) = s.data.sendLevSupport.system >> 8;
			*(buf+27) = s.data.sendLevSupport.system;
			*(buf+28) = s.data.sendLevSupport.level >> 8;
			*(buf+29) = s.data.sendLevSupport.level;
			*(buf+30) = s.data.sendLevSupport.num;
			*(buf+31) = s.data.sendLevSupport.types[0];
			*(buf+32) = s.data.sendLevSupport.types[1];
			*(buf+33) = s.data.sendLevSupport.types[2];
			*(buf+34) = s.data.sendLevSupport.types[3];
			*(buf+35) = s.data.sendLevSupport.types[4];
			*(buf+36) = s.data.sendLevSupport.types[5];
			*(buf+37) = calcChecksum(buf, 37);
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
			memcpy(buf+pos, s.data.srDeviceInfo.info, s.data.srDeviceInfo.len);
			pos += s.data.srDeviceInfo.len;
			*(buf+pos) = calcChecksum(buf, pos);
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

		case 0x0581:	// Pong
			*(buf+22) = s.data.srDeviceInfo.device >> 8;
			*(buf+23) = s.data.srDeviceInfo.device;
			*(buf+24) = s.data.srDeviceInfo.system >> 8;
			*(buf+25) = s.data.srDeviceInfo.system;
			*(buf+26) = s.data.srDeviceInfo.herstID >> 8;
			*(buf+27) = s.data.srDeviceInfo.herstID;
			*(buf+28) = s.data.srDeviceInfo.deviceID >> 8;
			*(buf+29) = s.data.srDeviceInfo.deviceID;
			*(buf+30) = s.data.srDeviceInfo.info[0];
			*(buf+31) = s.data.srDeviceInfo.info[1];
			*(buf+32) = s.data.srDeviceInfo.info[2];
			*(buf+33) = s.data.srDeviceInfo.info[3];
			*(buf+34) = s.data.srDeviceInfo.info[4];
			*(buf+35) = s.data.srDeviceInfo.info[5];
			*(buf+36) = calcChecksum(buf, 36);
			valid = true;
		break;
	}

	if (!valid)
	{
		delete[] buf;
		return 0;
	}

	strings::String b((char *)buf, s.hlen+4);
	sysl->TRACE(strings::String("AMXNet::makeBuffer:\n")+NameFormat::strToHex(b, 8, true));
	return buf;
}
