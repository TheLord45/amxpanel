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

#ifndef __AMXNET_H__
#define __AMXNET_H__

#include <functional>
#include <cstring>

#ifdef __APPLE__
using namespace boost;
#endif

namespace amx
{
	typedef struct
	{
		uint16_t MC;			// message command number
		uint16_t port;			// port number
		uint16_t level;			// level number (if any)
		bool channel;			// channel status
		uint16_t value;			// level value
		strings::String msg;	// message string
	}ANET_SEND;

	typedef union
	{
		unsigned char byte;	// type = 0x10
		char ch;			// type = 0x11
		uint16_t integer;	// type = 0x20 (also wide char)
		int16_t sinteger;	// type = 0x21
		uint32_t dword;		// type = 0x40
		int32_t sdword;		// type = 0x41
		float_t fvalue;		// type = 0x4f
		double_t dvalue;	// type = 0x8f
	}ANET_CONTENT;

	typedef struct
	{
		uint16_t device;		// device number
		uint16_t port;			// port number
		uint16_t system;		// system number
		uint16_t value;			// value of command
		unsigned char type;		// defines how to interpret the content of cmd
		ANET_CONTENT content;
	}ANET_MSG;

	typedef struct
	{
		uint16_t device;		// device number
		uint16_t port;			// port number
		uint16_t system;		// system number
		unsigned char type;		// Definnes the type of content (0x01 = 8 bit chars, 0x02 = 16 bit chars --> wide chars)
		uint16_t length;		// length of following content
		unsigned char content[1500];// content string
	}ANET_MSG_STRING;

	typedef struct
	{
		uint16_t device;		// device number
		uint16_t port;			// port number
		uint16_t system;		// system number
		unsigned char type;		// Definnes the type of content (0x01 = 8 bit chars, 0x02 = 16 bit chars --> wide chars)
		uint16_t length;		// length of following content
	}ANET_ASIZE;

	typedef struct
	{
		uint16_t device;		// device number
		uint16_t port;			// port number
		uint16_t system;		// system number
		uint16_t status;		// status code
		unsigned char type;		// defines how to interpret the content of cmd
		uint16_t length;		// length of following string
		unsigned char str[512];
	}ANET_ASTATCODE;

	typedef struct
	{
		uint16_t device;		// device number
		uint16_t port;			// port number
		uint16_t system;		// system number
		uint16_t level;			// level number
	}ANET_LEVEL;

	typedef struct
	{
		uint16_t device;		// device number
		uint16_t port;			// port number
		uint16_t system;		// system number
		uint16_t channel;		// level number
	}ANET_CHANNEL;

	typedef struct
	{
		uint16_t device;		// device number
		uint16_t system;		// system number
	}ANET_RPCOUNT;

	typedef struct				// Answer to request for port count
	{
		uint16_t device;		// device number
		uint16_t system;		// system number
		uint16_t pcount;		// number of supported ports
	}ANET_APCOUNT;

	typedef struct				// Request for port count
	{
		uint16_t device;		// device number
		uint16_t port;			// system number
		uint16_t system;		// number of supported ports
	}ANET_ROUTCHAN;

	typedef struct				// Answer to request for port count
	{
		uint16_t device;		// device number
		uint16_t port;			// system number
		uint16_t system;		// number of supported ports
		uint16_t count;			// number of supported channels
	}ANET_AOUTCHAN;

	typedef struct				// Answer to "request device info"
	{
		uint16_t device;		// device number
		uint16_t system;		// system number
		uint16_t flag;			// Bit 8 - If set, this message was generated in response to a key press, during the identification mode is active.
		unsigned char objectID;	// object ID
		unsigned char parentID;	// parent ID
		uint16_t herstID;		// herst ID
		uint16_t deviceID;		// device ID
		unsigned char serial[16]; // serial number
		uint16_t fwid;			// firmware ID
		unsigned char info[256];// several NULL terminated informations
	}ANET_ADEVINFO;

	typedef struct				// Answer to "master status"
	{
		uint16_t system;		// number of system
		uint16_t status;		// Bit field
		unsigned char str[256];	// Null terminated status string
	}ANET_ASTATUS;

