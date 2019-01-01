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

#include "syslog.h"
#include "connectionmanager.h"

extern Syslog *sysl;

namespace http
{
	namespace server
	{
		ConnectionManager::ConnectionManager()
		{
			sysl->TRACE(Syslog::ENTRY, std::string("ConnectionManager::ConnectionManager()"));
		}

		ConnectionManager::~ConnectionManager()
		{
			sysl->TRACE(Syslog::EXIT, std::string("ConnectionManager::ConnectionManager()"));
		}

		void ConnectionManager::start(ConnectionPtr c)
		{
			sysl->TRACE(std::string("ConnectionManager::start(ConnectionPtr c)"));
			m_connections.insert(c);
			c->start();
		}

		void ConnectionManager::stop(ConnectionPtr c)
		{
			sysl->TRACE(std::string("ConnectionManager::stop(ConnectionPtr c)"));
			m_connections.erase(c);
			c->stop();
		}

		void ConnectionManager::stop_all()
		{
			sysl->TRACE(std::string("ConnectionManager::stop_all()"));

			for (auto c: m_connections)
				c->stop();

			m_connections.clear();
		}
	} // namespace server
} // namespace http
