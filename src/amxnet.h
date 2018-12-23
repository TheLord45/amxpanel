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
#include "touchpanel.h"

namespace amx
{
	class AMXNet
	{
		public:
			AMXNet(asio::io_context& io_context);
			~AMXNet();

			void start(asio::ip::tcp::resolver::results_type endpoints);
			void stop();

			void setCallback(std::function<void(const strings::String&)> func) { callback = func; }

		protected:
			// Channel on or off from master or device
			typedef struct CHAN_ONOFF
			{
				uint16_t MC;			// type of content
				uint16_t device;		// device number
				uint16_t port;			// port number
				uint16_t system;		// system number
				uint16_t channel;		// channel number
			}CHAN_ONOFF;

			typedef struct MSG
			{
				uint16_t MC;			// type of content
				uint16_t device;		// device number
				uint16_t port;			// port number
				uint16_t system;		// system number
				uint16_t value;			// value of command
				unsigned char type;		// defines how to interpret the content of cmd
				typedef union
				{
					unsigned char byte;	// type = 0x10
					char ch;			// type = 0x11
					uint16_t ineteger;	// type = 0x20 (also wide char)
					int16_t sinteger;	// type = 0x21
					uint32_t dword;		// type = 0x40
					int32_t sdword;		// type = 0x41
					float_t fvalue;		// type = 0x4f
					double_t dvalue;	// type = 0x8f
				}content;
			}MSG;

			typedef struct MSG_STRING
			{
				uint16_t MC;			// type of content
				uint16_t device;		// device number
				uint16_t port;			// port number
				uint16_t system;		// system number
				unsigned char type;		// Definnes the type of content (0x01 = 8 bit chars, 0x02 = 16 bit chars --> wide chars)
				uint16_t length;		// length of following content
				unsigned char content[1500];// content string
			}MSG_STRING;

			typedef struct COMMAND		// Structure of type command (type = 0x00, status = 0x000c)
			{
				char ID;				// 0x00:        Always 0x02
				uint16_t hlen;			// 0x01 - 0x02: Header length (length + 3 for total length!)
				char sep1;				// 0x03:        Seperator always 0x02
				char type;				// 0x04:        Type of header (see above)
				uint16_t unk1;			// 0x05 - 0x06:
				uint16_t device1;		// 0x07 - 0x08: Channel number of panel
				uint16_t unk2;			// 0x09 - 0x0a: Level number
				uint16_t unk3;			// 0x0b - 0x0c: channel number of the panel
				uint16_t unk4;			// 0x0d - 0x0e: port number?
				uint16_t unk5;			// 0x0f - 0x10:
				char unk6;				// 0x11:        Always 0x0f
				uint16_t count;			// 0x12 - 0x13: Counter
				uint16_t status;		// 0x14 - 0x15: Header type?
				uint16_t device2;		// 0x16 - 0x17:
				uint16_t port2;			// 0x18 - 0x19:
				uint16_t system;		// 0x1a - 0x1b:
				char unk8;				// 0x1c
				uint16_t length;		// 0x1d - 0x1e:
				char command[256];  	// 0x1f - hlen + 2
				unsigned char checksum;	// Last byte: Checksum
			}COMMAND;

			typedef struct CNTINFO		// Structure of type command (type = 0x01, status = 0x0502)
			{
				char ID;				// 0x00:        Always 0x02
				uint16_t hlen;			// 0x01 - 0x02: Header length (length + 3 for total length!)
				char sep1;				// 0x03:        Seperator always 0x02
				char type;				// 0x04:        Type of header (see above)
				uint16_t unk1;			// 0x05 - 0x06:
				uint16_t unk2;		// 0x07 - 0x08: Channel number of panel
				uint16_t unk3;			// 0x09 - 0x0a: Level number
				uint16_t unk4;			// 0x0b - 0x0c: channel number of the panel
				uint16_t unk5;			// 0x0d - 0x0e: port number?
				uint16_t unk6;			// 0x0f - 0x10:
				char unk7;				// 0x11:        Always 0x0f
				uint16_t count;			// 0x12 - 0x13: = 0x0000
				uint16_t status;		// 0x14 - 0x15: 0x0502
				uint16_t channel2;		// 0x16 - 0x17:
				uint16_t port2;			// 0x18 - 0x19:
				uint16_t unk7;			// 0x1a - 0x1b:
				char unk8;				// 0x1c
				uint16_t length;		// 0x1d - 0x1e:
				char command[256];  	// 0x1f - hlen + 2
				unsigned char checksum;	// Last byte: Checksum
			}CNTINFO;

			/*
			 * Staus of bytes 0x14, 0x15:
			 *    0x0084  press?
			 *    0x0085  release?
			 *    0x008c  Initialize a SIP call?
			 *    0x0090 ?
			 *    0x0091 ?
			 *    0x0092 ?
			 *    0x0093  Button press
			 *    0x0094  Button release
			 *    0x0097  Identification string
			 *    0x0581 ?
			 */
			typedef struct PAN_HEAD
			{
				char ID;				// 0x00:        Always 0x02
				uint16_t hlen;			// 0x01 - 0x02: Header length (length + 3 for total length!)
				char sep1;				// 0x03:        Seperator always 0x02
				char type;				// 0x04:        Type of header?
				uint16_t unk1;			// 0x05 - 0x06:
				uint16_t unk2;			// 0x07 - 0x08:
				uint16_t unk3;			// 0x09 - 0x0a:
				uint16_t unk4;			// 0x0b - 0x0c:
				uint16_t device1;		// 0x0d - 0x0e: Channel number panel
				uint16_t unk5;			// 0x0f - 0x10: Always 0x0001?
				char unk6;				// 0x11:        Always 0x0f?
				uint16_t count;			// 0x12 - 0x13: Counter
				uint16_t status;		// 0x14 - 0x15: Status (see above)
				uint16_t device2;		// 0x16 - 0x17: Channel number panel
				uint16_t port;			// 0x18 - 0x19: Port number
				uint16_t system;		// 0x1a - 0x1b: system number?
				uint16_t channel;		// 0x1c - 0x1d: Channel number
				unsigned char checksum;
			}PAN_HEAD;

		private:
			void start_connect(asio::ip::tcp::resolver::results_type::iterator endpoint_iter);
			void handle_connect(const std::error_code& error, asio::ip::tcp::resolver::results_type::iterator endpoint_iter);
			void start_read();
			void handle_read(const std::error_code& error, std::size_t n);
			void start_write();
			void handle_write(const std::error_code& error);
			void check_deadline();
			uint16_t swapWord(uint16_t w);
			uint32_t swapDWord(uint32_t dw);

			bool stopped_ = false;
			asio::ip::tcp::resolver::results_type endpoints_;
			asio::ip::tcp::socket socket_;
			std::string input_buffer_;
			asio::steady_timer deadline_;
			asio::steady_timer heartbeat_timer_;
			std::function<void(const strings::String&)> callback;
	};
}

#endif
