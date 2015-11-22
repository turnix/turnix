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

#ifndef TIME_H
#define TIME_H

#include <stdbool.h>

#include <sys/time.h>

enum {
	SECS_PER_MIN	= 60,
	SECS_PER_HOUR	= SECS_PER_MIN * 60,
	SECS_PER_DAY	= SECS_PER_HOUR * 24,
	NSECS_PER_USEC	= 1000,
	USECS_PER_MSEC	= 1000,
	MSECS_PER_SEC	= 1000,
	USECS_PER_SEC	= USECS_PER_MSEC * MSECS_PER_SEC,
	NSECS_PER_SEC	= NSECS_PER_USEC * USECS_PER_SEC
};

enum clockid {
	CLOCK_REALTIME,
	CLOCK_MONOTONIC
};

typedef enum clockid clockid_t;

/* It is a simplified version */
struct tm {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
};

struct timespec {
	time_t	tv_sec;
	long	tv_nsec;
};

static inline bool is_leapyear(int year)
{
	return (year % 4) == 0 && ((year % 100) != 0 || (year % 400) == 0);
}

time_t time(time_t *t);
time_t mktime(struct tm *tm);
struct tm *gmtime_r(const time_t *timep, struct tm *result);
struct tm *localtime_r(const time_t *timep, struct tm *result);
void uptime(struct timeval *tv);
int clock_gettime(clockid_t clk_id, struct timespec *tp);
int clock_settime(clockid_t clk_id, const struct timespec *tp);

#endif  /* TIME_H */
