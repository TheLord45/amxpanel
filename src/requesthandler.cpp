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

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>
#include "requesthandler.h"
#include "mimetypes.h"
#include "reply.h"
#include "request.h"
#include "syslog.h"
#include "touchpanel.h"

extern std::string pName;
extern Syslog *sysl;
extern amx::TouchPanel *pTouchPanel;

namespace http
{
	namespace server
	{
		RequestHandler::RequestHandler(const std::string& doc_root)
				: m_doc_root(doc_root)
		{
			URIs.push_back(doc_root+"/weather/year");
			URIs.push_back(doc_root+"/weather/month");
			URIs.push_back(doc_root+"/weather/day");
			URIs.push_back(doc_root+"/weather/range");
		}

		void RequestHandler::HandleRequest(const Request& req, Reply& rep)
		{
			// Decode url to path.
			std::string request_path;

			if (!url_decode(req.uri, request_path))
			{
				rep = Reply::stock_reply(Reply::bad_request);
				return;
			}

			// Request path must be absolute and not contain "..".
			if (request_path.empty() || request_path[0] != '/' || request_path.find("..") != std::string::npos)
			{
				sysl->errlog("RequestHandler::HandleRequest: Bad rquest: "+request_path);
				rep = Reply::stock_reply(Reply::bad_request);
				return;
			}

			// If path ends in slash (i.e. is a directory) then add "index.html".
			if (request_path[request_path.length() - 1] == '/')
				request_path += "index.html";

			// Determine the file extension.
			std::size_t last_slash_pos = request_path.find_last_of("/");
			std::size_t last_dot_pos = request_path.find_last_of(".");
			std::string extension;

			if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos)
				extension = request_path.substr(last_dot_pos + 1);

			// Open the file to send back.
			std::string full_path = m_doc_root + request_path;
			// Test for a known virtual URL.
			// If it is a known virtual URL, don't try to read the file.
			bool virtURL = false;
			std::ifstream is;

			if (!(virtURL = testURL(full_path)))
			{
				is.open(full_path.c_str(), std::ios::in | std::ios::binary);

				if (!is)
				{
					sysl->log(Syslog::NOTICE, "RequestHandler::HandleRequest: URL not found: "+full_path);
					rep = Reply::stock_reply(Reply::not_found);
					return;
				}
				else
					sysl->DebugMsg("RequestHandler::HandleRequest: Loaded real file "+full_path);
			}

			// Fill out the reply to be sent to the client.
			rep.status = Reply::ok;
			// If it is a standard URL, call a function to create the
			// HTML page.
			if (!virtURL)
			{
				char buf[512];

				while (is.read(buf, sizeof(buf)).gcount() > 0)
					rep.content.append(buf, is.gcount());
			}
			else
			{
				sysl->DebugMsg("RequestHandler::HandleRequest: Virtual page \""+full_path+"\" was found.");
// --> FIXME!				Evaluate ev(req);
//				rep.content = ev.getHttpPage();
				rep.content = pTouchPanel->requestPage(req).toString();
				extension.assign("html");
			}

			if (virtURL)
				rep.headers.resize(4);
			else
				rep.headers.resize(2);

			rep.headers[0].name = "Content-Length";
			rep.headers[0].value = std::to_string(rep.content.length());
			rep.headers[1].name = "Content-Type";
			rep.headers[1].value = mime_types::extension_to_type(extension);

			if (virtURL)
			{
				rep.headers[2].name = "Cache-Control";
				rep.headers[2].value = "no-store, must-revalidate";
				rep.headers[3].name = "Expires";
				rep.headers[3].value = "0";
			}

			sysl->DebugMsg("RequestHandler::HandleRequest: Header for answer was initialized. (Mime type: "+rep.headers[1].value+")");
		}

		bool RequestHandler::url_decode(const std::string& in, std::string& out)
		{
			out.clear();
			out.reserve(in.length());

			for (std::size_t i = 0; i < in.length(); ++i)
			{
				if (in[i] == '%')
				{
					if (i + 3 <= in.length())
					{
						int value = 0;
						std::istringstream is(in.substr(i + 1, 2));

						if (is >> std::hex >> value)
						{
							out += static_cast<char>(value);
							i += 2;
						}
						else
						{
							return false;
						}
					}
					else
					{
						return false;
					}
				}
				else if (in[i] == '+')
				{
					out += ' ';
				}
				else
				{
					out += in[i];
				}
			}

			return true;
		}

		bool RequestHandler::testURL(const std::string& URL)
		{
			std::size_t pos = URL.find("?");
			std::string u;

			if (pos != std::string::npos)
				u = URL.substr(0, pos);
			else
				u = URL;

			for (auto iter = URIs.cbegin(); iter != URIs.cend(); ++iter)
			{
				if (u.compare(*iter) == 0)
					return true;
			}

			return false;
		}
	} // namespace server
} // namespace http
