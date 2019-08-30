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

#ifndef __MAP_H__
#define __MAP_H__

#include "panelstruct.h"

class Map
{
	public:
		Map(const std::string& rFile);
		~Map();

		void parse();
		bool isDone() { return done; }
		std::vector<amx::MAP_T>& getCm() { return maps.map_cm; }
		std::vector<amx::MAP_T>& getAm() { return maps.map_am; }
		std::vector<amx::MAP_T>& getLm() { return maps.map_lm; }
		std::vector<amx::MAP_BM_T>& getBm() { return maps.map_bm; }
		std::vector<std::string>& getSm() { return maps.map_sm; }
		std::vector<amx::MAP_T>& getStrm() { return maps.map_strm; }
		std::vector<amx::MAP_PM_T>& getPm() { return maps.map_pm; }

	private:
		bool done;
		std::string fname;
		amx::MAPS_T maps;
};

#endif
