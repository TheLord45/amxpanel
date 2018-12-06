//
//  datetime.h
//  hvl
//
//  Created by Andreas Theofilu on 05.05.18.
//

#ifndef __datetime_h__
#define __datetime_h__

#include <string>
#include <chrono>
#include "sunset.h"

class DateTime : public SunSet
{
	public:
		enum Season
		{
			WINTER,
			SPRING,
			SUMMER,
			AUTUM
		};

		/**
		 * Constructor - Creates a DateTime class initialized to the current
		 * date and time.
		 */
		DateTime();

		/**
		 * Constructor - Creates a DateTime class initialized to the current
		 * date and time.
		 *
		 * @param f
		 * The format of the string when the internal timestamp is transformed
		 * into a string. Default is "%Y-%m-%d %H:%M:%S%z".
		 */
		DateTime(std::string f);
		~DateTime() {}

		/**
		 * Returns the day.
		 *
		 * @return
		 * The day of the internal timestamp.
		 */
		int getDay();

		/**
		 * Returns the month.
		 *
		 * @return
		 * The month of the internal timestamp.
		 */
		int getMonth();

        /**
         * Returns the number of days in the month.
         *
         * @return
         * Number of days in current month.
         */
        int getMonthDays();

		/**
		 * Returns the year.
		 *
		 * @return
		 * The year of the internal timestamp.
		 */
		int getYear();

        /**
         * Returns the number of days in the year.
         *
         * @return
         * Number of days in current year.
         */
        int getYearDays();

		/**
		 * Returns the hour.
		 *
		 * @return
		 * The hour of the internal timestamp.
		 */
		int getHour();

		/**
		 * Returns the minute.
		 *
		 * @return
		 * The minute of the internal timestamp.
		 */
		int getMinute();

		/**
		 * Returns the second.
		 *
		 * @return
		 * The second of the internal timestamp.
		 */
		int getSecond();

		/**
		 * Returns the hundrets of a second.
		 *
		 * @return
		 * The hundrets of a second of the internal timestamp.
		 */
		int getHundrets();
		/**
		 * Returns the Day.
		 *
		 * @return
		 * The day of the internal timestamp.
		 */

		/**
		 * Returns the time zone.
		 *
		 * @return
		 * The timezone in hours.
		 */
		int getTimeZone();

		/**
		 * Returns allways the actual date and time independent from the
		 * internal timestamp.
		 *
		 * @return
		 * The actual time as a timestamp.
		 */
		std::chrono::system_clock::time_point getNow();

		/**
		 * Returns the internal timestamp as epoch. This means the number of
		 * seconds since 1970-01-01.
		 *
		 * @return
		 * The number of seconds since 1970-01-01.
		 */
		time_t getTimestamp();

		/**
		 * Returns the internal timestamp.
		 *
		 * @return
		 * The internal timestamp.
		 */
		std::chrono::system_clock::time_point getRawTime() { return timestamp; }

		/**
		 * Interval() - Adds or subtracts time from the internal date.
         * This function does not change the internal date!
		 *
		 * @param intval
		 * The interval as string command. Following commands are valid:
		 *   + <x> seconds|minutes|hours|days|weeks|months|years
		 *   - <x> seconds|minutes|hours|days|weeks|months|years
		 *
		 *   Where <x> is the number of seconds, minutes, ... to add or
		 *   subtract to.
		 *
		 * @return
		 * the calculated date
		 */
		std::chrono::system_clock::time_point Interval(std::string intval);

		/**
		 * This function returns the internal timestamp as a string in the
		 * set format. The default format is "%Y-%m-%d %H:%M:%S%z".
		 *
		 * @return
		 * The internal timestamp as a string.
		 */
		std::string toString() { return getTimestamp(format); }

