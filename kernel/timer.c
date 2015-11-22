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

#include <config.h>
#include <timer.h>
#include <pthread.h>
#include <interrupt.h>
#include <kernel.h>

#include <sys/param.h>

#ifndef CONFIG_TIMER_MAX_NUM
#define CONFIG_TIMER_MAX_NUM 32
#endif

#ifndef CONFIG_TZ
#define CONFIG_TZ (-480)
#endif

volatile unsigned long ticks = 0;

struct wall_clock wall_clock = {
	.tz = {
		.tz_minuteswest = CONFIG_TZ,
	}
};

static struct timeval monotonic_clock;

static struct {
	struct timer	*heap[CONFIG_TIMER_MAX_NUM];
	size_t		n;
} timer_context;

static void heap_swap(size_t i, size_t j)
{
	struct timer *tmp;

	tmp = timer_context.heap[i];
	timer_context.heap[i] = timer_context.heap[j];
	timer_context.heap[j] = tmp;
	timer_context.heap[i]->i = i;
	timer_context.heap[j]->i = j;
}

static void heap_pull_up(size_t i)
{
	size_t p;

	while (i > 0) {
		p = (i - 1) / 2;
		if (!time_before(timer_context.heap[i]->expires,
				 timer_context.heap[p]->expires)) {
			break;
		}
		heap_swap(i, p);
		i = p;
	}
}

static void heap_push_down(size_t i)
{
	size_t c, smallest;

	assert(i < timer_context.n);

	for (;;) {
		smallest = i;
		c = i * 2 + 1;
		if (c >= timer_context.n)
			break;
		if (time_after(timer_context.heap[smallest]->expires,
			       timer_context.heap[c]->expires)) {
			smallest = c;
		}
		if (++c < timer_context.n) {
			if (time_after(timer_context.heap[smallest]->expires,
				       timer_context.heap[c]->expires)) {
				smallest = c;
			}
		}
		if (i == smallest)
			break;
		heap_swap(i, smallest);
		i = smallest;
	}
}

static void heap_adjust(size_t i)
{
	assert(i < timer_context.n);

	if (i > 0) {
		size_t p = (i - 1) / 2;

		if (time_after(timer_context.heap[p]->expires,
			       timer_context.heap[i]->expires)) {
			heap_pull_up(i);
			return;
		}
	}
	heap_push_down(i);
}

void timer_add(struct timer *timer)
{
	size_t i;
	unsigned long flags = interrupt_disable();

	assert(timer_context.n < ARRAY_SIZE(timer_context.heap));

	i = timer_context.n++;
	timer_context.heap[i] = timer;
	timer_context.heap[i]->i = i;
	heap_pull_up(i);
	interrupt_enable(flags);
}

static void __timer_delete(struct timer *timer)
{
	size_t i = timer->i;

	if (i != --timer_context.n) {
		timer_context.heap[i] = timer_context.heap[timer_context.n];
		timer_context.heap[i]->i = i;
		heap_adjust(i);
	}
	timer->i = TIMER_INVALID_INDEX;
}

void timer_delete(struct timer *timer)
{
	unsigned long flags = interrupt_disable();

	if (timer->i != TIMER_INVALID_INDEX)
		__timer_delete(timer);
	interrupt_enable(flags);
}

struct sched_timer {
	struct timer	timer;
	pthread_t	thread;
};

static void sched_timeout(struct timer *timer)
{
	struct sched_timer *sched_timer;

	sched_timer = container_of(timer, struct sched_timer, timer);
	wake_up(sched_timer->thread);
}

long schedule_timeout(unsigned long timeout)
{
	struct sched_timer t;

	assert(!in_irq);

	t.timer.expires = ticks + timeout;
	t.timer.func = sched_timeout;
	t.thread = pthread_current;
	timer_add(&t.timer);
	schedule();
	timer_delete(&t.timer);

	return t.timer.expires - ticks;
}

unsigned int sleep(unsigned int seconds)
{
	long remain;

	pthread_current->state = PTHREAD_STATE_SLEEPING;
	remain = schedule_timeout(seconds * CONFIG_HZ);
	if (remain <= 0)
		remain =  0;
	else
		remain = howmany(remain, CONFIG_HZ);

	return remain;
}

int usleep(useconds_t usec)
{
	pthread_current->state = PTHREAD_STATE_SLEEPING;
	schedule_timeout(howmany(usec, USECS_PER_SEC / CONFIG_HZ));

	return 0;
}

static void update_timeval(struct timeval *tv)
{
	tv->tv_usec += USECS_PER_SEC / CONFIG_HZ;
	if (tv->tv_usec > USECS_PER_SEC) {
		tv->tv_sec += tv->tv_usec / USECS_PER_SEC;
		tv->tv_usec = tv->tv_usec % USECS_PER_SEC;
	}
}

void timer_update(void)
{
	unsigned long now = ++ticks;

	update_timeval(&wall_clock.tv);
	update_timeval(&monotonic_clock);

	while (timer_context.n > 0 &&
	       !time_after(timer_context.heap[0]->expires, now)) {
		struct timer *timer = timer_context.heap[0];

		__timer_delete(timer);
		timer->func(timer);
	}

#if CONFIG_RR
	if (--pthread_current->timeslice == 0)
		pthread_next = NULL;
#endif
	pthread_current->ticks++;
}

time_t time(time_t *t)
{
	if (t) {
		*t = wall_clock.tv.tv_sec;

		return *t;
	} else {
		return wall_clock.tv.tv_sec;
	}
}

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
	unsigned long flags = interrupt_disable();

	if (tv)
		*tv = wall_clock.tv;
	if (tz)
		*tz = wall_clock.tz;
	interrupt_enable(flags);

	return 0;
}

int settimeofday(const struct timeval *tv, const struct timezone *tz)
{
	unsigned long flags = interrupt_disable();

	if (tv)
		wall_clock.tv = *tv;
	if (tz)
		wall_clock.tz = *tz;
	interrupt_enable(flags);

	return 0;
}

void uptime(struct timeval *tv)
{
	unsigned long flags = interrupt_disable();

	assert(tv);
	*tv = monotonic_clock;
	interrupt_enable(flags);
}

int clock_gettime(clockid_t clk_id, struct timespec *tp)
{
	int retval = 0;
	unsigned long flags = interrupt_disable();

	switch (clk_id) {
	case CLOCK_REALTIME:
		tp->tv_sec = wall_clock.tv.tv_sec;
		tp->tv_nsec = wall_clock.tv.tv_usec * NSECS_PER_USEC;
		break;
	case CLOCK_MONOTONIC:
		tp->tv_sec = monotonic_clock.tv_sec;
		tp->tv_nsec = monotonic_clock.tv_usec * NSECS_PER_USEC;
		break;
	default:
		errno = EINVAL;
		retval = -1;
		break;
	}
	interrupt_enable(flags);

	return retval;
}

int clock_settime(clockid_t clk_id, const struct timespec *tp)
{
	int retval = 0;
	unsigned long flags = interrupt_disable();

	switch (clk_id) {
	case CLOCK_REALTIME:
		wall_clock.tv.tv_sec = tp->tv_sec;
		wall_clock.tv.tv_usec = tp->tv_nsec / NSECS_PER_USEC;
		break;
	case CLOCK_MONOTONIC:
	default:
		errno = EINVAL;
		retval = -1;
		break;
	}
	interrupt_enable(flags);

	return retval;
}
