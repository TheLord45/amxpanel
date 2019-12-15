/*
 * Copyright (C) 2019 by Andreas Theofilu <andreas@theosys.at>
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

#include <chrono>
#if __GNUC__ < 9
   #if __cplusplus < 201703L
      #warning "Your C++ compiler seems to have no support for C++17 standard!"
   #endif
   #include <experimental/filesystem>
   namespace fs = std::experimental::filesystem;
#else
   #include <filesystem>
   namespace fs = std::filesystem;
#endif
#include "syslog.h"
#include "config.h"
#include "trace.h"
#include "directory.h"

using namespace std;
using namespace dir;
using namespace chrono_literals;

extern Syslog *sysl;
extern Config *Configuration;

int Directory::readDir()
{
	DECL_TRACER("Directory::readDir()");

	if (path.empty())
		return 0;

	int count = 0;

	try
	{
		for(auto& p: fs::directory_iterator(path))
		{
			DFILES_T dr;
			string f = fs::path(p.path()).filename();

			if (f.at(0) == '.')
				continue;

			if (path.find("__system/") == string::npos && f.find("__system") != string::npos)
				continue;
#if __GNUC__ < 9
			if (path.find("scripts") != string::npos && fs::is_directory(p.path()))
#else
			if (path.find("scripts") != string::npos && p.is_directory())
#endif
				continue;

			count++;
			dr.count = count;
#if __GNUC__ < 9
			time_t ti = fs::last_write_time(p.path()).time_since_epoch().count();
#else
			time_t ti = p.last_write_time().time_since_epoch().count();
#endif
			dr.date = (ti / 1000000000) + 6437664000;

#if __GNUC__ < 9
			if (fs::is_directory(p.path()))
#else
			if (p.is_directory())
#endif
				dr.size = 0;
			else
#if __GNUC__ < 9
				dr.size = fs::file_size(p.path());
#else
				dr.size = p.file_size();
#endif

			if (strip)
				dr.name = f;
			else
				dr.name = p.path();

			dr.attr = 0;

#if __GNUC__ < 9
			if (fs::is_directory(p.path()))
#else
			if (p.is_directory())
#endif
				dr.attr = dr.attr | ATTR_DIRECTORY;
#if __GNUC__ < 9
			else if (fs::is_regular_file(p.path()))
#else
			else if (p.is_regular_file())
#endif
			{
				if (dr.name.find(".png") != string::npos || dr.name.find(".PNG") != string::npos ||
						dr.name.find(".jpg") != string::npos || dr.name.find(".JPG") != string::npos)
					dr.attr = dr.attr | ATTR_GRAPHIC;
				else if (dr.name.find(".wav") != string::npos || dr.name.find(".WAV") != string::npos ||
						dr.name.find(".mp3") != string::npos || dr.name.find(".MP3") != string::npos)
					dr.attr = dr.attr | ATTR_SOUND;
				else
					dr.attr = dr.attr | ATTR_TEXT;
			}

#if __GNUC__ < 9
			if (fs::is_symlink(p.path()))
#else
			if (p.is_symlink())
#endif
				dr.attr |= ATTR_LINK;

			entries.push_back(dr);

			if (Configuration->getDebug())
			{
				char buf[4096];
				char d, g, l;

				d = l = '_';
				g = ' ';

				if (dr.attr & ATTR_DIRECTORY)
					d = 'D';

				if (dr.attr & ATTR_GRAPHIC)
					g = 'g';
				else if (dr.attr & ATTR_SOUND)
					g = 's';
				else if (dr.attr & ATTR_TEXT)
					g = 't';

				if (dr.attr & ATTR_LINK)
					l = 'L';

				struct tm *t = localtime(&dr.date);

				if (t == nullptr)
					snprintf(buf, sizeof(buf), "%c%c%c %8zu 0000-00-00 00:00:00 %s", d, g, l, dr.size, dr.name.c_str());
				else
					snprintf(buf, sizeof(buf), "%c%c%c %8zu %4d-%02d-%02d %02d:%02d:%02d %s", d, g, l, dr.size, t->tm_year + 1900, t->tm_mon+1, t->tm_mday,
                         t->tm_hour, t->tm_min, t->tm_sec, dr.name.c_str());

				sysl->TRACE(string("Directory::readDir: ")+buf);
			}
		}

		done = true;
		sysl->TRACE("Directory::readDir: Read "+to_string(count)+" entries.");
	}
	catch(exception& e)
	{
		sysl->errlog(string("Directory::readDir: ")+e.what());
		entries.clear();
		return 0;
	}

	return count;
}

int Directory::readDir (const std::string &p)
{
	DECL_TRACER("Directory::readDir (const std::string &p)");

	path.assign(p);

	if (done)
		entries.clear();

	done = false;
	return readDir();
}

size_t Directory::getNumEntries()
{
	DECL_TRACER("Directory::getNumEntries()");

	if (done)
		return entries.size();

	return 0;
}

DFILES_T Directory::getEntry (size_t pos)
{
	DECL_TRACER("Directory::getEntry (size_t pos)");

	if (!done || pos >= entries.size())
	{
		DFILES_T d;
		d.attr = 0;
		d.count = 0;
		d.date = 0;
		d.size = 0;
		return d;
	}

	return entries.at(pos);
}

string Directory::stripPath (const string &p, size_t idx)
{
	DECL_TRACER("Directory::stripPath (const string &p, size_t idx)");

	if (!done || idx > entries.size())
		return "";

	size_t pos;
	DFILES_T dr = getEntry(idx);

	if ((pos = dr.name.find(p)) == string::npos)
		return "";

	return dr.name.substr(pos + p.length());
}

string Directory::stripPath (const string &p, const string &s)
{
	DECL_TRACER("Directory::stripPath (const string &p, const string &s)");

	size_t pos;

	if ((pos = s.find(p)) == string::npos)
		return "";

	return s.substr(pos + p.length());
}

size_t Directory::getFileSize (const string &f)
{
	DECL_TRACER("Directory::getFileSize (const string &f)");
	size_t s = 0;

	try
	{
		if (!fs::path(f).has_filename())
			return s;

		s = fs::file_size(f);
	}
	catch(exception& e)
	{
		sysl->errlog(string("Directory::getFileSize: ")+e.what());
		s = 0;
	}

	return s;
}

bool Directory::isFile (const string &f)
{
	DECL_TRACER("Directory::isFile (const string &f)");

	try
	{
		return fs::is_regular_file(f);
	}
	catch(exception& e)
	{
		sysl->errlog(string("Directory::isFile: ")+e.what());
	}

	return false;
}

bool Directory::isDirectory (const string &f)
{
	DECL_TRACER("Directory::isDirectory (const string &f)");

	try
	{
		return fs::is_directory(f);
	}
	catch(exception& e)
	{
		sysl->errlog(string("Directory::isFile: ")+e.what());
	}

	return false;
}

bool Directory::exists (const string &f)
{
	DECL_TRACER("Directory::exists (const string &f)");

	try
	{
		return fs::exists(f);
	}
	catch(exception& e)
	{
		sysl->errlog(string("Directory::isFile: ")+e.what());
	}

	return false;
}

bool Directory::checkDot (const string &s)
{
	DECL_TRACER("Directory::checkDot (const string &s)");

	size_t pos = s.find_last_of("/");
	string f = s;

	if (pos != string::npos)
		f = s.substr(pos + 1);

	if (f.at(0) == '.')
		return true;

	return false;
}
