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

#include <sys/utsname.h>
#ifdef __APPLE__
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>
#else
#include <asio/buffer.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/read_until.hpp>
#include <asio/steady_timer.hpp>
#include <asio/write.hpp>
#include <asio/read.hpp>
#endif
#include <functional>
#include <iostream>
#include <cstring>
#include <string>
#include <chrono>
#include <thread>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "syslog.h"
#include "strings.h"
#include "config.h"
#include "amxnet.h"
#include "nameformat.h"
#include "trace.h"
#include "str.h"
#include "expand.h"
#include "directory.h"

#ifdef __APPLE__
using namespace boost;
#endif

extern Syslog *sysl;
extern Config *Configuration;

using asio::steady_timer;
using asio::ip::tcp;

using namespace amx;
using namespace std;

using placeholders::_1;
using placeholders::_2;

string cmdList[] = {
	"@WLD-", "@AFP-", "@GCE-", "@APG-", "@CPG-", "@DPG-", "@PDR-", "@PHE-",
	"@PHP-", "@PHT-", "@PPA-", "@PPF-", "@PPG-", "@PPK-", "@PPM-", "@PPN-",
	"@PPT-", "@PPX", "@PSE-", "@PSP-", "@PST-", "PAGE-", "PPOF-", "PPOG-",
	"PPON-", "^ANI-", "^APF-", "^BAT-", "^BAU-", "^BCB-", "^BCF-", "^BCT-",
	"^BDO-", "^BFB-", "^BIM-", "^BLN-", "^BMC-", "^BMF-", "^BMI-", "^BML-",
	"^BMP-", "^BNC-", "^BNN-", "^BNT-", "^BOP-", "^BOR-", "^BOS-", "^BPP-",
	"^BRD-", "^BSF-", "^BSP-", "^BSM-", "^BSO-", "^BVL-", "^BVN-", "^BVP-",
	"^BVT-", "^BWW-", "^CPF-", "^DLD-", "^DPF-", "^ENA-", "^FON-", "^GDI-",
	"^GIV-", "^GLH-", "^GLL-", "^GRD-", "^GRU-", "^GSC-", "^GSN-", "^ICO-",
	"^IRM-", "^JSB-", "^JSI-", "^JST-", "^MBT-", "^MDC-", "^SHO-", "^TEC-",
	"^TEF-", "^TOP-", "^TXT-", "^UNI-", "^LPC-", "^LPR-", "^LPS-", "?BCB-",
	"?BCF-", "?BCT-", "?BMP-", "?BOP-", "?BRD-", "?BWW-", "?FON-", "?ICO-",
	"?JSB-", "?JSI-", "?JST-", "?TEC-", "?TEF-", "?TXT-", "ABEEP", "ADBEEP",
	"@AKB-", "AKEYB-", "AKEYP-", "AKEYR-", "@AKP-", "@AKR", "BEEP", "BRIT-",
	"@BRT-", "DBEEP", "@EKP-", "PKEYP-", "@PKP-", "SETUP", "SHUTDOWN", "SLEEP",
	"@SOU-", "@TKP-", "TPAGEON", "TPAGEOFF", "@VKB", "WAKE", "^CAL", "^KPS-",
	"^VKS-", "@PWD-", "^PWD-", "^BBR-", "^RAF-", "^RFR-", "^RMF-", "^RSR-",
	"^MODEL?", "^ICS-", "^ICE-", "^ICM-", "^PHN-", "?PHN-", "LEVON", "RXON",
	"\0"
};

#define NUMBER_CMDS		144

AMXNet::AMXNet()
	: deadline_(io_context),
	  heartbeat_timer_(io_context),
	  socket_(io_context)
{
	sysl->TRACE(Syslog::ENTRY, "AMXNet::AMXNet()");
	init();
}

AMXNet::AMXNet(const string& sn)
	: deadline_(io_context),
	  heartbeat_timer_(io_context),
	  socket_(io_context),
	  serNum(sn)
{
	sysl->TRACE(Syslog::ENTRY, "AMXNet::AMXNet(const string& sn)");
	init();
}

AMXNet::AMXNet(const string& sn, const string& nm)
	: deadline_(io_context),
	  heartbeat_timer_(io_context),
	  socket_(io_context),
	  panName(nm),
	  serNum(sn)
{
	sysl->TRACE(Syslog::ENTRY, "AMXNet::AMXNet(const string& sn)");
	size_t pos = nm.find(" (TPC)");

	if (pos != string::npos)
	{
		panName = nm.substr(0, pos) + "i";
		sysl->TRACE("AMXNet::AMXNet: Converted TP name: "+panName);
	}

	init();
}

AMXNet::~AMXNet()
{
	devInfo.clear();
	comStack.clear();
	callback = 0;
	stop();
    io_context.stop();
	sysl->TRACE(Syslog::EXIT, "AMXNet::~AMXNet()");
}

void AMXNet::init()
{
	callback = 0;
	stopped_ = false;
	sendCounter = 0;
	initSend = false;
	ready = false;
	write_busy = false;
	string version = "v2.01.00";		// A version > 2.0 is needed for file transfer!
	int devID = 0x0163, fwID = 0x0290;

	if (Configuration->getAMXPanelType().length() > 0)
		panName = Configuration->getAMXPanelType();
	else if (panName.empty())
		panName.assign("TheoSys");

	if (panName.find("MVP") != string::npos && panName.find("5200") != string::npos)
	{
		devID = 0x0149;
		fwID = 0x0310;
	}

	// Initialize the devive info structure
	DEVICE_INFO di;
	// Answer to MC = 0x0017 --> MC = 0x0097
	di.objectID = 0;
	di.parentID = 0;
	di.manufacturerID = 1;
	di.deviceID = devID;
	memset(di.serialNum, 0x20, sizeof(di.serialNum));

	if (!serNum.empty())
		memcpy(di.serialNum, serNum.c_str(), serNum.length());

	di.firmwareID = fwID;
	memset(di.versionInfo, 0, sizeof(di.versionInfo));
	strncpy(di.versionInfo, version.c_str(), version.length());
	memset(di.deviceInfo, 0, sizeof(di.deviceInfo));
	strncpy(di.deviceInfo, panName.c_str(), min(panName.length(), sizeof(di.deviceInfo)-1));
	memset(di.manufacturerInfo, 0, sizeof(di.manufacturerInfo));
	strncpy(di.manufacturerInfo, "TheoSys", 7);
	di.format = 2;
	di.len = 4;
	memset(di.addr, 0, sizeof(di.addr));
	devInfo.push_back(di);
	// Kernel info
	di.objectID = 2;
	di.firmwareID = fwID + 1;
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
	strncpy(di.versionInfo, "4.00.00", 7);
#endif
	devInfo.push_back(di);
}

void AMXNet::start(asio::ip::tcp::resolver::results_type endpoints, int id)
{
	DECL_TRACER(string("AMXNet::start(asio::ip::tcp::resolver::results_type endpoints, int id)"));
	endpoints_ = endpoints;
	panelID = id;

	try
	{
		start_connect(endpoints_.begin());
		deadline_.async_wait(bind(&AMXNet::check_deadline, this));
	}
	catch (std::exception& e)
	{
		sysl->errlog(string("AMXNet::start: Error: ")+e.what());
	}
}

bool AMXNet::isConnected()
{
	DECL_TRACER(string("AMXNet::isConnected() --> ")+((socket_.is_open())?"TRUE":"FALSE"));
	return socket_.is_open();
}

