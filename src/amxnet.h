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

		private:
			void start_connect(asio::ip::tcp::resolver::results_type::iterator endpoint_iter);
			void handle_connect(const std::error_code& error, asio::ip::tcp::resolver::results_type::iterator endpoint_iter);
			void start_read();
			void handle_read(const std::error_code& error, std::size_t n);
			void start_write();
			void handle_write(const std::error_code& error);
			void check_deadline();

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
