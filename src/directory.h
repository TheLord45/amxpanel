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

#ifndef __DIRECTORY_H__
#define __DIRECTORY_H__

#include <string>
#include <vector>

namespace dir
{
	#define ATTR_TEXT       0x0001
	#define ATTR_GRAPHIC    0x0002
	#define ATTR_SOUND      0x0004
	#define ATTR_DIRECTORY  0x0008
	#define ATTR_LINK       0x0010

	struct DFILES
	{
		int count;				// Counter starting by 1
		size_t size;			// Size of file (directory = 0)
		unsigned short attr;	// Attributes (internal only)
		time_t date;			// The last modification date of the file/directory
		std::string name;		// Name of file/directory
	};

	typedef DFILES DFILES_T;

	class Directory
	{
		std::vector<DFILES_T> entries;
		bool done{false};
		std::string path;
		bool strip{false};

		public:
			Directory() = default;
			explicit Directory(const std::string& p) : path{p} {}

			int readDir();
			int readDir(const std::string& p);
			size_t getNumEntries();
			void setPath(const std::string& p) { path.assign(p); }
			void setStripPath(bool b) { strip = b; }
			size_t getFileSize(const std::string& f);
			bool isFile(const std::string& f);
			bool isDirectory(const std::string& f);
			bool exists(const std::string& f);
			DFILES_T getEntry(size_t pos);
			std::string stripPath(const std::string& p, size_t idx);
			std::string stripPath(const std::string& p, const std::string& s);
			bool testDirectory(unsigned short att) { return (att & ATTR_DIRECTORY); }
			bool testText(unsigned short att) { return (att & ATTR_TEXT); }
			bool testGraphic(unsigned short att) { return (att & ATTR_GRAPHIC); }
			bool testSound(unsigned short att) { return (att & ATTR_SOUND); }
			bool testLink(unsigned short att) { return (att & ATTR_LINK); }

		private:
			bool checkDot(const std::string& s);
	};
}

#endif