void AMXNet::stop()
{
	DECL_TRACTHR("AMXNet::stop: Stopping the client...");

	if (stopped_)
		return;

	stopped_ = true;
#ifdef __APPLE__
	system::error_code ignored_error;
#else
	error_code ignored_error;
#endif
	try
	{
		deadline_.cancel();
		heartbeat_timer_.cancel();
		socket_.shutdown(asio::socket_base::shutdown_both, ignored_error);
		socket_.close(ignored_error);
		sysl->TRACE(string("AMXNet::stop: Client was stopped."), true);
	}
	catch (std::exception& e)
	{
		sysl->errlogThr(string("AMXNet::stop: Error: ")+e.what());
	}
}

void AMXNet::Run()
{
	DECL_TRACER("AMXNet::Run()");

	while (reconCounter < 3)
	{
		try
		{
			asio::ip::tcp::resolver r(io_context);
			start(r.resolve(Configuration->getAMXController(), to_string(Configuration->getAMXPort())), panelID);
			io_context.run();
			sysl->TRACE("AMXNet::Run: Thread ended.");

			if (stopped_)
				break;
		}
		catch (std::exception& e)
		{
			sysl->errlog("AMXNet::Run: Error connecting to "+Configuration->getAMXController()+":"+to_string(Configuration->getAMXPort())+" ["+e.what()+"]");
		}

		reconCounter++;
	}

	stopped_ = true;
}

void AMXNet::start_connect(asio::ip::tcp::resolver::results_type::iterator endpoint_iter)
{
	DECL_TRACER("AMXNet::start_connect(asio::ip::tcp::resolver::results_type::iterator endpoint_iter)");

	if (socket_.is_open())
	{
		stop();
		return;
	}

	if (endpoint_iter != endpoints_.end())
	{
		sysl->TRACE("AMXNet::start_connect: Trying "+endpoint_iter->endpoint().address().to_string()+":"+to_string(endpoint_iter->endpoint().port())+" ...");

		// Set a deadline for the connect operation.
		deadline_.expires_after(chrono::seconds(120));
		stopped_ = false;
		// Start the asynchronous connect operation.
		socket_.async_connect(endpoint_iter->endpoint(), bind(&AMXNet::handle_connect, this, _1, endpoint_iter));
	}
	else
	{
		// There are no more endpoints to try. Shut down the client.
		stop();
	}
}

void AMXNet::handle_connect(const error_code& error, asio::ip::tcp::resolver::results_type::iterator endpoint_iter)
{
	DECL_TRACTHR("AMXNet::handle_connect(const error_code& error, asio::ip::tcp::resolver::results_type::iterator endpoint_iter)");

	reconCounter = 0;

	if (stopped_)
		return;

	// The async_connect() function automatically opens the socket at the start
	// of the asynchronous operation. If the socket is closed at this time then
	// the timeout handler must have run first.
	if (!socket_.is_open())
	{
		sysl->TRACE("AMXNet::handle_connect: Connect timed out", true);

		// Try the next available endpoint.
		start_connect(++endpoint_iter);
    }
	else if (error)		// Check if the connect operation failed before the deadline expired.
	{
		sysl->errlogThr(string("AMXNet::handle_connect: Connect error: ")+error.message());

		// We need to close the socket used in the previous connection attempt
		// before starting a new one.
		socket_.close();

		// Try the next available endpoint.
		start_connect(++endpoint_iter);
	}
	else
	{
		sysl->logThr(Syslog::INFO, "AMXNet::handle_connect: Connected to "+endpoint_iter->endpoint().address().to_string()+":"+to_string(endpoint_iter->endpoint().port()));

		try
		{
			while (isRunning())
			{
				// Start the input actor.
				start_read();

				// Start the output actor.
				if (isRunning())
					start_write();
			}

			if (!stopped_ && killed)
				stop();
		}
		catch (std::exception& e)
		{
			sysl->errlogThr(string("AMXNet::handle_connect: Error: ")+e.what());
		}
	}
}

void AMXNet::start_read()
{
	DECL_TRACTHR("start_read()");

	if (!isRunning())
		return;

	// Set a deadline for the read operation.
	deadline_.expires_after(chrono::seconds(120));
	protError = false;
	comm.clear();
#ifdef __APPLE__
	system::error_code error;
#else
	asio::error_code error;
#endif
	// Start an asynchronous operation to read a message.
	// Read the first byte. It should be 0x02
	if (asio::read(socket_, asio::buffer(buff_, 1), asio::transfer_exactly(1), error) == 1)
		handle_read(error, 1, RT_ID);
	else
		throw invalid_argument(error.message());

	if (asio::read(socket_, asio::buffer(buff_, 2), asio::transfer_exactly(2), error) == 2)
		handle_read(error, 2, RT_LEN);
	else
		throw invalid_argument(error.message());

	if (asio::read(socket_, asio::buffer(buff_, 1), asio::transfer_exactly(1), error) == 1)
		handle_read(error, 1, RT_SEP1);
	else
		throw invalid_argument(error.message());

	if (asio::read(socket_, asio::buffer(buff_, 1), asio::transfer_exactly(1), error) == 1)
		handle_read(error, 1, RT_TYPE);
	else
		throw invalid_argument(error.message());

	if (asio::read(socket_, asio::buffer(buff_, 2), asio::transfer_exactly(2), error) == 2)
		handle_read(error, 2, RT_WORD1);
	else
		throw invalid_argument(error.message());

	if (asio::read(socket_, asio::buffer(buff_, 2), asio::transfer_exactly(2), error) == 2)
		handle_read(error, 2, RT_DEVICE);
	else
		throw invalid_argument(error.message());

	if (asio::read(socket_, asio::buffer(buff_, 2), asio::transfer_exactly(2), error) == 2)
		handle_read(error, 2, RT_WORD2);
	else
		throw invalid_argument(error.message());

	if (asio::read(socket_, asio::buffer(buff_, 2), asio::transfer_exactly(2), error) == 2)
		handle_read(error, 2, RT_WORD3);
	else
		throw invalid_argument(error.message());

	if (asio::read(socket_, asio::buffer(buff_, 2), asio::transfer_exactly(2), error) == 2)
		handle_read(error, 2, RT_WORD4);
	else
		throw invalid_argument(error.message());

	if (asio::read(socket_, asio::buffer(buff_, 2), asio::transfer_exactly(2), error) == 2)
		handle_read(error, 2, RT_WORD5);
	else
		throw invalid_argument(error.message());

	if (asio::read(socket_, asio::buffer(buff_, 1), asio::transfer_exactly(1), error) == 1)
		handle_read(error, 1, RT_SEP2);
	else
		throw invalid_argument(error.message());

	if (asio::read(socket_, asio::buffer(buff_, 2), asio::transfer_exactly(2), error) == 2)
		handle_read(error, 2, RT_COUNT);
	else
		throw invalid_argument(error.message());

	if (asio::read(socket_, asio::buffer(buff_, 2), asio::transfer_exactly(2), error) == 2)
		handle_read(error, 2, RT_MC);
	else
		throw invalid_argument(error.message());

	// Calculate the length of the data block. This is the rest of the total length
	size_t len = (comm.hlen + 3) - 0x0015;

	if (len > BUF_SIZE)
	{
		sysl->errlogThr("AMXnet::start_read: Length to read is "+to_string(len)+" bytes, but the buffer is only " + to_string(BUF_SIZE) + " bytes!");
		return;
	}

	if (asio::read(socket_, asio::buffer(buff_, len), asio::transfer_exactly(len), error) == len)
		handle_read(error, len, RT_DATA);
	else
		throw invalid_argument(error.message());
}

