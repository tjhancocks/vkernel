/*
  Copyright (c) 2018-2019 Tom Hancocks
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
 */

#include <time.h>
#include <print.h>

int64_t current_timestamp = 0;

////////////////////////////////////////////////////////////////////////////////

int64_t time(void)
{
	return current_timestamp;
}

////////////////////////////////////////////////////////////////////////////////

#define EPOCH_YEAR	1970

static inline bool is_leap_year(int32_t y)
{
	return (y % 4 == 0) && (!(y % 100 == 0) || (y % 400 == 0));
}

static const int32_t __days_lookup[24] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, /* Normal Year */
	31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, /* Leap Year */
};

static inline int32_t days_in_month(int32_t year, int32_t month)
{
	return __days_lookup[month + (is_leap_year(year) ? 12 : 0)];
}

static inline int32_t days_in_year(int32_t year)
{
	int32_t days = 0;
	for (int32_t month = 0; month < 12; ++month) 
		days += days_in_month(year, month);
	return days;
}

int64_t make_timestamp(
	int32_t year, int32_t month, int32_t day, 
	int32_t hour, int32_t min, int32_t second
) {
	/* Calculate the number of seconds that are represented by h:m:s */
	int64_t seconds = second + (min * 60) + (hour * 3600);

	/* Calculate the number of days since/to the epoch. */
	int64_t total_days = 0;
	int32_t current_year = year;
	year -= EPOCH_YEAR;
	--month;
	--day;

	if (year < 0) {
		/* TODO: working out number of days until... */
		klogc(
			swarn, "Attempted to make timestamp for before 1970. Returning 0.\n"
		);
		return 0;
	}
	else {
		/* We're working out number of days since... */
		for (int32_t i = 0; i < year; ++i)
			total_days += days_in_year(EPOCH_YEAR + i);

		for (int32_t i = 0; i < month; ++i)
			total_days += days_in_month(current_year, i);

		total_days += day;

		/* Add the number of days on to the seconds */
		seconds += total_days * 86400;
	}

	return seconds;
}
