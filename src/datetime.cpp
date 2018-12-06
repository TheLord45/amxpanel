//
//  datetime.cpp
//  hvl
//
//  Created by Andreas Theofilu on 05.05.18.
//

#include <string>
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <math.h>
#include "datetime.h"
#include "syslog.h"

extern Syslog *sysl;

using namespace std;

DateTime::DateTime()
{
	format = "%Y-%m-%d %H:%M:%S%z";
	timestamp = chrono::system_clock::now();
}

DateTime::DateTime(string f)
{
	format = f;
	timestamp = chrono::system_clock::now();
}

int DateTime::getDay()
{
	time_t t = getTimestamp();
	std::tm *lt = std::localtime(&t);
	return lt->tm_mday;
}

int DateTime::getMonth()
{
	time_t t = getTimestamp();
	std::tm *lt = std::localtime(&t);
	return lt->tm_mon + 1;
}

int DateTime::getMonthDays()
{
    return checkDayOfMonth(getMonth(), getYear());
}

int DateTime::getYear()
{
	time_t t = getTimestamp();
	std::tm *lt = std::localtime(&t);
	return lt->tm_year + 1900;
}

int DateTime::getYearDays()
{
    //             Ja  Fe  Ma  Ap  Ma  Ju  Jl  Au  Se  Ok  No  De
    int ds[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    int year = getYear();

    if (((year % 4) == 0 && (year % 100) != 0) || (year % 400) == 0)
        ds[1] = 29;

    int sum = 0;

    for (int i = 0; i < 12; i++)
        sum += ds[i];

    return sum;
}

int DateTime::getHour()
{
	time_t t = getTimestamp();
	std::tm *lt = std::localtime(&t);
	return lt->tm_hour;
}

int DateTime::getMinute()
{
	time_t t = getTimestamp();
	std::tm *lt = std::localtime(&t);
	return lt->tm_min;
}

int DateTime::getSecond()
{
	time_t t = getTimestamp();
	std::tm *lt = std::localtime(&t);
	return lt->tm_sec;
}

int DateTime::getHundrets()
{
	__int64_t ms = chrono::duration_cast<chrono::milliseconds>(timestamp.time_since_epoch()).count();
	return (int)(ms - (ms / 1000));
}

int DateTime::getTimeZone()
{
	time_t t = getTimestamp();
	std::tm *lt = std::localtime(&t);
	return lt->tm_isdst;
}

std::chrono::system_clock::time_point DateTime::getNow()
{
	return chrono::system_clock::now();
}

time_t DateTime::getTimestamp()
{
	return chrono::system_clock::to_time_t(timestamp);
}

string DateTime::getTimestamp(string f)
{
	time_t t = getTimestamp();
	std::tm *lt = std::localtime(&t);
	return doFormat(lt, f);
}

string DateTime::getTimestamp(const chrono::system_clock::time_point& t)
{
	time_t ti = chrono::system_clock::to_time_t(t);
	std::tm *lt = std::localtime(&ti);
	return doFormat(lt, format);
}

time_t DateTime::getEpoch()
{
	return chrono::system_clock::to_time_t(timestamp);
}

/**
 * setTimestamp - expects a timestamp in ISO8601
 *
 * The function sets the internal time of the class to the given time.
 */
void DateTime::setTimestamp(std::string tim)
{
	if (tim.find("-") == string::npos || tim.find(":") == string::npos)
	{
		sysl->errlog("DateTime::setTimestamp: Error invalid timestamp: "+tim);
		return;
	}

	std::tm epoch = {};
	int pos = 0;
	string part = "";
	bool has_data = false;
	memset(&epoch, 0, sizeof(epoch));	// Initialize the struct
	epoch.tm_isdst = -1;				// Set time zone to unknown

	for (char c : tim)
	{
		if (c == '-' || c == ' ' || c == ':' || c == '+')
		{
			switch (pos)
			{
				case 0: epoch.tm_year = atoi(part.c_str()) - 1900; break;
				case 1: epoch.tm_mon = atoi(part.c_str()) - 1; break;
				case 2: epoch.tm_mday = atoi(part.c_str()); break;
				case 3: epoch.tm_hour = atoi(part.c_str()); break;
				case 4: epoch.tm_min = atoi(part.c_str()); break;
				case 5: epoch.tm_sec = atoi(part.c_str()); break;
				case 6:
					int tz = atoi(part.c_str());

					if (tz > 99)
					{
						long sec = (tz / 100) * 3600;
						sec = (tz - (sec / 3600 * 100)) * 60;
						epoch.tm_gmtoff = sec;
					}
					else
						epoch.tm_gmtoff = stol(part) * 3600;
				break;
			}

			pos++;
			has_data = false;

			if (c == '+')
			{
				part = c;
				has_data = true;
			}
			else
				part = "";
		}
		else
		{
			part += c;
			has_data = true;
		}
	}

	if (has_data && pos >= 6)
	{
		int tz = atoi(part.c_str());

		if (tz > 99)
		{
			long sec = (tz / 100) * 3600;
			sec = (tz - (sec / 3600 * 100)) * 60;
			epoch.tm_gmtoff = sec;
		}
		else
			epoch.tm_gmtoff = atol(part.c_str()) * 3600;
	}

	time_t base = mktime(&epoch);
	timestamp = chrono::system_clock::from_time_t(base);
}

/**
 * setTimestamp - expects a timestamp in ISO8601
 */
void DateTime::setTimestamp(char *tm)
{
    if (tm == 0)
        return;

	string t = "";
	t.append(tm);
	setTimestamp(t);
}

void DateTime::setTimestamp(int year, int month, int day, int hour, int minute, int second, int tz)
{
	int d = day;

	if (day < 1)
		d = checkDayOfMonth(month, year);
	else
	{
		int cd = checkDayOfMonth(month, year);

		if (day > cd)
			d = cd;
	}

	struct std::tm t;
	t.tm_sec = second;			// second of minute (0 .. 59 and 60 for leap seconds)
	t.tm_min = minute;			// minute of hour (0 .. 59)
	t.tm_hour = hour;			// hour of day (0 .. 23)
	t.tm_mday = d;				// day of month (0 .. 31)
	t.tm_mon = month - 1;		// month of year (0 .. 11)
	t.tm_year = year - 1900;	// year since 1900
	t.tm_isdst = -1;			// determine whether daylight saving time

	if (tz > 99 || tz < -99)
	{
		long sec = (tz / 100) * 3600;
		sec = (tz - (sec / 3600 * 100)) * 60;
		t.tm_gmtoff = sec;
		t.tm_isdst = 1;
	}
	else if (tz != 0)
	{
		t.tm_gmtoff = tz * 3600;
		t.tm_isdst = 1;
	}

	std::time_t tt = std::mktime(&t);

	if (tt == -1)
	{
		sysl->errlog(std::string("DateTime::setTimestamp: No valid system time!"));
//		throw "no valid system time";
		return;
	}

	timestamp = std::chrono::system_clock::from_time_t(tt);
}

/**
 * doFormat() - formats a string according to the patterns in the
 * format string. Valid patterns are:
 *
 */
string DateTime::doFormat(const std::tm *t, string f)
{
	ostringstream out;

	out << put_time(t, f.c_str());
	return out.str();
}

int DateTime::checkDayOfMonth(int mon, int year)
{
	if (mon < 1 || mon > 31)
		return 0;

	switch(mon)
	{
		case 1:		return 31;		// January

		case 2:						// February
			if (((year % 4) == 0 && (year % 100) != 0) || (year % 400) == 0)
				return 29;
			else
				return 28;
		break;

		case 3:		return 31;		// March
		case 4:		return 30;		// April
		case 5:		return 31;		// Mai
		case 6:		return 30;		// June
		case 7:		return 31;		// July
		case 8:		return 31;		// August
		case 9:		return 30;		// September
		case 10:	return 31;		// October
		case 11:	return 30;		// November
		case 12:	return 31;		// December
	}

	return 0;
}

chrono::system_clock::time_point DateTime::Interval(string intval)
{
	unsigned long pos1, pos2;

	if ((pos1 = intval.find(" ")) == string::npos)
		return timestamp;

	if ((pos2 = intval.find(" ", pos1 + 1)) == string::npos)
		return timestamp;

	unsigned long anz = std::atoi(intval.substr(pos1 + 1, pos2 - pos1).c_str());
	chrono::hours day(anz * 24);
	chrono::hours week(anz * 24 * 7);

	if (intval.find("+") != string::npos)
	{
		if (intval.find("seconds") != string::npos || intval.find("second") != string::npos)
			return timestamp + chrono::seconds(anz);

		if (intval.find("minutes") != string::npos || intval.find("minute") != string::npos)
			return timestamp + chrono::minutes(anz);

		if (intval.find("hours") != string::npos || intval.find("hour") != string::npos)
			return timestamp + chrono::hours(anz);

		if (intval.find("days") != string::npos || intval.find("day") != string::npos)
			return timestamp + day;

		if (intval.find("weeks") != string::npos || intval.find("week") != string::npos)
			return timestamp + week;

		if (intval.find("months") != string::npos || intval.find("month") != string::npos)
        {
            DateTime dt = *this;
            int m = dt.getMonth();
            int y = dt.getYear();

            for (unsigned long i = 0; i < anz; i++)
            {
                m++;

                if (m > 12)
                {
                    m = 1;
                    y++;
                }

                dt.setTimestamp(y, m, dt.getDay(), dt.getHour(), dt.getMinute(), dt.getSecond());
            }

			return dt.timestamp;
        }

		if (intval.find("years") != string::npos || intval.find("year") != string::npos)
        {
            DateTime dt = *this;
            int y = dt.getYear();

            for (unsigned long i = 0; i < anz; i++)
            {
                y++;
                dt.setTimestamp(y, dt.getMonth(), dt.getDay(), dt.getHour(), dt.getMinute(), dt.getSecond());
            }

			return dt.timestamp;
        }
	}
	else
	{
		if (intval.find("seconds") != string::npos || intval.find("second") != string::npos)
			return timestamp - chrono::seconds(anz);

		if (intval.find("minutes") != string::npos || intval.find("minute") != string::npos)
			return timestamp - chrono::minutes(anz);

		if (intval.find("hours") != string::npos || intval.find("hour") != string::npos)
			return timestamp - chrono::hours(anz);

		if (intval.find("days") != string::npos || intval.find("day") != string::npos)
			return timestamp - day;

		if (intval.find("weeks") != string::npos || intval.find("week") != string::npos)
			return timestamp - week;

		if (intval.find("months") != string::npos || intval.find("month") != string::npos)
        {
            DateTime dt = *this;
            int m = dt.getMonth();
            int y = dt.getYear();

            for (unsigned long i = 0; i < anz; i++)
            {
                m--;

                if (m < 1)
                {
                    m = 12;
                    y--;
                }

                dt.setTimestamp(y, m, dt.getDay(), dt.getHour(), dt.getMinute(), dt.getSecond());
            }

			return dt.timestamp;
        }

        if (intval.find("years") != string::npos || intval.find("year") != string::npos)
        {
            DateTime dt = *this;
            int y = dt.getYear();

            for (unsigned long i = 0; i < anz; i++)
            {
                y--;
                dt.setTimestamp(y, dt.getMonth(), dt.getDay(), dt.getHour(), dt.getMinute(), dt.getSecond());
            }

			return dt.timestamp;
        }
	}

	return timestamp;
}

DateTime DateTime::sunrise(double la, double lo)
{
	int st = 0;

	if (getTimeZone() == 0)
		st = -1;

	setPosition(la, lo, st);
	setCurrentDate(getYear(), getMonth(), getDay());
	double tm = calcSunrise();		// time in minutes since midnight
	int hour, min;
	hour = tm / 60.0;
	min = tm - ((double)hour * 60.0);
	DateTime dt;
	dt.setTimestamp(getYear(), getMonth(), getDay(), hour, min);
	return dt;
}

DateTime DateTime::sunset(double la, double lo)
{
	int st = 0;

	if (getTimeZone() == 0)
		st = -1;

	setPosition(la, lo, st);
	setCurrentDate(getYear(), getMonth(), getDay());
	double tm = calcSunset();		// time in minutes since midnight
	int hour, min;
	hour = tm / 60.0;
	min = tm - ((double)hour * 60.0);
	DateTime dt;
	dt.setTimestamp(getYear(), getMonth(), getDay(), hour, min);
	return dt;
}

DateTime::Season DateTime::getSeason()
{
	DateTime start, end;
	int year = getYear();
	start.setTimestamp(year, 12, 01);				// Winter start
	end.setTimestamp(year+1, 2, 28, 23, 59, 59);	// Winter end

	if (getRawTime() >= start.getRawTime() && getRawTime() <= end.getRawTime())
		return DateTime::WINTER;

	start.setTimestamp(year, 3, 01);				// Spring start
	end.setTimestamp(year, 05, 31, 23, 59, 59);		// Spring end

	if (getRawTime() >= start.getRawTime() && getRawTime() <= end.getRawTime())
		return DateTime::SPRING;

	start.setTimestamp(year, 6, 01);				// Summer start
	end.setTimestamp(year, 8, 31, 23, 59, 59);		// Summer end

	if (getRawTime() >= start.getRawTime() && getRawTime() <= end.getRawTime())
		return DateTime::SUMMER;

	// Else we've autum
	return DateTime::AUTUM;
}

bool DateTime::compareDate(DateTime dt)
{
	if (getYear() == dt.getYear() && getMonth() == dt.getMonth() &&
		getDay() == dt.getDay())
		return true;

	return false;
}

ostringstream DateTime::operator<< (DateTime& t)
{
	ostringstream o;
	o << getTimestamp(t.getRawTime());
	timestamp = t.getRawTime();
	return o;
}

ostringstream DateTime::operator>> (DateTime& t)
{
	ostringstream o;
	o << getTimestamp(timestamp);
	t.setTimestamp(timestamp);
	return o;
}

bool DateTime::operator==(DateTime& t) const
{
	time_t ms1 = chrono::system_clock::to_time_t(timestamp);
	time_t ms2 = chrono::system_clock::to_time_t(t.getRawTime());

	if (ms1 == ms2)
		return true;

	return false;
}

bool DateTime::operator!=(DateTime& t) const
{
	time_t ms1 = chrono::system_clock::to_time_t(timestamp);
	time_t ms2 = chrono::system_clock::to_time_t(t.getRawTime());

	if (ms1 != ms2)
		return true;

	return false;
}

bool DateTime::operator<(DateTime& t) const
{
	__int64_t ms1 = chrono::duration_cast<chrono::milliseconds>(timestamp.time_since_epoch()).count();
	__int64_t ms2 = chrono::duration_cast<chrono::milliseconds>(t.getRawTime().time_since_epoch()).count();

	if (ms1 < ms2)
		return true;

	return false;
}

bool DateTime::operator<=(DateTime& t) const
{
	__int64_t ms1 = chrono::duration_cast<chrono::milliseconds>(timestamp.time_since_epoch()).count();
	__int64_t ms2 = chrono::duration_cast<chrono::milliseconds>(t.getRawTime().time_since_epoch()).count();

	if (ms1 <= ms2)
		return true;

	return false;
}

bool DateTime::operator>(DateTime& t) const
{
	__int64_t ms1 = chrono::duration_cast<chrono::milliseconds>(timestamp.time_since_epoch()).count();
	__int64_t ms2 = chrono::duration_cast<chrono::milliseconds>(t.getRawTime().time_since_epoch()).count();

	if (ms1 > ms2)
		return true;

	return false;
}

bool DateTime::operator>=(DateTime& t) const
{
	__int64_t ms1 = chrono::duration_cast<chrono::milliseconds>(timestamp.time_since_epoch()).count();
	__int64_t ms2 = chrono::duration_cast<chrono::milliseconds>(t.getRawTime().time_since_epoch()).count();

	if (ms1 >= ms2)
		return true;

	return false;
}

DateTime& DateTime::operator= (DateTime& t)
{
	timestamp = t.getRawTime();
	return *this;
}

DateTime& DateTime::operator= (DateTime& t) const
{
	DateTime *me = const_cast<DateTime *>(this);
	me->timestamp = t.getRawTime();
	return *me;
}

DateTime& DateTime::operator+ (DateTime& t)
{
	__int64_t ms1 = chrono::duration_cast<chrono::milliseconds>(timestamp.time_since_epoch()).count();
	__int64_t ms2 = chrono::duration_cast<chrono::milliseconds>(t.getRawTime().time_since_epoch()).count();
	__int64_t erg = 0;

	if (ms1 > ms2)
		erg = ms1 + (ms1 - ms2);
	else
		erg = ms1 + (ms2 - ms1);

	std::chrono::duration<long> dur(erg);
	std::chrono::time_point<std::chrono::system_clock> dt(dur);
	return *this;
}

DateTime& DateTime::operator- (DateTime& t)
{
	__int64_t ms1 = chrono::duration_cast<chrono::milliseconds>(timestamp.time_since_epoch()).count();
	__int64_t ms2 = chrono::duration_cast<chrono::milliseconds>(t.getRawTime().time_since_epoch()).count();
	__int64_t erg = 0;

	if (ms1 > ms2)
		erg = ms1 - (ms1 - ms2);
	else
		erg = ms1 - (ms2 - ms1);

	std::chrono::duration<long> dur(erg);
	std::chrono::time_point<std::chrono::system_clock> dt(dur);
	return *this;
}

DateTime& DateTime::operator+= (DateTime& t)
{
	__int64_t ms1 = chrono::duration_cast<chrono::milliseconds>(timestamp.time_since_epoch()).count();
	__int64_t ms2 = chrono::duration_cast<chrono::milliseconds>(t.getRawTime().time_since_epoch()).count();
	__int64_t erg = 0;

	if (ms1 > ms2)
		erg = ms1 + (ms1 - ms2);
	else
		erg = ms1 + (ms2 - ms1);

	std::chrono::duration<long> dur(erg);
	std::chrono::time_point<std::chrono::system_clock> dt(dur);
	timestamp = dt;
	return *this;
}

DateTime& DateTime::operator-= (DateTime& t)
{
	__int64_t ms1 = chrono::duration_cast<chrono::milliseconds>(timestamp.time_since_epoch()).count();
	__int64_t ms2 = chrono::duration_cast<chrono::milliseconds>(t.getRawTime().time_since_epoch()).count();
	__int64_t erg = 0;

	if (ms1 > ms2)
		erg = ms1 - (ms1 - ms2);
	else
		erg = ms1 - (ms2 - ms1);

	std::chrono::duration<long> dur(erg);
	std::chrono::time_point<std::chrono::system_clock> dt(dur);
	timestamp = dt;
	return *this;
}