#ifdef __APPLE__
void AMXNet::handle_read(const system::error_code& error, size_t n, R_TOKEN tk)
#else
void AMXNet::handle_read(const asio::error_code& error, size_t n, R_TOKEN tk)
#endif
{
	DECL_TRACTHR("handle_read(const error_code& error, size_t n, R_TOKEN tk)");

	if (!isRunning())
		return;

	if (killed)
	{
		stop();
		return;
	}

	uint32_t dw;
	int val, pos;
	size_t len;
	bool ignore = false;
	ANET_SEND s;		// Used to answer system requests
	string cmd;

	if (!error)
	{
		len = (n < BUF_SIZE) ? n : BUF_SIZE-1;
		input_buffer_.assign((char *)&buff_[0], len);

		if (len >= 8)
			sysl->DebugMsg("AMXNet::handle_read: read:\n"+NameFormat::strToHex(input_buffer_, 8, true, 26)+"\n\t\t\t\t\tToken: "+to_string(tk)+", "+to_string(len)+" bytes", true);
		else
			sysl->DebugMsg("AMXNet::handle_read: read: "+NameFormat::strToHex(input_buffer_, 1)+", Token: "+to_string(tk)+", "+to_string(len)+" bytes", true);

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
				if (protError || !isRunning())
					break;

				sysl->TRACE("AMXNet::handle_read: Received message type: 0x"+NameFormat::toHex(comm.MC, 4), true);

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
						memset(&comm.data.message_string.content[0], 0, sizeof(comm.data.message_string.content));
						len = (buff_[6] == 0x01) ? comm.data.message_string.length : comm.data.message_string.length * 2;

						if (len >= sizeof(comm.data.message_string.content))
						{
							len = sizeof(comm.data.message_string.content) - 1;
							comm.data.message_string.length = (buff_[6] == 0x01) ? len : len / 2;
						}

						memcpy(&comm.data.message_string.content[0], &buff_[9], len);
						pos = (int)(len + 10);
						comm.checksum = buff_[pos];
						cmd.assign((char *)&comm.data.message_string.content[0], len);
						sysl->DebugMsg("AMXNet::handle_read: cmd="+cmd, true);

						if (isCommand(cmd))
						{
							sysl->DebugMsg("AMXNet::handle_read: Command found!");
							oldCmd.assign(cmd);
						}
						else
						{
							oldCmd.append(cmd);
							sysl->DebugMsg("AMXNet::handle_read: Concatenated cmd="+oldCmd, true);
							memset(&comm.data.message_string.content[0], 0, sizeof(comm.data.message_string.content));
							memcpy(&comm.data.message_string.content[0], oldCmd.c_str(), sizeof(comm.data.message_string.content)-1);
							comm.data.message_string.length = oldCmd.length();
							oldCmd.clear();
						}

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
                        memset(comm.data.srDeviceInfo.info, 0, sizeof(comm.data.srDeviceInfo.info));
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

						sysl->TRACE(string("AMXNet::handle_read: S/N: ")+(char *)&comm.data.srDeviceInfo.serial[0]+" | "+(char *)&comm.data.srDeviceInfo.info[0], true);
					break;

					case 0x00a1:	// request status
						reqDevStatus = makeWord(buff_[0], buff_[1]);
						comm.checksum = buff_[2];
					break;

					case 0x0204:	// file transfer
						s.device = comm.device2;
						comm.data.filetransfer.ftype = makeWord(buff_[0], buff_[1]);
						comm.data.filetransfer.function = makeWord(buff_[2], buff_[3]);
						pos = 4;

						if (comm.data.filetransfer.ftype == 0 && comm.data.filetransfer.function == 0x0105)			// Directory exist?
						{
							for (size_t i = 0; i < 0x0104; i++)
							{
								comm.data.filetransfer.data[i] = buff_[pos];
								pos++;
							}

							comm.data.filetransfer.data[0x0103] = 0;
							handleFTransfer(s, comm.data.filetransfer);
						}
						else if (comm.data.filetransfer.ftype == 4 && comm.data.filetransfer.function == 0x0100)	// Controller have more files
							handleFTransfer(s, comm.data.filetransfer);
						else if (comm.data.filetransfer.ftype == 0 && comm.data.filetransfer.function == 0x0100)	// Request directory listing
						{
							comm.data.filetransfer.unk = makeWord(buff_[4], buff_[5]);
							pos = 6;

							for (size_t i = 0; i < 0x0104; i++)
							{
								comm.data.filetransfer.data[i] = buff_[pos];
								pos++;
							}

							comm.data.filetransfer.data[0x0103] = 0;
							handleFTransfer(s, comm.data.filetransfer);
						}
						else if (comm.data.filetransfer.ftype == 4 && comm.data.filetransfer.function == 0x0102)	// controller will send a file
						{
							comm.data.filetransfer.unk = makeDWord(buff_[4], buff_[5], buff_[6], buff_[7]);
							comm.data.filetransfer.unk1 = makeDWord(buff_[8], buff_[9], buff_[10], buff_[11]);
							pos = 12;

							for (size_t i = 0; i < 0x0104; i++)
							{
								comm.data.filetransfer.data[i] = buff_[pos];
								pos++;
							}

							comm.data.filetransfer.data[0x0103] = 0;
							handleFTransfer(s, comm.data.filetransfer);
						}
						else if (comm.data.filetransfer.ftype == 4 && comm.data.filetransfer.function == 0x0103)	// file or part of a file
						{
							comm.data.filetransfer.unk = makeWord(buff_[4], buff_[5]);
							pos = 6;

							for (size_t i = 0; i < comm.data.filetransfer.unk; i++)
							{
								comm.data.filetransfer.data[i] = buff_[pos];
								pos++;
							}

							handleFTransfer(s, comm.data.filetransfer);
						}
						else if (comm.data.filetransfer.ftype == 0 && comm.data.filetransfer.function == 0x0104)	// Does file exist;
						{
							for (size_t i = 0; i < 0x0104; i++)
							{
								comm.data.filetransfer.data[i] = buff_[pos];
								pos++;
							}

							comm.data.filetransfer.data[0x0103] = 0;
							handleFTransfer(s, comm.data.filetransfer);
						}
						else if (comm.data.filetransfer.ftype == 4 && comm.data.filetransfer.function == 0x0104)	// request a file
						{
							comm.data.filetransfer.unk = makeWord(buff_[4], buff_[5]);
							pos = 6;

							for (size_t i = 0; i < 0x0104; i++)
							{
								comm.data.filetransfer.data[i] = buff_[pos];
								pos++;
							}

							comm.data.filetransfer.data[0x0103] = 0;
							handleFTransfer(s, comm.data.filetransfer);
						}
						else if (comm.data.filetransfer.ftype == 4 && comm.data.filetransfer.function == 0x0106)	// ACK for 0x0105
						{
							comm.data.filetransfer.unk = makeDWord(buff_[4], buff_[5], buff_[6], buff_[7]);
							pos = 8;
							handleFTransfer(s, comm.data.filetransfer);
						}
						else if (comm.data.filetransfer.ftype == 4 && comm.data.filetransfer.function == 0x0002)	// request next part of file
							handleFTransfer(s, comm.data.filetransfer);
						else if (comm.data.filetransfer.ftype == 4 && comm.data.filetransfer.function == 0x0003)	// File content from controller
						{
							comm.data.filetransfer.unk = makeWord(buff_[4], buff_[5]);	// length of data block
							pos = 6;

							for (size_t i = 0; i < comm.data.filetransfer.unk; i++)
							{
								comm.data.filetransfer.data[i] = buff_[pos];
								pos++;
							}

							handleFTransfer(s, comm.data.filetransfer);
						}
						else if (comm.data.filetransfer.ftype == 4 && comm.data.filetransfer.function == 0x0004)	// End of file
							handleFTransfer(s, comm.data.filetransfer);
						else if (comm.data.filetransfer.ftype == 4 && comm.data.filetransfer.function == 0x0005)	// End of file ACK
							handleFTransfer(s, comm.data.filetransfer);
						else if (comm.data.filetransfer.ftype == 4 && comm.data.filetransfer.function == 0x0006)	// End of directory listing ACK
						{
							comm.data.filetransfer.unk = makeWord(buff_[4], buff_[5]);	// length of received data block
							pos = 6;
							handleFTransfer(s, comm.data.filetransfer);
						}
						else if (comm.data.filetransfer.ftype == 4 && comm.data.filetransfer.function == 0x0007)	// End of file transfer
							handleFTransfer(s, comm.data.filetransfer);
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

			default:		// Every unknown or unsupported command/request should be ignored.
				ignore = true;
		}
	}
	else
	{
		sysl->errlogThr(string("AMXNet::handle_read: Error on receive: ")+error.message());
		stop();
	}
}

