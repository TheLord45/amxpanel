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
			/*
			 *  Header types from controller:
			 *     0x00  Normal header with informations or a command
			 *     0x01  Startheader with controler identification
			 *     0x08  End of initialisation
			 * 
			 *  Header type from Panel
			 *     0x00  Initializing information
			 *     0x12  Panel identification
			 */
			typedef struct CNT_HEAD
			{
				char ID;			// 0x00:        Always 0x02
				uint16_t hlen;		// 0x01 - 0x02: Header length (length + 3 for total length!)
				char sep1;			// 0x03:        Seperator always 0x02
				char type;			// 0x04:        Type of header (see above)
				uint16_t pan_port;	// 0x05 - 0x06: Port number of panel
				uint16_t device;	// 0x07 - 0x08: Channel number of panel
				uint16_t pan_level;	// 0x09 - 0x0a: Level number
				uint16_t channel;	// 0x0d - 0x0e: channel number of the panel
				uint16_t port;		// 0x0f - 0x10: port number?
				char unk7;			// 0x11:
				uint16_t count;		// 0x12 - 0x13: From Panel: Counter per command
				uint16_t unk9;		// 0x14 - 0x15:
				uint16_t channel2;	// 0x16 - 0x17:
				uint16_t port2;		// 0x18 - 0x19:
				uint16_t unk10;		// 0x1a - 0x1b:
				uint16_t unk11;		// 0x1c - 0x1d:
				
				uint16_t len;		// 0x1d - 0x1e: Length of command
				uint16_t unk12;		// 0x1f - 0x1f:
				uint16_t modID;		// 0x20 - 0x21: Internal model ID
				char serial[16];	// 0x22 - 0x31: A serial number or "N/A" or nothing
				uint16_t firmwareID;// 0x32 - 0x33: Internal modul number
			}HEAD;

			typedef struct PAN_HEAD
			{
				char ID;			// 0x00:        Always 0x02
				uint16_t hlen;		// 0x01 - 0x02: Header length (length + 3 for total length!)
				char sep1;			// 0x03:        Seperator always 0x02
				char type;			// 0x04:        Type of header
				uint16_t unk1;		// 0x05 - 0x06:
				uint16_t unk2;		// 0x07 - 0x08:
				uint16_t unk3;		// 0x09 - 0x0a:
				uint16_t unk4;		// 0x0b - 0x0c:
				uint16_t device1;	// 0x0d - 0x0e: Channel number panel

				uint16_t count;		// 0x12 - 0x13: Counter
				uint16_t bchannel;	// 0x14 - 0x15: 
				uint16_t device2;	// 0x16 - 0x17: Channel number panel
				uint16_t port;		// 0x18 - 0x19: Port number
				uint16_t level;		// 0x0a - 0x0b: level number?
				uint16_t channel;	// 0x1c - 0x0d: Channel number
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