		/**
		 * Returns the internal timestamp as a string in the given format.
		 *
		 * @param f
		 * The format of the timestamp. The following table shows valid
		 * format strings:
		 *
		 *   Conversion    Explanation
		 *   specifier
		 *      %          writes literal %. The full conversion specification must be %%.
		 *      n          writes newline character
		 *      t          writes horizontal tab character
		 *      Y          writes year as a decimal number, e.g. 2018
		 *      EY         writes year in the alternative representation,
		 *                 e.g. 平成23年 (year Heisei 23) instead of 2011年
		 *                 (year 2011) in ja_JP locale.
		 *      y          writes last 2 digits of year as a decimal number (range [00,99])
		 *      0y         writes last 2 digits of year using the alternative numeric system, e.g. 十一 instead of 11 in ja_JP locale
		 *      Ey         writes year as offset from locale's alternative calendar period %EC (locale-dependent)
		 *      C          writes first 2 digits of year as a decimal number (range [00,99])
		 *      EC         writes name of the base year (period) in the locale's alternative representation, e.g. 平成 (Heisei era) in ja_JP
		 *      G          writes ISO 8601 week-based year, i.e. the year that contains the specified week.
		 *                 In IS0 8601 weeks begin with Monday and the first week of the year must satisfy the following requirements:
		 *                     * Includes January 4
		 *                     * Includes first Thursday of the year
		 *      g          writes last 2 digits of ISO 8601 week-based year, i.e. the year that contains the specified week (range [00,99]).
		 *                 In IS0 8601 weeks begin with Monday and the first week of the year must satisfy the following requirements:
		 *                     * Includes January 4
		 *                     * Includes first Thursday of the year
		 *      b          writes abbreviated month name, e.g. Oct (locale dependent)
		 *      h          synonym of b
		 *      B          writes full month name, e.g. October (locale dependent)
		 *      m          writes month as a decimal number (range [01,12])
		 *      0m         writes month using the alternative numeric system, e.g. 十二 instead of 12 in ja_JP locale
		 *      U          writes week of the year as a decimal number (Sunday is the first day of the week) (range [00,53])
		 *      OU         writes week of the year, as by %U, using the alternative numeric system, e.g. 五十二 instead of 52 in ja_JP locale
		 *      W          writes week of the year as a decimal number (Monday is the first day of the week) (range [00,53])
		 *      OW         writes week of the year, as by %W, using the alternative numeric system, e.g. 五十二 instead of 52 in ja_JP locale
		 *      V          writes ISO 8601 week of the year (range [01,53]).
		 *                 I n IS0 8601 weeks begin with Monday* and the first week of the year must satisfy the following requirements:
		 *                    * Includes January 4
		 *                    * Includes first Thursday of the year
		 *      OV         writes week of the year, as by %V, using the alternative numeric system, e.g. 五十二 instead of 52 in ja_JP locale
		 *      j          writes day of the year as a decimal number (range [001,366])
		 *      d          writes day of the month as a decimal number (range [01,31])
		 *      Od         writes zero-based day of the month using the alternative numeric system, e.g 二十七 instead of 23 in ja_JP locale.
		 *                 Single character is preceded by a space.
		 *      e          writes day of the month as a decimal number (range [1,31]).
		 *                 Single digit is preceded by a space.
		 *      Oe         writes one-based day of the month using the alternative numeric system, e.g. 二十七 instead of 27 in ja_JP locale
		 *                 Single character is preceded by a space.
		 *      a          writes abbreviated weekday name, e.g. Fri (locale dependent)
		 *      A          writes full weekday name, e.g. Friday (locale dependent)
		 *      w          writes weekday as a decimal number, where Sunday is 0 (range [0-6])
		 *      Ow         writes weekday, where Sunday is 0, using the alternative numeric system, e.g. 二 instead of 2 in ja_JP locale
		 *      u          writes weekday as a decimal number, where Monday is 1 (ISO 8601 format) (range [1-7])
		 *      Ou         writes weekday, where Monday is 1, using the alternative numeric system, e.g. 二 instead of 2 in ja_JP locale
		 *      H          writes hour as a decimal number, 24 hour clock (range [00-23])
		 *      OH         writes hour from 24-hour clock using the alternative numeric system, e.g. 十八 instead of 18 in ja_JP locale
		 *      I          writes hour as a decimal number, 12 hour clock (range [01,12])
		 *      OI         writes hour from 12-hour clock using the alternative numeric system, e.g. 六 instead of 06 in ja_JP locale
		 *      M          writes minute as a decimal number (range [00,59])
		 *      OM         writes minute using the alternative numeric system, e.g. 二十五 instead of 25 in ja_JP locale
		 *      S          writes second as a decimal number (range [00,60])
		 *      OS         writes second using the alternative numeric system, e.g. 二十四 instead of 24 in ja_JP locale
		 *      c          writes standard date and time string, e.g. Sun Oct 17 04:41:13 2010 (locale dependent)
		 *      Ec         writes alternative date and time string, e.g. using 平成23年 (year Heisei 23) instead of 2011年 (year 2011) in ja_JP locale
		 *      x          writes localized date representation (locale dependent)
		 *      Ex         writes alternative date representation, e.g. using 平成23年 (year Heisei 23) instead of 2011年 (year 2011) in ja_JP locale
		 *      X          writes localized time representation (locale dependent)
		 *      EX         writes alternative time representation (locale dependent)
		 *      D          equivalent to "%m/%d/%y"
		 *      F          equivalent to "%Y-%m-%d" (the ISO 8601 date format)
		 *      r          writes localized 12-hour clock time (locale dependent)
		 *      R          equivalent to "%H:%M"
		 *      T          equivalent to "%H:%M:%S" (the ISO 8601 time format)
		 *      p          writes localized a.m. or p.m. (locale dependent)
		 *      z          writes offset from UTC in the ISO 8601 format (e.g. -0430), or no characters if the time zone information is not available
		 *      Z          writes time zone name or abbreviation, or no characters if the time zone information is not available (locale dependent)
		 *
		 * @return
		 * the internal timestamp formated as string.
		 */
		std::string getTimestamp(std::string f);

