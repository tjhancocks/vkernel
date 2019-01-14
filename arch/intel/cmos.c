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


#if (__i386__ || __x86_64__)

#include <arch/intel/intel.h>
#include <bcd.h>
#include <print.h>
#include <time.h>

////////////////////////////////////////////////////////////////////////////////

#define CURRENT_YEAR	2019

#define CMOS_ADDRESS	0x70
#define CMOS_DATA		0x71

static struct {
	bool bcd_encoded;
	bool is_24;
} cmos;

////////////////////////////////////////////////////////////////////////////////

static inline bool cmos_updating(void)
{
	outb(CMOS_ADDRESS, cmos_rtc_sts_a);
	return inb(CMOS_DATA) & 0x80;
}

static inline uint8_t cmos_decode(uint8_t value)
{
	return cmos.bcd_encoded ? bcd_decode(value) : value;
}

////////////////////////////////////////////////////////////////////////////////

static inline uint32_t cmos_read(enum cmos_reg *r)
{
	outb(CMOS_ADDRESS, r);
	return inb(CMOS_DATA);
}

static int64_t cmos_read_time(void)
{
	uint8_t second, minute, hour, day, month;
	uint16_t year;
	uint8_t lsecond, lminute, lhour, lday, lmonth;
	uint16_t lyear;

	/* Read initial values */
	while (cmos_updating()) nop;
	second = cmos_read(cmos_rtc_seconds);
	minute = cmos_read(cmos_rtc_minutes);
	hour = cmos_read(cmos_rtc_hours);
	day = cmos_read(cmos_rtc_day);
	month = cmos_read(cmos_rtc_month);
	year = cmos_read(cmos_rtc_year);

	/* Update the values */
	do {
		/* Store previous values */
		lsecond = second;
		lminute = minute;
		lhour = hour;
		lday = day;
		lmonth = month;
		lyear = year;

		/* Read new values */
		while (cmos_updating()) nop;
		second = cmos_read(cmos_rtc_seconds);
		minute = cmos_read(cmos_rtc_minutes);
		hour = cmos_read(cmos_rtc_hours);
		day = cmos_read(cmos_rtc_day);
		month = cmos_read(cmos_rtc_month);
		year = cmos_read(cmos_rtc_year);
	} while (
		(second != lsecond) || (minute != lminute) || (hour != lhour) ||
		(day != lday) || (month != lmonth) || (year != lyear)
	);

	/* Perform any BCD conversions required, and then convert to a UNIX 
	   epoch timestamp */
	second = cmos_decode(second);
	minute = cmos_decode(minute);
	hour = cmos_decode(hour);
	day = cmos_decode(day);
	month = cmos_decode(month);
	year = cmos_decode(year);

	year += (year >= 90) ? 1900 : 2000;

	if (cmos.is_24 && (hour & 0x80)) {
		hour = ((hour & 0x7F) + 12) % 24;
	}

	/* Calculate the time in seconds. */
	int64_t epoch = make_timestamp(year, month, day, hour, minute, second);
	return epoch;
}

////////////////////////////////////////////////////////////////////////////////

extern int64_t current_timestamp;

void init_cmos(void)
{
	/* Determine what characteristics the RTC registers have */
	uint8_t flags = cmos_read(cmos_rtc_sts_b);
	cmos.is_24 = (flags & 0x02) ? true : false;
	cmos.bcd_encoded = (flags & 0x04) ? false : true;

	/* Read the initial date information. From this we need to determine
	   the current time. */
	current_timestamp = cmos_read_time();
	klog("The current time is: %lld\n", current_timestamp);
}

#endif