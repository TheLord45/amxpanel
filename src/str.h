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

#ifndef __STR_H__
#define __STR_H__

#include <string>
#include <vector>
#include <algorithm>

class Str : public std::string
{
	public:
		Str() {}
		Str(const std::string& str) : mStr(str) {}

		std::string get() { return mStr; }
		void set(const std::string& str) { mStr = str; }

		std::string& replace(const std::string& old, const std::string& neu);
		const std::string& replace(const std::string& old, const std::string& neu) const;
		static std::string replace(std::string& str, const std::string& old, const std::string& neu);

		int caseCompare(const std::string& str);
		int caseCompare(const std::string& str) const;
		static int caseCompare(const std::string& st1, const std::string& str2);

		std::vector<std::string> split(char sep);
		std::vector<std::string> split(const std::string& seps);
		const std::vector<std::string> split(char sep) const;
		const std::vector<std::string> split(const std::string& seps) const;
		static std::vector<std::string> split(const std::string& str, char sep);
		static std::vector<std::string> split(const std::string& str, const std::string& seps);

		bool isNumeric();
		bool isNumeric() const;
		static bool isNumeric(const std::string& str);

		static inline std::string &ltrim(std::string &s) {
			s.erase(s.begin(), std::find_if(s.begin(), s.end(),
											std::not1(std::ptr_fun<int, int>(std::isspace))));
			return s;
		}

		inline std::string &ltrim() {
			mStr.erase(mStr.begin(), std::find_if(mStr.begin(), mStr.end(),
											std::not1(std::ptr_fun<int, int>(std::isspace))));
			return mStr;
		}

		// trim from end
		static inline std::string &rtrim(std::string &s) {
			s.erase(std::find_if(s.rbegin(), s.rend(),
								 std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
			return s;
		}

		inline std::string &rtrim() {
			mStr.erase(std::find_if(mStr.rbegin(), mStr.rend(),
								 std::not1(std::ptr_fun<int, int>(std::isspace))).base(), mStr.end());
			return mStr;
		}

		// trim from both ends
		static inline std::string &trim(std::string &s) {
			return ltrim(rtrim(s));
		}

		inline std::string &trim() {
			ltrim();
			return rtrim();
		}

	private:
		std::vector<std::string> _split(const std::string& str, const std::string& seps, const bool trimEmpty = false);

		std::string mStr;
};

#endif