bool AMXNet::sendCommand (const ANET_SEND& s)
{
	DECL_TRACTHR("AMXNet::sendCommand (const ANET_SEND& s)");

	bool status = false;
	size_t len, size;
	ANET_COMMAND com;
	com.clear();
	com.MC = s.MC;

	if (s.MC == 0x0204)		// file transfer
		com.device1 = s.device;
	else
		com.device1 = 0;

	com.device2 = panelID;
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

		case 0x0086:	// output channel on
		case 0x0088:	// feedback/input channel on
			com.data.channel.device = com.device2;
			com.data.channel.port = s.port;
			com.data.channel.system = com.system;
			com.data.channel.channel = s.channel;
			com.hlen = 0x0016 - 0x0003 + sizeof(ANET_CHANNEL);
			comStack.push_back(com);
			status = true;
		break;

		case 0x0087:	// output channel off
		case 0x0089:	// feedback/input channel off
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
			com.data.message_value.type = 0x20;		// unsigned integer
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

			if (s.msg.length() >= sizeof(com.data.message_string.content))
				len = sizeof(com.data.message_string.content) - 1;
			else
				len = s.msg.length();

			com.data.message_string.length = len;
			strncpy((char *)&com.data.message_string.content[0], s.msg.c_str(), len);
			com.hlen = 0x0016 - 3 + 9 + len;
			comStack.push_back(com);
			status = true;
		break;

		case 0x008d:	// Custom event
			com.data.customEvent.device = com.device2;
			com.data.customEvent.port = s.port;
			com.data.customEvent.system = com.system;
			com.data.customEvent.ID = s.ID;
			com.data.customEvent.type = s.type;
			com.data.customEvent.flag = s.flag;
			com.data.customEvent.value1 = s.value1;
			com.data.customEvent.value2 = s.value2;
			com.data.customEvent.value3 = s.value3;
			com.data.customEvent.dtype = s.dtype;

			if (s.msg.length() >= sizeof(com.data.customEvent.data))
				len = sizeof(com.data.customEvent.data) - 1;
			else
				len = s.msg.length();

			com.data.customEvent.length = len;
			memset(com.data.customEvent.data, 0, sizeof(com.data.customEvent.data));
			memcpy(&com.data.customEvent.data[0], s.msg.c_str(), s.msg.length());
			com.hlen = 0x0016 - 3 + 29 + s.msg.length();
			comStack.push_back(com);
			status = true;
		break;

		case 0x0090:		// port count
			com.data.sendPortNumber.device = com.device2;
			com.data.sendPortNumber.system = com.system;
			com.data.sendPortNumber.pcount = s.value;
			com.hlen = 0x0016 - 3 + 6;
			comStack.push_back(com);
			status = true;
		break;

		case 0x0091:		// output channel count
		case 0x0092:		// send level count
			com.data.sendOutpChannels.device = com.device2;
			com.data.sendOutpChannels.port = s.port;
			com.data.sendOutpChannels.system = com.system;
			com.data.sendOutpChannels.count = s.value;
			com.hlen = 0x0016 - 3 + 8;
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
			com.hlen = 0x0016 - 3 + 9;
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

		case 0x0204:		// File transfer
			com.port1 = 0;
			com.port2 = 0;
			com.data.filetransfer.ftype = s.dtype;
			com.data.filetransfer.function = s.type;
			com.data.filetransfer.info1 = s.value;
			com.data.filetransfer.info2 = s.level;
			com.data.filetransfer.unk = s.value1;
			com.data.filetransfer.unk1 = s.value2;
			com.data.filetransfer.unk2 = s.value3;
			size = min(s.msg.length(), sizeof(com.data.filetransfer.data)-1);
			memcpy(com.data.filetransfer.data, s.msg.c_str(), size);
			com.data.filetransfer.data[size] = 0;
			len = 4;

			if (s.dtype == 0)
			{
				switch(s.type)
				{
					case 0x0001: len += 2; break;
					case 0x0101: len += 16 + size + 1; break;
					case 0x0102: len += 19 + size + 1; break;
				}
			}
			else
			{
				switch(s.type)
				{
					case 0x0003: len += 2 + s.value1; break;
					case 0x0101: len += 8; break;
					case 0x0103: len += 6; break;
					case 0x0105: len += 8; break;
				}
			}

			com.hlen = 0x0016 - 3 + len;
            comStack.push_back(com);
			status = true;
		break;

		case 0x0581:		// Pong
			com.data.srDeviceInfo.device = panelID; // Configuration->getAMXChannel();
			com.data.srDeviceInfo.system = Configuration->getAMXSystem();
			com.data.srDeviceInfo.herstID = devInfo[0].manufacturerID;
			com.data.srDeviceInfo.deviceID = devInfo[0].deviceID;
			com.data.srDeviceInfo.info[0] = 2;	// Type: IPv4 address
			com.data.srDeviceInfo.info[1] = 4;	// length of following data

			{
				string addr = socket_.local_endpoint().address().to_string();
				vector<string> parts = Str::split(addr, '.');

				for (size_t i = 0; i < parts.size(); i++)
					com.data.srDeviceInfo.info[i+2] = (unsigned char)atoi(parts[i].c_str());
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

void AMXNet::handleFTransfer (ANET_SEND &s, ANET_FILETRANSFER &ft)
{
	DECL_TRACTHR("AMXNet::handleFTransfer (ANET_SEND &s, ANET_FILETRANSFER &ft)");

	int len;
	ANET_COMMAND ftr;
	ftr.MC = 0x1000;
	ftr.device1 = s.device;
	ftr.device2 = s.device;
	ftr.port1 = 0;
	ftr.port2 = 0;
	ftr.count = 0;
	ftr.data.filetransfer.ftype = ft.ftype;
	ftr.data.filetransfer.function = ft.function;
	ftr.data.filetransfer.data[0] = 0;

	if (ft.ftype == 0 && ft.function == 0x0105)		// Create directory
	{
		s.channel = 0;
		s.level = 0;
		s.port = 0;
		s.value = 0;
		s.MC = 0x0204;
		s.dtype = 0;				// ftype --> function type
		s.type = 0x0001;   			// function
		s.value1 = 0;				// 1st data byte 0x00
		s.value2 = 0x10;			// 2nd data byte 0x10
		string f((char *)&ft.data);
		sysl->TRACE("AMXNet::handleFTransfer: 0x0000/0x0105: Directory "+f+" exist?", true);

		if (f.compare(0, 8, "AMXPanel") == 0)
		{
			if (f.find("/images") > 0)
				mkdir(string(Configuration->getHTTProot()+"/images").c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
			else if (f.find("/sounds") > 0)
				mkdir(string(Configuration->getHTTProot()+"/sounds").c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
			else if (f.find("/fonts") > 0)
				mkdir(string(Configuration->getHTTProot()+"/fonts").c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
		}
		else if (f.compare(0, 8, "__system") == 0)
		{
			if (access(string(Configuration->getHTTProot()+"/__system").c_str(), R_OK | W_OK | X_OK))
			{
				mkdir(string(Configuration->getHTTProot()+"/__system").c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
				mkdir(string(Configuration->getHTTProot()+"/__system/graphics").c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
			}
		}

		sendCommand(s);

		if (!receiveSetup)
		{
			receiveSetup = true;
			ftransfer.maxFiles = countFiles();

			if (callback)
				callback(ftr);
		}
	}
	else if (ft.ftype == 0 && ft.function == 0x0100)	// Request directory
	{
		string fname((char *)&ft.data);
		string amxpath(fname);
		string realPath;
		size_t pos = 0;
		len = 0;
		dir::Directory dr;

		if (fname.compare("AMXPanel/") == 0)
		{
			realPath.assign(Configuration->getHTTProot());
			amxpath.assign("/opt/amx/user/AMXPanel");
		}
		else if ((pos = fname.find("AMXPanel/")) != string::npos)
		{
			if (pos == 0)
				amxpath = "/opt/amx/user/"+fname;

			realPath = dr.stripPath("AMXPanel", fname);
			realPath = Configuration->getHTTProot() + "/" + realPath;

			if (dr.isFile(realPath))
				len = dr.getFileSize(realPath);
		}

		sysl->TRACE("AMXNet::handleFTransfer: 0x0000/0x0100: Request directory "+fname, true);
		snprintf((char *)&ftr.data.filetransfer.data[0], sizeof(ftr.data.filetransfer.data), "Syncing %d files ...", ftransfer.maxFiles);

		if (callback)
			callback(ftr);

		s.channel = 0;
		s.level = 0;
		s.port = 0;
		s.value = 0;
		s.MC = 0x0204;
		s.dtype = 0x0000;
		s.type = 0x0101;
		s.value1 = len;		// File length
		s.value2 = 0x0000be42;
		s.value3 = 0x00003e75;
		s.msg = amxpath;
		sendCommand(s);
		// Read the directory tree
		dr.setStripPath(true);
		dr.readDir(realPath);
		amxpath = fname;

		if (amxpath.length() > 1 && amxpath.at(amxpath.length()-1) == '/')
			amxpath = amxpath.substr(0, amxpath.length()-1);

		for (pos = 0; pos < dr.getNumEntries(); pos++)
		{
			dir::DFILES_T df = dr.getEntry(pos);
			s.type = 0x0102;

			s.value = (dr.testDirectory(df.attr)) ? 1 : 0;	// Depends on type of entry
			s.level = dr.getNumEntries();		// # entries
			s.value1 = df.count;				// counter
			s.value2 = df.size;					// Size of file
			s.value3 = df.date;					// Last modification date (epoch)
			s.msg.assign(amxpath+"/"+df.name);
			sendCommand(s);
		}
	}
	else if (ft.ftype == 4 && ft.function == 0x0100)	// Have more files to send.
	{
		sysl->TRACE("AMXNet::handleFTransfer: 0x0004/0x0100: Have more files to send.", true);
		s.channel = 0;
		s.level = 0;
		s.port = 0;
		s.value = 0;
		s.MC = 0x0204;
		s.dtype = 4;				// ftype --> function type
		s.type = 0x0101;   			// function:
		s.value1 = 0x01bb3000;		// ?
		s.value2 = 0;				// ?
		sendCommand(s);
	}
	else if (ft.ftype == 4 && ft.function == 0x0102)	// Controller will send a file
	{
		string f((char*)&ft.data);
		size_t pos;
		rcvFileName.assign(Configuration->getHTTProot());

		if (f.find("AMXPanel") != string::npos)
		{
			pos = f.find_first_of("/");
			rcvFileName.append(f.substr(pos));
		}
		else
		{
			rcvFileName.append("/");
			rcvFileName.append((char*)&ft.data);
		}

		if (rcvFile != nullptr)
			fclose(rcvFile);

		rcvFile = fopen(rcvFileName.c_str(), "w+");

		if (!rcvFile)
		{
			sysl->errlogThr("AMXNet::handleFTransfer: Error creating file "+rcvFileName);
			isOpenRcv = false;
		}
		else
			isOpenRcv = true;

		sysl->TRACE("AMXNet::handleFTransfer: 0x0004/0x0102: Controller will send file "+rcvFileName, true);
		ftransfer.actFileNum++;
		ftransfer.lengthFile = ft.unk;

		if (ftransfer.actFileNum > ftransfer.maxFiles)
			ftransfer.maxFiles = ftransfer.actFileNum;

		ftransfer.percent = (int)(100.0 / (double)ftransfer.maxFiles * (double)ftransfer.actFileNum);
		pos = rcvFileName.find_last_of("/");
		string shfn;

		if (pos != string::npos)
			shfn = NameFormat::cp1250ToUTF8(rcvFileName.substr(pos+1));
		else
			shfn = NameFormat::cp1250ToUTF8(rcvFileName);

		snprintf((char*)&ftr.data.filetransfer.data[0], sizeof(ftr.data.filetransfer.data), "[%d/%d]&nbsp;%s", ftransfer.actFileNum, ftransfer.maxFiles, shfn.c_str());
		ftr.count = ftransfer.percent;
		ftr.data.filetransfer.info1 = 0;

		if (callback)
			callback(ftr);

		posRcv = 0;
		lenRcv = ft.unk;
		s.channel = 0;
		s.level = 0;
		s.port = 0;
		s.value = 0;
		s.MC = 0x0204;
		s.dtype = 4;				// ftype --> function type
		s.type = 0x0103;   			// function: ready for receiving file
		s.value1 = MAX_CHUNK;		// Maximum length of a chunk
		s.value2 = ft.unk1;			// ID?
		sendCommand(s);
	}
	else if (ft.ftype == 0 && ft.function == 0x0104)	// Delete file <name>
	{
		dir::Directory dr;
		s.channel = 0;
		s.level = 0;
		s.port = 0;
		s.value = 0;
		s.MC = 0x0204;
		string f((char*)&ft.data[0]);
		size_t pos = 0;

		if ((pos = f.find("AMXPanel/")) == string::npos)
			pos = f.find("__system/");

		sysl->TRACE("AMXNet::handleFTransfer: 0x0000/0x0104: Delete file "+f, true);

		if (pos != string::npos)
			f = Configuration->getHTTProot()+"/"+f.substr(pos+9);
		else
			f = Configuration->getHTTProot()+"/"+f;

		if (dr.exists(f))
		{
			s.dtype = 0;				// ftype --> function type
			s.type = 0x0002;   			// function: yes file exists
			remove(f.c_str());
		}
		else	// Send: file was deleted although it does not exist.
		{
			sysl->errlogThr("AMXNet::handleFTransfer: [DELETE] File "+f+" not found!");
			s.dtype = 0;				// ftype --> function type
			s.type = 0x0002;   			// function: yes file exists
		}
/*		else	// Following will stop the transfer immediately.
		{
			sysl->errlog("AMXNet::handleFTransfer: File "+f+" not found!");
			s.dtype = 0;				// ftype --> function type
			s.type = 1;					// function: No, file/directory does not exit --> This is an assumption!
			s.value1 = 0;				// 1st data byte 0x00
			s.value2 = 0;				// 2nd data byte 0x00 --> does not exist?
		}
*/
		sendCommand(s);

		if (ftransfer.actDelFile == 0)
		{
			ftransfer.actDelFile++;
			ftransfer.percent = (int)(100.0 / (double)ftransfer.maxFiles * (double)ftransfer.actDelFile);
			ftr.count = ftransfer.percent;

			if (callback)
				callback(ftr);
		}
		else
		{
			ftransfer.actDelFile++;
			int prc = (int)(100.0 / (double)ftransfer.maxFiles * (double)ftransfer.actDelFile);

			if (prc != ftransfer.percent)
			{
				ftransfer.percent = prc;
				ftr.count = prc;

				if (callback)
					callback(ftr);
			}
		}
	}
	else if (ft.ftype == 4 && ft.function == 0x0104)	// request a file
	{
		string f((char*)&ft.data);
		size_t pos;
		len = 0;
		sndFileName.assign(Configuration->getHTTProot());
		sysl->TRACE("AMXNet::handleFTransfer: 0x0004/0x0104: Request file "+f, true);

		if (f.find("AMXPanel") != string::npos)
		{
			pos = f.find_first_of("/");
			sndFileName.append(f.substr(pos));
		}
		else
		{
			sndFileName.append("/");
			sndFileName.append(f);
		}

		if (!access(sndFileName.c_str(), R_OK))
		{
			struct stat s;

			if (stat(sndFileName.c_str(), &s) == 0)
				len = s.st_size;
			else
				len = 0;
		}
		else if (sndFileName.find("/version.xma") > 0)
			len = 0x0015;
		else
			len = 0;

		sysl->TRACE("AMXNet::handleFTransfer: 0x0004/0x0104: ("+to_string(len)+") File: "+sndFileName, true);

		s.channel = 0;
		s.level = 0;
		s.port = 0;
		s.value = 0;
		s.MC = 0x0204;
		s.dtype = 4;				// ftype --> function type
		s.type = 0x0105;   			// function
		s.value1 = len;				// length of file to send
		s.value2 = 0x00001388;		// ID for device when sending a file.
		sendCommand(s);
	}
	else if (ft.ftype == 4 && ft.function == 0x0106)	// Controller is ready for receiving file
	{
		sysl->TRACE("AMXNet::handleFTransfer: 0x0004/0x0106: Controller is ready for receiving file.", true);

		if (!access(sndFileName.c_str(), R_OK))
		{
			struct stat st;
			stat(sndFileName.c_str(), &st);
			len = st.st_size;
			lenSnd = len;
			posSnd = 0;
			sndFile = fopen(sndFileName.c_str(), "r");

			if (!sndFile)
			{
				sysl->errlogThr("AMXNet::handleFTransfer: Error reading file "+sndFileName);
				len = 0;
				isOpenSnd = false;
			}
			else
				isOpenSnd = true;

			if (isOpenSnd && len <= MAX_CHUNK)
			{
				char *buf = new char[len+1];
				fread(buf, 1, len, sndFile);
				s.msg.assign(buf, len);
				delete[] buf;
				posSnd = len;
			}
			else if (isOpenSnd)
			{
				char *buf = new char[MAX_CHUNK+1];
				fread(buf, 1, MAX_CHUNK, sndFile);
				s.msg.assign(buf, MAX_CHUNK);
				delete[] buf;
				posSnd = MAX_CHUNK;
				len = MAX_CHUNK;
			}
		}
		else if (sndFileName.find("/version.xma") > 0)
		{
			s.msg.assign("<version>9</version>\n");
			len = s.msg.length();
			posSnd = len;
		}
		else
			len = 0;

		s.channel = 0;
		s.level = 0;
		s.port = 0;
		s.value = 0;
		s.MC = 0x0204;
		s.dtype = 4;				// ftype --> function type
		s.type = 0x0003;   			// function: Sending file with length <len>
		s.value1 = len;				// length of content to send
		sendCommand(s);
	}
	else if (ft.ftype == 4 && ft.function == 0x0002)	// request next part of file
	{
		sysl->TRACE("AMXNet::handleFTransfer: 0x0004/0x0002: Request next part of file.", true);
		s.channel = 0;
		s.level = 0;
		s.port = 0;
		s.value = 0;
		s.MC = 0x0204;
		s.dtype = 4;				// ftype --> function type

		if (posSnd < lenSnd)
		{
			s.type = 0x0003;		// Next part of file

			if ((posSnd + MAX_CHUNK) > lenSnd)
				len = lenSnd - posSnd;
			else
				len = MAX_CHUNK;

			s.value1 = len;

			if (isOpenSnd)
			{
				char *buf = new char[len+1];
				fread(buf, 1, len, sndFile);
				s.msg.assign(buf, len);
				delete[] buf;
				posSnd += len;
			}
			else
				s.value1 = 0;
		}
		else
			s.type = 0x0004;		// function: End of file reached

		sendCommand(s);
	}
	else if (ft.ftype == 4 && ft.function == 0x0003)	// File content
	{
		sysl->TRACE("AMXNet::handleFTransfer: 0x0004/0x0003: Received (part of) file.", true);
		len = ft.unk;

		if (isOpenRcv)
		{
			fwrite(ft.data, 1, len, rcvFile);
			posRcv += ft.unk;
		}
		else
			sysl->warnlogThr("AMXNet::handleFTransfer: No open file to write to!");

		s.channel = 0;
		s.level = 0;
		s.port = 0;
		s.value = 0;
		s.MC = 0x0204;
		s.dtype = 4;				// ftype --> function type
		s.type = 0x0002;   			// function: Request next part of file
		sendCommand(s);

		int prc = (int)(100.0 / (double)ftransfer.lengthFile * (double)posRcv);

		if (prc != ftr.data.filetransfer.info1)
		{
			ftr.data.filetransfer.info1 = (int)(100.0 / (double)ftransfer.lengthFile * (double)posRcv);
			ftr.count = ftransfer.percent;

			if (callback)
				callback(ftr);
		}
	}
	else if (ft.ftype == 4 && ft.function == 0x0004)	// End of file
	{
		sysl->TRACE("AMXNet::handleFTransfer: 0x0004/0x0004: End of file.", true);

		if (isOpenRcv)
		{
			unsigned char buf[8];
			fseek(rcvFile, 0, SEEK_SET);
			fread(buf, 1, sizeof(buf), rcvFile);
			fclose(rcvFile);
			isOpenRcv = false;
			rcvFile = nullptr;
			posRcv = 0;

			if (buf[0] == 0x1f && buf[1] == 0x8b)	// GNUzip compressed?
			{
				Expand exp(rcvFileName);
				exp.unzip();
			}
		}

		ftr.count = ftransfer.percent;
		ftr.data.filetransfer.info1 = 100;

		if (callback)
			callback(ftr);

		s.channel = 0;
		s.level = 0;
		s.port = 0;
		s.value = 0;
		s.MC = 0x0204;
		s.dtype = 4;				// ftype --> function type
		s.type = 0x0005;   			// function: ACK, file received. No answer expected.
		sendCommand(s);
	}
	else if (ft.ftype == 4 && ft.function == 0x0005)	// ACK, controller received file, no answer
	{
		sysl->TRACE("AMXNet::handleFTransfer: 0x0004/0x0005: Controller received file.", true);
		posSnd = 0;
		lenSnd = 0;

		if (isOpenSnd && sndFile != nullptr)
			fclose(sndFile);

		ftransfer.lengthFile = 0;
		sndFile = nullptr;
	}
	else if (ft.ftype == 4 && ft.function == 0x0006)	// End of directory transfer ACK
	{
		sysl->TRACE("AMXNet::handleFTransfer: 0x0004/0x0006: End of directory transfer.", true);
	}
	else if (ft.ftype == 4 && ft.function == 0x0007)	// End of file transfer
	{
		sysl->TRACE("AMXNet::handleFTransfer: 0x0004/0x0007: End of file transfer.", true);

		if (callback)
			callback(ftr);

		receiveSetup = false;
	}
}

int AMXNet::msg97fill(ANET_COMMAND *com)
{
	DECL_TRACTHR("AMXNet::msg97fill(ANET_COMMAND *com)");

	int pos = 0;
	unsigned char buf[512];

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
		pos = (int)strlen(devInfo[i].versionInfo) + 1;
		memcpy(buf+pos, devInfo[i].deviceInfo, strlen(devInfo[i].deviceInfo));
		pos += strlen(devInfo[i].deviceInfo) + 1;
		memcpy(buf+pos, devInfo[i].manufacturerInfo, strlen(devInfo[i].manufacturerInfo));
		pos += strlen(devInfo[i].manufacturerInfo)+1;
		*(buf+pos) = 0x02;	// type IP address
		pos++;
		*(buf+pos) = 0x04;	// field length: 4 bytes
		// Now the IP Address
		string addr = socket_.local_endpoint().address().to_string();
		vector<string> parts = Str::split(addr, '.');

		for (size_t i = 0; i < parts.size(); i++)
		{
			pos++;
			*(buf+pos) = (unsigned char)atoi(parts[i].c_str());
		}

		pos++;
		com->data.srDeviceInfo.len = pos;
		memcpy(com->data.srDeviceInfo.info, buf, pos);
		com->hlen = 0x0016 - 3 + 31 + pos - 1;
		comStack.push_back(*com);
		sendCounter++;
		com->count = sendCounter;
	}

	return pos;
}

void AMXNet::start_write()
{
	DECL_TRACTHR("AMXNet::start_write()");

	if (!isRunning())
		return;

	if (write_busy)
		return;

	write_busy = true;

	while (comStack.size() > 0)
	{
		if (!isRunning())
		{
			comStack.clear();
			write_busy = false;
			return;
		}

		send = comStack.at(0);
		comStack.erase(comStack.begin());	// delete oldest element
		unsigned char *buf = makeBuffer(send);

		if (buf == 0)
		{
			sysl->errlogThr("AMXNet::start_write: Error creating a buffer! Token number: "+to_string(send.MC));
			continue;
		}

		asio::async_write(socket_, asio::buffer(buf, send.hlen + 4), bind(&AMXNet::handle_write, this, _1));
		delete[] buf;
	}

	write_busy = false;
}

void AMXNet::handle_write(const error_code& error)
{
	DECL_TRACTHR("AMXNet::handle_write(const error_code& error)");

	if (!isRunning())
		return;

	if (!error)
	{
		while (comStack.size() == 0)
			heartbeat_timer_.expires_after(chrono::microseconds(150));

		heartbeat_timer_.async_wait(bind(&AMXNet::start_write, this));
	}
	else
	{
		sysl->errlogThr("AMXNet::handle_write: Error on heartbeat: "+error.message());
		stop();
	}
}

void AMXNet::check_deadline()
{
	DECL_TRACTHR("AMXNet::check_deadline()");

	if (!isRunning())
		return;

	// Check whether the deadline has passed. We compare the deadline against
	// the current time since a new asynchronous operation may have moved the
	// deadline before this actor had a chance to run.
	if (deadline_.expiry() <= steady_timer::clock_type::now())
	{
		// The deadline has passed. The socket is closed so that any outstanding
		// asynchronous operations are cancelled.
		if (socket_.is_open())
			socket_.close();

		// There is no longer an active deadline. The expiry is set to the
		// maximum time point so that the actor takes no action until a new
		// deadline is set.
		deadline_.expires_at(steady_timer::time_point::max());
	}

	// Put the actor back to sleep.
	deadline_.async_wait(bind(&AMXNet::check_deadline, this));
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
	DECL_TRACTHR("AMXNet::calcChecksum(const unsigned char* buffer, size_t len)");
	unsigned long sum = 0;

	for (size_t i = 0; i < len; i++)
		sum += (unsigned long)(*(buffer+i)) & 0x000000ff;

	sum &= 0x000000ff;
	sysl->TRACE("AMXNet::calcChecksum: Checksum="+NameFormat::toHex((int)sum, 2)+", #bytes="+to_string(len)+" bytes.", true);
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

bool AMXNet::isCommand(const string& cmd)
{
	DECL_TRACTHR("AMXNet::isCommand(string& cmd)");

	int i = 0;

	while (cmdList[i][0] != 0)
	{
		if (cmd.find(cmdList[i]) == 0)
			return true;

		i++;
	}

	return false;
}

unsigned char *AMXNet::makeBuffer (const ANET_COMMAND& s)
{
	DECL_TRACTHR("AMXNet::makeBuffer (const ANET_COMMAND& s)");

	int pos = 0;
	int len;
	bool valid = false;
	unsigned char *buf;

	try
	{
		buf = new unsigned char[s.hlen+5];
		memset(buf, 0, s.hlen+5);
	}
	catch(std::exception& e)
	{
		sysl->errlogThr(string("AMXNet::makeBuffer: Error allocating memory: ")+e.what());
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

		case 0x008d:	// Custom event
			*(buf+22) = s.data.customEvent.device >> 8;
			*(buf+23) = s.data.customEvent.device;
			*(buf+24) = s.data.customEvent.port >> 8;
			*(buf+25) = s.data.customEvent.port;
			*(buf+26) = s.data.customEvent.system >> 8;
			*(buf+27) = s.data.customEvent.system;
			*(buf+28) = s.data.customEvent.ID >> 8;
			*(buf+29) = s.data.customEvent.ID;
			*(buf+30) = s.data.customEvent.type >> 8;
			*(buf+31) = s.data.customEvent.type;
			*(buf+32) = s.data.customEvent.flag >> 8;
			*(buf+33) = s.data.customEvent.flag;
			*(buf+34) = s.data.customEvent.value1 >> 24;
			*(buf+35) = s.data.customEvent.value1 >> 16;
			*(buf+36) = s.data.customEvent.value1 >> 8;
			*(buf+37) = s.data.customEvent.value1;
			*(buf+38) = s.data.customEvent.value2 >> 24;
			*(buf+39) = s.data.customEvent.value2 >> 16;
			*(buf+40) = s.data.customEvent.value2 >> 8;
			*(buf+41) = s.data.customEvent.value2;
			*(buf+42) = s.data.customEvent.value3 >> 24;
			*(buf+43) = s.data.customEvent.value3 >> 16;
			*(buf+44) = s.data.customEvent.value3 >> 8;
			*(buf+45) = s.data.customEvent.value3;
			*(buf+46) = s.data.customEvent.dtype;
			*(buf+47) = s.data.customEvent.length >> 8;
			*(buf+48) = s.data.customEvent.length;
			pos = 49;

			if (s.data.customEvent.length > 0)
			{
				memcpy(buf+pos, s.data.customEvent.data, s.data.customEvent.length);
				pos += s.data.customEvent.length;
			}

			*(buf+pos) = 0;
			*(buf+pos+1) = 0;
			pos += 2;
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
			memset((void*)&s.data.sendStatusCode.str[0], 0, sizeof(s.data.sendStatusCode.str));
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

		case 0x0204:	// file transfer
			*(buf+22) = s.data.filetransfer.ftype >> 8;
			*(buf+23) = s.data.filetransfer.ftype;
			*(buf+24) = s.data.filetransfer.function >> 8;
			*(buf+25) = s.data.filetransfer.function;
			pos = 26;

			switch(s.data.filetransfer.function)
			{
				case 0x0001:
					*(buf+26) = s.data.filetransfer.unk;
					*(buf+27) = s.data.filetransfer.unk1;
					pos = 28;
				break;

				case 0x0003:
					*(buf+26) = s.data.filetransfer.unk >> 8;
					*(buf+27) = s.data.filetransfer.unk;
					pos = 28;

					for (uint32_t i = 0; i < s.data.filetransfer.unk && pos < (s.hlen+3); i++)
					{
						*(buf+pos) = s.data.filetransfer.data[i];
						pos++;
					}
				break;

				case 0x0101:
					if (s.data.filetransfer.ftype == 0)
					{
						*(buf+26) = s.data.filetransfer.unk >> 24;
						*(buf+27) = s.data.filetransfer.unk >> 16;
						*(buf+28) = s.data.filetransfer.unk >> 8;
						*(buf+29) = s.data.filetransfer.unk;
						*(buf+30) = s.data.filetransfer.unk1 >> 24;
						*(buf+31) = s.data.filetransfer.unk1 >> 16;
						*(buf+32) = s.data.filetransfer.unk1 >> 8;
						*(buf+33) = s.data.filetransfer.unk1;
						*(buf+34) = s.data.filetransfer.unk2 >> 24;
						*(buf+35) = s.data.filetransfer.unk2 >> 16;
						*(buf+36) = s.data.filetransfer.unk2 >> 8;
						*(buf+37) = s.data.filetransfer.unk2;
						*(buf+38) = 0x00;
						*(buf+39) = 0x00;
						*(buf+40) = 0x3e;
						*(buf+41) = 0x75;
						pos = 42;
						len = 0;

						while (s.data.filetransfer.data[len] != 0)
						{
							*(buf+pos) = s.data.filetransfer.data[len];
							len++;
							pos++;
						}

						*(buf+pos) = 0;
						pos++;
					}
					else
					{
						*(buf+26) = s.data.filetransfer.unk >> 24;
						*(buf+27) = s.data.filetransfer.unk >> 16;
						*(buf+28) = s.data.filetransfer.unk >> 8;
						*(buf+29) = s.data.filetransfer.unk;
						*(buf+30) = 0x00;
						*(buf+31) = 0x00;
						*(buf+32) = 0x00;
						*(buf+33) = 0x00;
						pos = 34;
					}
				break;

				case 0x0102:
					*(buf+26) = 0x00;
					*(buf+27) = 0x00;
					*(buf+28) = 0x00;
					*(buf+29) = s.data.filetransfer.info1;			// dir flag
					*(buf+30) = s.data.filetransfer.info2 >> 8;		// # entries
					*(buf+31) = s.data.filetransfer.info2;
					*(buf+32) = s.data.filetransfer.unk >> 8;		// counter
					*(buf+33) = s.data.filetransfer.unk;
					*(buf+34) = s.data.filetransfer.unk1 >> 24;		// file size
					*(buf+35) = s.data.filetransfer.unk1 >> 16;
					*(buf+36) = s.data.filetransfer.unk1 >> 8;
					*(buf+37) = s.data.filetransfer.unk1;
					*(buf+38) = (s.data.filetransfer.info1 == 1) ? 0x0c : 0x0b;
					*(buf+39) = (s.data.filetransfer.info1 == 1) ? 0x0e : 0x13;
					*(buf+40) = 0x07;
					*(buf+41) = s.data.filetransfer.unk2 >> 24;		// Date
					*(buf+42) = s.data.filetransfer.unk2 >> 16;
					*(buf+43) = s.data.filetransfer.unk2 >> 8;
					*(buf+44) = s.data.filetransfer.unk2;
					pos = 45;
					len = 0;

					while (s.data.filetransfer.data[len] != 0)
					{
						*(buf+pos) = s.data.filetransfer.data[len];
						pos++;
						len++;
					}

					*(buf+pos) = 0;
					pos++;
				break;

				case 0x0103:
					*(buf+26) = s.data.filetransfer.unk >> 8;
					*(buf+27) = s.data.filetransfer.unk;
					*(buf+28) = s.data.filetransfer.unk1 >> 24;
					*(buf+29) = s.data.filetransfer.unk1 >> 16;
					*(buf+30) = s.data.filetransfer.unk1 >> 8;
					*(buf+31) = s.data.filetransfer.unk1;
					pos = 32;
				break;

				case 0x105:
					*(buf+26) = s.data.filetransfer.unk >> 24;
					*(buf+27) = s.data.filetransfer.unk >> 16;
					*(buf+28) = s.data.filetransfer.unk >> 8;
					*(buf+29) = s.data.filetransfer.unk;
					*(buf+30) = s.data.filetransfer.unk1 >> 24;
					*(buf+31) = s.data.filetransfer.unk1 >> 16;
					*(buf+32) = s.data.filetransfer.unk1 >> 8;
					*(buf+33) = s.data.filetransfer.unk1;
					pos = 34;
				break;
			}

			*(buf+pos) = calcChecksum(buf, pos);
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

	string b((char *)buf, s.hlen+4);
	sysl->TRACE("AMXNet::makeBuffer:\n"+NameFormat::strToHex(b, 8, true, 26), true);
	return buf;
}

void AMXNet::setSerialNum(const string& sn)
{
	DECL_TRACER("AMXNet::setSerialNum(const string& sn)");

	serNum = sn;
	size_t len = (sn.length() > 15) ? 15 : sn.length();

	for (size_t i = 0; i < devInfo.size(); i++)
		memcpy(devInfo[i].serialNum, sn.c_str(), len);
}

int AMXNet::countFiles()
{
	DECL_TRACTHR("AMXNet::countFiles()");

	int count = 0;
	ifstream in;

	try
	{
		in.open(Configuration->getHTTProot()+"/manifest.xma", fstream::in);

		if (!in)
			return 0;

		for (string line; getline(in, line);)
			count++;

		in.close();
	}
	catch(exception& e)
	{
		sysl->errlogThr(string("AMXNet::countFiles: ")+e.what());
		return 0;
	}

	return count;
}
