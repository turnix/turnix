/*
 * Copyright (c) 2015 Changli Gao <xiaosuo@gmail.com>
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <time.h>
#include <timer.h>
#include <stddef.h>
#include <assert.h>

static const int days_to_mon[] = {
	31,
	31 + 28,
	31 + 28 + 31,
	31 + 28 + 31 + 30,
	31 + 28 + 31 + 30 + 31,
	31 + 28 + 31 + 30 + 31 + 30,
	31 + 28 + 31 + 30 + 31 + 30 + 31,
	31 + 28 + 31 + 30 + 31 + 30 + 31 + 31,
	31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
	31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
	31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30
};

time_t mktime(struct tm *tm)
{
	int curr_year = 1900 + tm->tm_year;
	int year;
	long days = 0;

	for (year = 1970; year < curr_year; ++year)
		days += 365 + is_leapyear(year);
	if (tm->tm_mon > 0) {
		days += days_to_mon[tm->tm_mon - 1];
		if (tm->tm_mon > 1 && is_leapyear(curr_year))
			++days;
	}
	days += tm->tm_mday - 1;

	return days * SECS_PER_DAY + tm->tm_hour * SECS_PER_HOUR +
		tm->tm_min * SECS_PER_MIN + tm->tm_sec;
}

struct tm *gmtime_r(const time_t *timep, struct tm *result)
{
	long n, days;
	int year, days_year;
	bool leapyear;

	assert(timep);
	assert(result);

	n = *timep;
	days = n / SECS_PER_DAY;
	result->tm_wday = (days + 4) % 7;
	n %= SECS_PER_DAY;
	result->tm_hour = n / SECS_PER_HOUR;
	n %= SECS_PER_HOUR;
	result->tm_min = n / SECS_PER_MIN;
	result->tm_sec = n % SECS_PER_MIN;

	for (year = 1970; ; ++year) {
		leapyear = is_leapyear(year);
		days_year = 365 + leapyear;
		if (days_year > days)
			break;
		days -= days_year;
	}
	result->tm_year = year - 1900;

	for (result->tm_mon = 0; result->tm_mon < 11; result->tm_mon++) {
		if (days_to_mon[result->tm_mon] +
		    (leapyear && result->tm_mon > 0) > days) {
			break;
		}
	}
	if (result->tm_mon > 0) {
		days -= days_to_mon[result->tm_mon - 1] +
			(leapyear && result->tm_mon > 1);
	}

	result->tm_mday = days + 1;

	return result;
}

struct tm *localtime_r(const time_t *timep, struct tm *result)
{
	time_t sec = *timep - wall_clock.tz.tz_minuteswest * SECS_PER_MIN;

	return gmtime_r(&sec, result);
}
