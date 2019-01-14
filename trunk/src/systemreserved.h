/*
 * Copyright (C) 2019 by Andreas Theofilu <andreas@theosys.at>
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

#ifndef __SYSTEMRESERVED_H__
#define __SYSTEMRESERVED_H__

#include <vector>
#include "strings.h"

namespace amx
{
	class SystemReserved
	{
		public:
			inline bool isSystemReserved(int ap)
			{
				for (size_t i = 0; i < sysReserved.size(); i++)
					if (sysReserved[i].res == ap)
						return true;

				return false;
			}

			inline const strings::String& getFuncName(int ap)
			{
				for (size_t i = 0; i < sysReserved.size(); i++)
					if (sysReserved[i].res == ap)
						return sysReserved[i].fname;

				return none;
			}

			inline const strings::String& getButtonName(int ap)
			{
				for (size_t i = 0; i < sysReserved.size(); i++)
					if (sysReserved[i].res == ap)
						return sysReserved[i].bname;

					return none;
			}

			inline const strings::String getBorderStyle(const strings::String& name)
			{
				for (size_t i = 0; i < sysBorders.size(); i++)
				{
					if (sysBorders[i].name.caseCompare(name) == 0)
					{
						strings::String ret = strings::String("  ")+sysBorders[i].style1+"\n";

						if (!sysBorders[i].style2.empty())
							ret += strings::String("  ")+sysBorders[i].style2+"\n";

						if (!sysBorders[i].style3.empty())
							ret += strings::String("  ")+sysBorders[i].style3+"\n";

						if (!sysBorders[i].style4.empty())
							ret += strings::String("  ")+sysBorders[i].style4+"\n";

						return ret;
					}
				}

				return none;
			}

		private:
			const strings::String none;

			typedef struct
			{
				int res;				// system reserved ID
				strings::String fname;	// function name
				strings::String bname;	// button name
			}SYS_RESERVED;

			typedef struct
			{
				strings::String name;	// The name of the border
				strings::String style1;	// The 1st style command
				strings::String style2;	// The 2st style command
				strings::String style3;	// The 3st style command
				strings::String style4;	// The 4st style command
			}SYS_BORDERS;

			const std::vector<SYS_RESERVED> sysReserved =
			{
				{ 141, "startTimeStandard", "btTimeStandard" },	// Standard time
				{ 142, "startTimeAMPM", "btTimeAM/PM" },		// Time AM/PM
				{ 143, "startTime24", "btTime24" },				// 24 hour time
				{ 151, "startDate_151", "btDate151" },			// Date: weekday
				{ 152, "startDate_152", "btDate152" },			// Date: mm/dd
				{ 153, "startDate_153", "btDate153" },			// Date: dd/mm
				{ 154, "startDate_154", "btDate154" },			// Date: mm/dd/yyyy
				{ 155, "startDate_155", "btDate155" },			// Date: dd/mm/yyyy
				{ 156, "startDate_156", "btDate156" },			// Date: month dd, yyyy
				{ 157, "startDate_157", "btDate157" },			// Date: dd month, yyyy
				{ 158, "startDate_158", "btDate158" }			// Date: yyyy-mm-dd
			};

			const std::vector<SYS_BORDERS> sysBorders =
			{
				{ "Single Line", "border-style: solid;", "border-width: 1px;", "", "" },
				{ "Double Line", "border-style: solid;", "border-width: 2px;", "", "" },
				{ "Quad Line", "border-style: solid;", "border-width: 4px;", "", "" },
				{ "Picture Frame", "border-style: double;", "", "", "" },
				{ "Circle 15", "border-style: solid;", "border-width: 2px;", "border-radius: 15px;", "" },
				{ "Circle 25", "border-style: solid;", "border-width: 2px;", "border-radius: 25px;", "" },
				{ "Circle 35", "border-style: solid;", "border-width: 2px;", "border-radius: 35px;", "" },
				{ "Circle 45", "border-style: solid;", "border-width: 2px;", "border-radius: 45px;", "" },
				{ "Circle 55", "border-style: solid;", "border-width: 2px;", "border-radius: 55px;", "" },
				{ "Circle 65", "border-style: solid;", "border-width: 2px;", "border-radius: 65px;", "" },
				{ "Circle 75", "border-style: solid;", "border-width: 2px;", "border-radius: 75px;", "" },
				{ "Circle 85", "border-style: solid;", "border-width: 2px;", "border-radius: 85px;", "" },
				{ "Circle 95", "border-style: solid;", "border-width: 2px;", "border-radius: 95px;", "" },
				{ "Circle 105", "border-style: solid;", "border-width: 2px;", "border-radius: 105px;", "" },
				{ "Circle 115", "border-style: solid;", "border-width: 2px;", "border-radius: 115px;", "" },
				{ "Circle 125", "border-style: solid;", "border-width: 2px;", "border-radius: 125px;", "" },
				{ "Circle 135", "border-style: solid;", "border-width: 2px;", "border-radius: 135px;", "" },
				{ "Circle 145", "border-style: solid;", "border-width: 2px;", "border-radius: 145px;", "" },
				{ "Circle 155", "border-style: solid;", "border-width: 2px;", "border-radius: 155px;", "" },
				{ "Circle 165", "border-style: solid;", "border-width: 2px;", "border-radius: 165px;", "" },
				{ "Circle 175", "border-style: solid;", "border-width: 2px;", "border-radius: 175px;", "" },
				{ "AMX Elite Inset -L", "border-style: groove;", "border-width: 10px;", "", "" },
				{ "AMX Elite Raised -L", "border-style: ridge;", "border-width: 10px;", "", "" },
				{ "AMX Elite Inset -M", "border-style: groove;", "border-width: 5px;", "", "" },
				{ "AMX Elite Raised -M", "border-style: ridge;", "border-width: 5px;", "", "" },
				{ "AMX Elite Inset -S", "border-style: groove;", "border-width: 2px;", "", "" },
				{ "AMX Elite Raised -S", "border-style: ridge;", "border-width: 2px;", "", "" },
				{ "Bevel Inset -L", "border-style: inset;", "border-width: 10px;", "", "" },
				{ "Bevel Raised -L", "border-style: outset;", "border-width: 10px;", "", "" },
				{ "Bevel Inset -M", "border-style: inset;", "border-width: 5px;", "", "" },
				{ "Bevel Raised -M", "border-style: outset;", "border-width: 5px;", "", "" },
				{ "Bevel Inset -S", "border-style: inset;", "border-width: 2px;", "", "" },
				{ "Bevel Raised -S", "border-style: outset;", "border-width: 2px;", "", "" }
			};
	};
}

#endif
