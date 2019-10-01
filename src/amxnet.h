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

#ifndef __AMXNET_H__
#define __AMXNET_H__

#include <functional>
#include <cstring>
#include <cstdio>
#include <atomic>

#ifdef __APPLE__
using namespace boost;
#endif

extern std::atomic<bool> killed;

namespace amx
{
	#define MAX_CHUNK	0x07d0	// Maximum size a part of a file can have. The file will be splitted into this size of chunks.
	#define BUF_SIZE	0x1000	// 4096 bytes

	typedef struct
	{
		uint16_t device{0};		// Device ID of panel
		uint16_t MC{0};			// message command number
		uint16_t port{0};		// port number
		uint16_t level{0};		// level number (if any)
		uint16_t channel{0};	// channel status
		uint16_t value{0};		// level value
		// this is for custom events
		uint16_t ID{0};			// ID of button
		uint16_t type{0};		// Type of event
		uint16_t flag{0};		// Flag
		uint32_t value1{0};		// Value 1
		uint32_t value2{0};		// Value 2
		uint32_t value3{0};		// Value 3
		unsigned char dtype{0};	// Type of data
		std::string msg;		// message string
	}ANET_SEND;

	typedef union
	{
		unsigned char byte;	// type = 0x10
		char ch;			// type = 0x11
		uint16_t integer;	// type = 0x20 (also wide char)
		int16_t sinteger;	// type = 0x21
		uint32_t dword;		// type = 0x40
		int32_t sdword;		// type = 0x41
		float fvalue;		// type = 0x4f
		double dvalue;		// type = 0x8f
	}ANET_CONTENT;

	typedef struct ANET_MSG
	{
		uint16_t device;		// device number
		uint16_t port;			// port number
		uint16_t system;		// system number
		uint16_t value;			// value of command
		unsigned char type;		// defines how to interpret the content of cmd
		ANET_CONTENT content;
	}ANET_MSG;

	typedef struct ANET_MSG_STRING
	{
		uint16_t device;		// device number
		uint16_t port;			// port number
		uint16_t system;		// system number
		unsigned char type;		// Definnes the type of content (0x01 = 8 bit chars, 0x02 = 16 bit chars --> wide chars)
		uint16_t length;		// length of following content
		unsigned char content[1500];// content string
	}ANET_MSG_STRING;

	typedef struct ANET_ASIZE
	{
		uint16_t device;		// device number
		uint16_t port;			// port number
		uint16_t system;		// system number
		unsigned char type;		// Defines the type of content (0x01 = 8 bit chars, 0x02 = 16 bit chars --> wide chars)
		uint16_t length;		// length of following content
	}ANET_ASIZE;

	typedef struct ANET_LEVSUPPORT
	{
		uint16_t device;		// device number
		uint16_t port;			// port number
		uint16_t system;		// system number
		uint16_t level;			// level number
		unsigned char num;		// number of supported types
		unsigned char types[6];	// Type codes
	}ANET_LEVSUPPORT;

	typedef struct ANET_ASTATCODE
	{
		uint16_t device;		// device number
		uint16_t port;			// port number
		uint16_t system;		// system number
		uint16_t status;		// status code
		unsigned char type;		// defines how to interpret the content of cmd
		uint16_t length;		// length of following string
		unsigned char str[512];
	}ANET_ASTATCODE;

	typedef struct ANET_LEVEL
	{
		uint16_t device;		// device number
		uint16_t port;			// port number
		uint16_t system;		// system number
		uint16_t level;			// level number
	}ANET_LEVEL;

	typedef struct ANET_CHANNEL
	{
		uint16_t device;		// device number
		uint16_t port;			// port number
		uint16_t system;		// system number
		uint16_t channel;		// level number
	}ANET_CHANNEL;

	typedef struct ANET_RPCOUNT
	{
		uint16_t device;		// device number
		uint16_t system;		// system number
	}ANET_RPCOUNT;

	typedef struct ANET_APCOUNT	// Answer to request for port count
	{
		uint16_t device;		// device number
		uint16_t system;		// system number
		uint16_t pcount;		// number of supported ports
	}ANET_APCOUNT;

