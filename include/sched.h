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

#ifndef SCHED_H
#define SCHED_H

#include <errno.h>

enum {
	SCHED_RR,
};

enum {
	SCHED_RR_PRIORITY_MIN = 0,
	SCHED_RR_PRIORITY_IDLE = SCHED_RR_PRIORITY_MIN,
	SCHED_RR_PRIORITY_DEFAULT = 16,
	SCHED_RR_PRIORITY_MAX = 31,
};

struct sched_param {
	int sched_priority;
};

static inline int sched_get_priority_min(int policy)
{
	if (policy != SCHED_RR) {
		errno = EINVAL;
		return -1;
	}

	return SCHED_RR_PRIORITY_MIN;
}

static inline int sched_get_priority_max(int policy)
{
	if (policy != SCHED_RR) {
		errno = EINVAL;
		return -1;
	}

	return SCHED_RR_PRIORITY_MAX;
}

#endif  /* SCHED_H */
