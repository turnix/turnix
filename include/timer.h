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

#ifndef TIMER_H
#define TIMER_H

#include <config.h>
#include <stddef.h>
#include <unistd.h>
#include <time.h>

#define time_after(a, b)	((long)((b) - (a)) < 0)
#define time_before(a, b)	time_after(b, a)

#ifndef CONFIG_HZ
#define CONFIG_HZ 100
#endif

#define TIMER_INVALID_INDEX	((size_t)-1l)

struct timer {
	unsigned long	expires;
	size_t		i;
	void		(*func)(struct timer *timer);
};

struct wall_clock {
	struct timeval	tv;
	struct timezone	tz;
};

extern volatile unsigned long ticks;

enum {
	TICKS_PER_SEC	= CONFIG_HZ,
	MSECS_PER_TICK	= MSECS_PER_SEC / CONFIG_HZ,
	NSECS_PER_TICK	= NSECS_PER_SEC / CONFIG_HZ,
};

extern struct wall_clock wall_clock;

void timer_update(void);

void timer_add(struct timer *timer);

void timer_delete(struct timer *timer);

long schedule_timeout(unsigned long timeout);

#endif  /* TIMER_H */