		/**
		 * Converts the given timestamp into a string. To format the string it
		 * uses the internal saved format string.
		 * This function does not change the internal saved timestamp.
		 *
		 * @param t
		 * The timestamp to convert.
		 *
		 * @return
		 * The timestamp as a string.
		 */
		std::string getTimestamp(const std::chrono::system_clock::time_point& t);

		/**
		 * This function returns the internal format string.
		 *
		 * @return
		 * The internal format string.
		 */
		std::string getFormat() { return format; }

		/**
		 * This function returns the epoch of the internal date. This are the
		 * seconds since 01/01/1970.
		 *
		 * @return
		 * The epoch.
		 */
		time_t getEpoch();

		/**
		 * Sets the format string.
		 *
		 * @param f
		 * Sets the internal format string. Look at function getTimestamp(f) to
		 * get a list of available parameters.
		 */
		void setFormat(std::string f) { format = f; };

		/**
		 * Sets the internal timestamp.
		 *
		 * @param t
		 * A timestamp to set the internal timestamp.
		 */
		void setTimestamp(std::chrono::system_clock::time_point t) { timestamp = t; }

		/**
		 * Sets the internal timestamp.
		 *
		 * @param tm
		 * This string must be formated in ISO8601 format.
		 */
		void setTimestamp(std::string tm);

		/**
		 * Sets the internal timestamp.
		 *
		 * @param tm
		 * This string must be formated in ISO8601 format.
		 */
		void setTimestamp(char *tm);

		/**
		 * Sets the internal timestamp.
		 *
		 * @param year
		 * The year e.g. 2018
		 *
		 * @param month
		 * The month in the range from 1 to 12. This parameter is optional. If
		 * omitted it is 1.
		 *
		 * @param day
		 * The day in the range from 1 to 31. This parameter is optional. If
		 * omitted it is 1.
		 *
		 * @param hour
		 * The hour in the range from 0 to 23. This parameter is optional. If
		 * omitted it is 1.
		 *
		 * @param minute
		 * The minute in the range from 0 to 59. This parameter is optional. If
		 * omitted it is 1.
		 *
		 * @param second
		 * The second in the range from 0 to 59. This parameter is optional. If
		 * omitted it is 1.
		 *
		 * @param tz
		 * The time zone in hours in the range from 0 to 24. This parameter is optional. If
		 * omitted it is 1.
		 */
		void setTimestamp(int year, int month=1, int day=1, int hour=0, int minute=0, int second=0, int tz=0);

		/**
		 * This function sets the internal timestamp to the actual date and time.
		 */
		void setNow() { timestamp = std::chrono::system_clock::now(); }

		/**
		 * This function calculates the sunrise and returns it.
		 *
		 * @param la
		 * The latitude of the location.
		 *
		 * @param lo
		 * The longitude of the location.
		 *
		 * @return
		 * Returns the timestamp of where the sun starts rising.
		 */
		DateTime sunrise(double la, double lo);

		/**
		 * This function calculates the sunset and returns it.
		 *
		 * @param la
		 * The latitude of the location.
		 *
		 * @param lo
		 * The longitude of the location.
		 *
		 * @return
		 * Returns the timestamp of where the sun starts rising.
		 */
		DateTime sunset(double la, double lo);

		/**
		 * Tetermines the season of the year
		 *
		 * @return
		 * Enum containing the season of the year
		 */
		Season getSeason();

		/**
		 * Compares only the date. If it is the same day, regardless of the time,
		 * it returns true.
		 *
		 * @param dt1
		 * First timestamp to compare.
		 *
		 * @param dt2
		 * Second timestamp to compare.
		 */
		bool compareDate(DateTime dt);

		/**
		 * Returns the moon age. The method calculates the age of the moon from
		 * the internal date.
		 *
		 * @return
		 * A number between 0 to 30, where 0 or 30 is "new moon" and 15 is "full
		 * moon". A value less than 15 means waxing moon and value above 15
		 * means waning moon.
		 */
		int moonAge() { return moonPhase(getEpoch()); }

		// overwritten operators
		std::ostringstream operator<< (DateTime& t);
		std::ostringstream operator>> (DateTime& t);
		DateTime& operator= (DateTime& t);
		DateTime& operator= (DateTime& t) const;
		bool operator== (DateTime& t) const;
		bool operator!= (DateTime& t) const;
		DateTime& operator+ (DateTime& t);
		DateTime& operator- (DateTime& t);
		DateTime& operator+= (DateTime& t);
		DateTime& operator-= (DateTime& t);
		bool operator< (DateTime& t) const;
		bool operator<= (DateTime& t) const;
		bool operator> (DateTime& t) const;
		bool operator>= (DateTime& t) const;

	private:
		std::string doFormat(const struct tm *t, std::string f);
		int checkDayOfMonth(int mon, int year);

		std::string format;
		std::chrono::system_clock::time_point timestamp;
};

#endif /* datetime_h */
