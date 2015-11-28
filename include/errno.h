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

#ifndef ERRNO_H
#define ERRNO_H

#include <config.h>
#include <stddef.h>
#include <stdint.h>
#include <arch.h>

#include <sys/queue.h>

enum {
	EBUSY = 1,
	EINVAL,
	EAGAIN,
	ERANGE,
	EDEADLK,
	ESRCH,
	ETIMEDOUT,
};

enum pthread_state {
	PTHREAD_STATE_NONE,
	PTHREAD_STATE_INIT,
	PTHREAD_STATE_RUNNING,
	PTHREAD_STATE_SLEEPING,
	PTHREAD_STATE_EXIT,
};

enum {
	PTHREAD_NAME_SIZE = 16
};

struct pthread {
	struct arch_context	context;
	enum pthread_state	state;
	void			*retval;
	void			*stack_addr;
	size_t			stack_size;
	TAILQ_ENTRY(, pthread)	link;
	uint8_t			priority;
#if CONFIG_RR
	uint8_t			timeslice;
#endif
	uint8_t			flags;
	char			name[PTHREAD_NAME_SIZE];
	struct pthread		*waiter;
	int			error_code;
	unsigned long		ticks;
};

extern struct pthread *pthread_current;

#define errno (pthread_current->error_code)

#endif  /* ERRNO_H */