	typedef union
	{
		ANET_CHANNEL chan_state;
		ANET_MSG message_value;
		ANET_MSG_STRING message_string;
		ANET_LEVEL level;
		ANET_CHANNEL channel;
		ANET_RPCOUNT reqPortCount;
		ANET_APCOUNT sendPortNumber;
		ANET_ROUTCHAN reqOutpChannels;
		ANET_AOUTCHAN sendOutpChannels;
		ANET_ASTATCODE sendStatusCode;
		ANET_ASIZE sendSize;
		ANET_LEVEL reqLevels;
		ANET_ADEVINFO srDeviceInfo;		// send/receive device info
	}ANET_DATA;

	typedef struct ANET_COMMAND	// Structure of type command (type = 0x00, status = 0x000c)
	{
		char ID;				// 0x00:        Always 0x02
		uint16_t hlen;			// 0x01 - 0x02: Header length (length + 3 for total length!)
		char sep1;				// 0x03:        Seperator always 0x02
		char type;				// 0x04:        Type of header
		uint16_t unk1;			// 0x05 - 0x06: always 0x001
		uint16_t device1;		// 0x07 - 0x08: receive: device, send: 0x000
		uint16_t port1;			// 0x09 - 0x0a: receive: port,   send: 0x001
		uint16_t system;		// 0x0b - 0x0c: receive: system, send: 0x001
		uint16_t device2;		// 0x0d - 0x0e: send: device,    receive: 0x0000
		uint16_t port2;			// 0x0f - 0x10: send: port,      receive: 0x0001
		char unk6;				// 0x11:        Always 0x0f
		uint16_t count;			// 0x12 - 0x13: Counter
		uint16_t MC;			// 0x14 - 0x15: Message command identifier
		ANET_DATA data;			// 0x16 - n     Data block
		unsigned char checksum;	// last byte:   Checksum

		void clear()
		{
			ID = 0x02;
			hlen = 0;
			sep1 = 0x02;
			type = 0;
			unk1 = 1;
			device1 = 0;
			port1 = 0;
			system = 0;
			device2 = 0;
			port2 = 0;
			unk6 = 0x0f;
			count = 0;
			MC = 0;
			memset(&data, 0, sizeof(ANET_DATA));
			checksum = 0;
		}
	}ANET_COMMAND;

	class AMXNet
	{
		public:
			AMXNet(asio::io_context& io_context);
			~AMXNet();

			void start(asio::ip::tcp::resolver::results_type endpoints);
			void stop();

			void setCallback(std::function<void(const ANET_COMMAND&)> func) { callback = func; }
			bool sendCommand(const ANET_SEND& s);

		private:
			enum R_TOKEN
			{
				RT_NONE,
				RT_ID,
				RT_LEN,
				RT_SEP1,
				RT_TYPE,
				RT_WORD1,
				RT_DEVICE,
				RT_WORD2,
				RT_WORD3,
				RT_WORD4,
				RT_WORD5,
				RT_SEP2,
				RT_COUNT,
				RT_MC,
				RT_DATA
			};

			void init();
			void start_connect(asio::ip::tcp::resolver::results_type::iterator endpoint_iter);
			void handle_connect(const std::error_code& error, asio::ip::tcp::resolver::results_type::iterator endpoint_iter);
			void start_read();
			void handle_read(const asio::error_code& error, size_t n, R_TOKEN tk);
			void start_write();
			void handle_write(const std::error_code& error);
			void check_deadline();
			uint16_t swapWord(uint16_t w);
			uint32_t swapDWord(uint32_t dw);
			unsigned char calcChecksum(const unsigned char *buffer, size_t len);
			uint16_t makeWord(unsigned char b1, unsigned char b2);
			uint32_t makeDWord(unsigned char b1, unsigned char b2, unsigned char b3, unsigned char b4);
			unsigned char *makeBuffer(const ANET_COMMAND& s);

			bool stopped_ = false;
			asio::ip::tcp::resolver::results_type endpoints_;
			asio::ip::tcp::socket socket_;
			strings::String input_buffer_;
			unsigned char buff_[2048];
			asio::steady_timer deadline_;
			asio::steady_timer heartbeat_timer_;
			std::function<void(const ANET_COMMAND&)> callback;
			bool protError;				// true = error on receive --> disconnect
			uint16_t reqDevStatus;
			ANET_COMMAND comm;				// received command
			ANET_COMMAND send;				// answer / request
			uint16_t sendCounter;		// Counter increment on every send
			std::vector<ANET_COMMAND> comStack;	// commands to answer
			strings::String serial;
			strings::String version;
			strings::String manufacturer;
			bool write_busy;
			bool identified;
	};
}

#endif