	typedef struct ANET_ROUTCHAN	// Request for port count
	{
		uint16_t device;		// device number
		uint16_t port;			// system number
		uint16_t system;		// number of supported ports
	}ANET_ROUTCHAN;

	typedef struct ANET_AOUTCHAN	// Answer to request for port count
	{
		uint16_t device;		// device number
		uint16_t port;			// system number
		uint16_t system;		// number of supported ports
		uint16_t count;			// number of supported channels
	}ANET_AOUTCHAN;

	typedef struct ANET_ADEVINFO // Answer to "request device info"
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
		unsigned char info[512];// several NULL terminated informations
		int len;				// length of field info
	}ANET_ADEVINFO;

	typedef struct ANET_ASTATUS	// Answer to "master status"
	{
		uint16_t system;		// number of system
		uint16_t status;		// Bit field
		unsigned char str[512];	// Null terminated status string
	}ANET_ASTATUS;

	typedef struct ANET_CUSTOM	// Custom event
	{
		uint16_t device;		// Device number
		uint16_t port;			// Port number
		uint16_t system;		// System number
		uint16_t ID;			// ID of event (button ID)
		uint16_t type;			// Type of event
		uint16_t flag;			// Flag
		uint32_t value1;		// Value 1
		uint32_t value2;		// Value 2
		uint32_t value3;		// Value 3
		unsigned char dtype;	// type of following data
		uint16_t length;		// length of following string
		unsigned char data[255];// Custom data
	}ANET_CUSTOM;

	typedef struct ANET_FILETRANSFER	// File transfer
	{
		uint16_t ftype;			// 0 = not used, 1=IR, 2=Firmware, 3=TP file, 4=Axcess2
		uint16_t function;		// The function to be performed, or length of data block
		uint16_t info1;
        uint16_t info2;
		uint32_t unk;			// ?
		uint32_t unk1;			// ?
		uint32_t unk2;			// ?
		uint32_t unk3;			// ?
		unsigned char data[2048];// Function specific data
	}ANET_FILETRANSFER;

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
		ANET_LEVSUPPORT sendLevSupport;
		ANET_ADEVINFO srDeviceInfo;		// send/receive device info
		ANET_CUSTOM customEvent;
		ANET_FILETRANSFER filetransfer;
	}ANET_DATA;

	typedef struct ANET_COMMAND	// Structure of type command (type = 0x00, status = 0x000c)
	{
		char ID{2};				// 0x00:        Always 0x02
		uint16_t hlen{0};		// 0x01 - 0x02: Header length (length + 3 for total length!)
		char sep1{2};			// 0x03:        Seperator always 0x02
		char type{0};			// 0x04:        Type of header
		uint16_t unk1{1};		// 0x05 - 0x06: always 0x0001
		uint16_t device1{0};	// 0x07 - 0x08: receive: device, send: 0x000
		uint16_t port1{0};		// 0x09 - 0x0a: receive: port,   send: 0x001
		uint16_t system{0};		// 0x0b - 0x0c: receive: system, send: 0x001
		uint16_t device2{0};	// 0x0d - 0x0e: send: device,    receive: 0x0000
		uint16_t port2{0};		// 0x0f - 0x10: send: port,      receive: 0x0001
		char unk6{0x0f};		// 0x11:        Always 0x0f
		uint16_t count{0};		// 0x12 - 0x13: Counter
		uint16_t MC{0};			// 0x14 - 0x15: Message command identifier
		ANET_DATA data;			// 0x16 - n     Data block
		unsigned char checksum{0};	// last byte:   Checksum

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
			checksum = 0;
		}
	}ANET_COMMAND;

	typedef struct
	{
		unsigned char objectID;		// Unique 8-bit identifier that identifies this structure of information
		unsigned char parentID;		// Refers to an existing object ID. If 0, has this object to any parent object (parent).
		uint16_t manufacturerID;	// Value that uniquely identifies the manufacture of the device.
		uint16_t deviceID;			// Value that uniquely identifies the device type.
		char serialNum[16];			// Fixed length of 16 bytes.
		uint16_t firmwareID;		// Value that uniquely identifies the object code that the device requires.
		// NULL terminated text field
		char versionInfo[16];		// A closed with NULL text string that specifies the version of the reprogrammable component.
		char deviceInfo[32];		// A closed with NULL text string that specifies the name or model number of the device.
		char manufacturerInfo[32];	// A closed with NULL text string that specifies the name of the device manufacturer.
		unsigned char format;		// Value that indicates the type of device specific addressing information following.
		unsigned char len;			// Value that indicates the length of the following device-specific addressing information.
		unsigned char addr[8];		// Extended address as indicated by the type and length of the extended address.
	}DEVICE_INFO;

	typedef struct FTRANSFER
	{
		int percent{0};				// Status indicating the percent done
		int maxFiles{0};			// Total available files
		int lengthFile{0};			// Total length of currently transfered file
		int actFileNum{0};			// Number of currently transfered file.
		int actDelFile{0};			// Number of currently deleted file.
	}FTRANSFER;

	class AMXNet
	{
		public:
			AMXNet();
			explicit AMXNet(const std::string& sn);
			explicit AMXNet(const std::string& sn, const std::string& nm);
			~AMXNet();

			void Run();
			void start(asio::ip::tcp::resolver::results_type endpoints, int id);
			void stop();

			void setCallback(std::function<void(const ANET_COMMAND&)> func) { callback = func; }
			bool sendCommand(const ANET_SEND& s);
			bool isConnected();
			bool isStopped() { return stopped_; }
			void setPanelID(int id) { panelID = id; }
			void setSerialNum(const std::string& sn);
			asio::ip::tcp::socket& getSocket() { return socket_; }
			bool setupStatus() { return receiveSetup; }
			void setPanName(const std::string& nm) { panName.assign(nm); }

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
#ifdef __APPLE__
			void handle_read(const system::error_code& error, size_t n, R_TOKEN tk);
#else
			void handle_read(const asio::error_code& error, size_t n, R_TOKEN tk);
#endif
			void start_write();
			void handle_write(const std::error_code& error);
			void handleFTransfer(ANET_SEND& s, ANET_FILETRANSFER& ft);
			void check_deadline();
			uint16_t swapWord(uint16_t w);
			uint32_t swapDWord(uint32_t dw);
			unsigned char calcChecksum(const unsigned char *buffer, size_t len);
			uint16_t makeWord(unsigned char b1, unsigned char b2);
			uint32_t makeDWord(unsigned char b1, unsigned char b2, unsigned char b3, unsigned char b4);
			unsigned char *makeBuffer(const ANET_COMMAND& s);
			int msg97fill(ANET_COMMAND *com);
			bool isCommand(const std::string& cmd);
			bool isRunning() { return !(stopped_ || killed); }
			int countFiles();

			asio::io_context io_context;
			asio::steady_timer deadline_;
			asio::steady_timer heartbeat_timer_;

			bool stopped_{false};
			asio::ip::tcp::resolver::results_type endpoints_;
			asio::ip::tcp::socket socket_;
			std::string input_buffer_;
			unsigned char buff_[BUF_SIZE];
			std::function<void(const ANET_COMMAND&)> callback;
			std::function<bool(AMXNet *)> cbWebConn;
			std::string panName;		// The technical name of the panel
			bool protError{false};		// true = error on receive --> disconnect
			std::atomic<int> reconCounter{0};	// Reconnect counter
			uint16_t reqDevStatus{0};
			ANET_COMMAND comm;			// received command
			ANET_COMMAND send;			// answer / request
			uint16_t sendCounter{0};	// Counter increment on every send
			std::vector<ANET_COMMAND> comStack;	// commands to answer
			bool initSend{false};		// TRUE = all init messages are send.
			bool ready{false};			// TRUE = ready for communication
			bool write_busy{false};
			std::vector<DEVICE_INFO> devInfo;
			std::string oldCmd;
			int panelID{0};				// Panel ID of currently legalized panel.
			std::string serNum;
			std::atomic<bool> receiveSetup{false};
			std::string sndFileName;
			std::string rcvFileName;
			FILE *rcvFile{nullptr};
			FILE *sndFile{nullptr};
			bool isOpenSnd{false};
			bool isOpenRcv{false};
			size_t posRcv{0};
			size_t lenRcv{0};
			size_t posSnd{0};
			size_t lenSnd{0};
			FTRANSFER ftransfer;
	};
}

#endif
