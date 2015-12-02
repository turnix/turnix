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
#include <pthread.h>
#include <interrupt.h>
#include <kernel.h>
#include <stdlib.h>
#include <stringify.h>
#include <strings.h>
#include <string.h>
#include <timer.h>
#include <arch.h>

#ifndef CONFIG_PTHREAD_MAX_NUM
#define CONFIG_PTHREAD_MAX_NUM 32
#endif

#ifndef CONFIG_TIMESLICE
#define CONFIG_TIMESLICE 10
#endif

static struct pthread pthreads[CONFIG_PTHREAD_MAX_NUM];

static struct pthread pthread_idle;

static struct {
	struct pthread_queue	level[SCHED_RR_PRIORITY_MAX + 1];
	int			bitmap;
} run_queue;

static void run_queue_init(void)
{
	size_t i;

	for (i = 0; i < ARRAY_SIZE(run_queue.level); ++i)
		TAILQ_INIT(&run_queue.level[i]);
	run_queue.bitmap = 0;
}

static void run_queue_enqueue(pthread_t thread)
{
	struct pthread_queue *q = &run_queue.level[thread->priority];

	if (TAILQ_EMPTY(q)) {
		int i = SCHED_RR_PRIORITY_MAX - thread->priority;

		run_queue.bitmap |= (1 << i);
	}
#if CONFIG_RR
	thread->timeslice = CONFIG_TIMESLICE;
#endif
	TAILQ_INSERT_TAIL(&run_queue.level[thread->priority], thread, link);
}

static pthread_t run_queue_peek(void)
{
	struct pthread_queue *q;
	pthread_t th;
	int i = ffs(run_queue.bitmap);

	assert(i);
	--i;
	q = &run_queue.level[SCHED_RR_PRIORITY_MAX - i];
	th = TAILQ_FIRST(q);
	assert(th);

	return th;
}

static void run_queue_dequeue(pthread_t thread)
{
	if (!TAILQ_ENTRY_EMPTY(&thread->link)) {
		struct pthread_queue *q = &run_queue.level[thread->priority];

		TAILQ_REMOVE(q, thread, link);
		TAILQ_ENTRY_INIT(&thread->link);
		if (TAILQ_EMPTY(q)) {
			int i = SCHED_RR_PRIORITY_MAX - thread->priority;

			run_queue.bitmap &= ~(1 << i);
		}
	}
}

int pthread_setschedprio(pthread_t thread, int priority)
{
	if (thread->priority != priority) {
		unsigned long flags = interrupt_disable();

		if (thread->priority != priority) {
			if (!TAILQ_ENTRY_EMPTY(&thread->link)) {
				run_queue_dequeue(thread);
				thread->priority = priority;
				run_queue_enqueue(thread);
				schedule();
			} else {
				thread->priority = priority;
			}
		}
		interrupt_enable(flags);
	}

	return 0;
}

pthread_t pthread_current;
pthread_t pthread_next;

extern unsigned long idle_stack_bottom;

void pthread_init(void)
{
	pthread_next = pthread_current = &pthread_idle;
	run_queue_init();

	pthread_idle.state = PTHREAD_STATE_RUNNING;
	pthread_idle.stack_addr = &idle_stack_bottom;
	pthread_idle.stack_size = CONFIG_IDLE_STACK_SIZE;
	TAILQ_ENTRY_INIT(&pthread_idle.link);
	pthread_idle.priority = SCHED_RR_PRIORITY_IDLE;
#if CONFIG_RR
	pthread_idle.timeslice = CONFIG_TIMESLICE;
#endif
	pthread_idle.flags = PTHREAD_FLAG_DETACH;
	strcpy(pthread_idle.name, "idle");
	pthread_idle.waiter = NULL;
	pthread_idle.error_code = 0;
	pthread_idle.stime.tv_sec = 0;
	pthread_idle.stime.tv_usec = 0;
	run_queue_enqueue(&pthread_idle);
}

void __schedule(void)
{
	if (pthread_current->state != PTHREAD_STATE_RUNNING) {
		run_queue_dequeue(pthread_current);
#if CONFIG_RR
	} else if (pthread_current->timeslice == 0) {
		run_queue_dequeue(pthread_current);
		run_queue_enqueue(pthread_current);
#endif
	}

	pthread_next = run_queue_peek();
}

void schedule(void)
{
	unsigned long flags;

	if (in_irq) {
		pthread_next = NULL;

		return;
	}

	flags = interrupt_disable();
	__schedule();
	if (pthread_next != pthread_current)
		arch_context_switch();
	interrupt_enable(flags);
}

static void __pthread_set_running(pthread_t th)
{
	unsigned long flags = interrupt_disable();

	th->state = PTHREAD_STATE_RUNNING;
	if (TAILQ_ENTRY_EMPTY(&th->link))
		run_queue_enqueue(th);
	interrupt_enable(flags);
}

void wake_up(pthread_t th)
{
	__pthread_set_running(th);
	schedule();
}

void __pthread_exit(void)
{
	interrupt_disable();
	if (pthread_current->flags & PTHREAD_FLAG_DETACH) {
		pthread_current->state = PTHREAD_STATE_NONE;
	} else {
		pthread_current->state = PTHREAD_STATE_EXIT;
		if (pthread_current->waiter)
			wake_up(pthread_current->waiter);
	}
	schedule();
}

int pthread_detach(pthread_t thread)
{
	int retval = 0;
	unsigned int flags = interrupt_disable();

	switch (thread->state) {
	case PTHREAD_STATE_NONE:
	case PTHREAD_STATE_INIT:
		retval = ESRCH;
		break;
	case PTHREAD_STATE_RUNNING:
	case PTHREAD_STATE_SLEEPING:
		if (thread->flags & PTHREAD_FLAG_DETACH) {
			retval = EINVAL;
		} else {
			thread->flags |= PTHREAD_FLAG_DETACH;
			if (thread->waiter)
				wake_up(thread->waiter);
		}
		break;
	case PTHREAD_STATE_EXIT:
		thread->state = PTHREAD_STATE_NONE;
		break;
	default:
		assert(0);
	}
	interrupt_enable(flags);

	return retval;
}

static inline bool has_loop(pthread_t th)
{
	pthread_t i;

	for (i = th->waiter; i; i = i->waiter) {
		if (i == th)
			return true;
	}

	return false;
}

int pthread_join(pthread_t thread, void **retval)
{
	int ret;
	unsigned long flags = interrupt_disable();

	for (;;) {
		if (thread->state == PTHREAD_STATE_EXIT) {
			if (retval)
				*retval = thread->retval;
			thread->state = PTHREAD_STATE_NONE;
			ret = 0;
			break;
		}
		if (thread->state == PTHREAD_STATE_NONE) {
			ret = ESRCH;
			break;
		}
		if ((thread->flags & PTHREAD_FLAG_DETACH) || thread->waiter) {
			ret = EINVAL;
			break;
		}
		thread->waiter = pthread_current;
		if (has_loop(thread)) {
			ret = EDEADLK;
			thread->waiter = NULL;
			break;
		}
		pthread_current->state = PTHREAD_STATE_SLEEPING;
		schedule();
		thread->waiter = NULL;
	}
	interrupt_enable(flags);

	return ret;
}

void pthread_exit(void *retval)
{
	pthread_current->retval = retval;
	__pthread_exit();
}

static void __start_routine(void *(*start_routine)(void *), void *arg)
{
	pthread_exit(start_routine(arg));
}

static inline void stack_check_init(void *stack_addr, size_t stack_size)
{
	unsigned long *addr = stack_addr;

	stack_size /= sizeof(long);
	while (stack_size-- > 0)
		*addr++ = STACK_FILL;
}

size_t stack_check_size(pthread_t th)
{
	unsigned long *used = th->stack_addr;
	unsigned long *top = th->stack_addr + th->stack_size;

	while (used < top && *used == STACK_FILL)
		++used;

	return (top - used) * sizeof(long);
}

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
		   void *(*start_routine)(void *), void *arg)
{
	unsigned int i;
	unsigned long flags;
	pthread_t th;

	flags = interrupt_disable();
	for (i = 0; i < ARRAY_SIZE(pthreads); ++i) {
		if (pthreads[i].state == PTHREAD_STATE_NONE) {
			pthreads[i].state = PTHREAD_STATE_INIT;
			break;
		}
	}
	interrupt_enable(flags);
	if (i == ARRAY_SIZE(pthreads))
		return EAGAIN;
	th = &pthreads[i];
	th->retval = NULL;
	stack_check_init(attr->stack_addr, attr->stack_size);
	th->stack_addr = attr->stack_addr;
	th->stack_size = attr->stack_size;
	TAILQ_ENTRY_INIT(&th->link);
	th->priority = attr->priority;
#if CONFIG_RR
	th->timeslice = CONFIG_TIMESLICE;
#endif
	th->flags = attr->flags;
	strcpy(th->name, "unnamed");
	th->waiter = NULL;
	th->error_code = 0;
	th->stime.tv_sec = 0;
	th->stime.tv_usec = 0;
	arch_pthread_init(th, __start_routine, start_routine, arg);
	wake_up(th);
	*thread = th;

	return 0;
}

int pthread_yield(void)
{
	schedule();

	return 0;
}

int pthread_getname_np(pthread_t thread, char *buf, size_t size)
{
	int retval = 0;
	size_t len = strlen(thread->name);
	unsigned long flags = interrupt_disable();

	if (len >= size)
		retval = ERANGE;
	else
		memcpy(buf, thread->name, len + 1);

	interrupt_enable(flags);

	return retval;
}

int pthread_setname_np(pthread_t thread, const char *name)
{
	int retval = 0;
	size_t size = strlen(name);
	unsigned long flags = interrupt_disable();

	if (size >= sizeof(thread->name))
		retval = ERANGE;
	else
		memcpy(thread->name, name, size + 1);

	interrupt_enable(flags);

	return retval;
}

static inline int atomic_sub_return(int v, volatile int *ptr)
{
	return atomic_add_return(-v, ptr);
}

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
	mutex->lock = 1;
	TAILQ_INIT(&mutex->wq);
	mutex->type = attr->type;
	mutex->owner = NULL;
	mutex->recursive_count = 0;

	return 0;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
	(void)mutex;
	assert(TAILQ_EMPTY(&mutex->wq));

	return 0;
}

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
	unsigned long flags;
	struct wait w;

	assert(!in_irq);

	if (atomic_sub_return(1, &mutex->lock) == 0) {
		mutex->owner = pthread_current;
		mutex->recursive_count = 1;
		return 0;
	}
	if (mutex->type == PTHREAD_MUTEX_RECURSIVE_NP &&
	    mutex->owner == pthread_current) {
		mutex->recursive_count++;
		return 0;
	}
	flags = interrupt_disable();
	w.thread = pthread_self();
	TAILQ_INSERT_TAIL(&mutex->wq, &w, link);
	for (;;) {
		if (mutex->lock == 1) {
			mutex->lock = -1;
			break;
		} else {
			mutex->lock = -1;
		}
		pthread_self()->state = PTHREAD_STATE_SLEEPING;
		schedule();
	}
	TAILQ_REMOVE(&mutex->wq, &w, link);
	if (TAILQ_EMPTY(&mutex->wq))
		mutex->lock = 0;
	interrupt_enable(flags);
	mutex->owner = pthread_current;
	mutex->recursive_count = 1;

	return 0;
}

int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
	if (atomic_sub_return(1, &mutex->lock) == 0) {
		mutex->owner = pthread_current;
		mutex->recursive_count = 1;
		return 0;
	}
	if (mutex->type == PTHREAD_MUTEX_RECURSIVE_NP &&
	    mutex->owner == pthread_current) {
		mutex->recursive_count++;
		return 0;
	}

	return EBUSY;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	unsigned long flags;
	struct wait *w;

	if (--mutex->recursive_count != 0)
		return 0;
	mutex->owner = NULL;
	if (atomic_add_return(1, &mutex->lock) == 1)
		return 0;
	flags = interrupt_disable();
	mutex->lock = 1;
	w = TAILQ_FIRST(&mutex->wq);
	if (w)
		wake_up(w->thread);
	interrupt_enable(flags);

	return 0;
}

int pthread_cond_init(pthread_cond_t *cond, pthread_condattr_t *attr)
{
	TAILQ_INIT(&cond->wq);
	(void)attr;

	return 0;
}

int pthread_cond_destroy(pthread_cond_t *cond)
{
	(void)cond;

	return 0;
}

int pthread_cond_signal(pthread_cond_t *cond)
{
	unsigned long flags;
	struct wait *w;

	flags = interrupt_disable();
	w = TAILQ_FIRST(&cond->wq);
	if (w) {
		TAILQ_REMOVE(&cond->wq, w, link);
		TAILQ_ENTRY_INIT(&w->link);
		wake_up(w->thread);
	}
	interrupt_enable(flags);

	return 0;
}

int pthread_cond_broadcast(pthread_cond_t *cond)
{
	unsigned long flags;
	struct wait *w;

	flags = interrupt_disable();
	if (!TAILQ_EMPTY(&cond->wq)) {
		while ((w = TAILQ_FIRST(&cond->wq))) {
			TAILQ_REMOVE(&cond->wq, w, link);
			TAILQ_ENTRY_INIT(&w->link);
			__pthread_set_running(w->thread);
		}
		schedule();
	}
	interrupt_enable(flags);

	return 0;
}

int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex,
			   const struct timespec *abstime)
{
	unsigned long flags;
	struct wait w;
	int retval = 0;

	w.thread = pthread_self();
	flags = interrupt_disable();
	TAILQ_INSERT_TAIL(&cond->wq, &w, link);
	w.thread->state = PTHREAD_STATE_SLEEPING;
	pthread_mutex_unlock(mutex);
	if (abstime) {
		struct timespec now;
		unsigned long delta;

		clock_gettime(CLOCK_REALTIME, &now);
		if (abstime->tv_sec > now.tv_sec ||
		    (abstime->tv_sec == now.tv_sec &&
		     abstime->tv_nsec > now.tv_nsec)) {
			delta = (abstime->tv_sec - now.tv_sec) * TICKS_PER_SEC +
				(abstime->tv_nsec - now.tv_nsec) / NSECS_PER_TICK;
			if (delta == 0)
				delta = 1;
			if (schedule_timeout(delta) <= 0)
				retval = ETIMEDOUT;
		} else {
			retval = ETIMEDOUT;
		}
	} else {
		schedule();
	}
	pthread_mutex_lock(mutex);
	if (!TAILQ_ENTRY_EMPTY(&w.link))
		TAILQ_REMOVE(&cond->wq, &w, link);
	interrupt_enable(flags);

	return retval;
}

void pthread_foreach(void (*callback)(pthread_t ))
{
	size_t i;
	unsigned long flags = interrupt_disable();

	for (i = 0; i < ARRAY_SIZE(pthreads); ++i) {
		switch (pthreads[i].state) {
		case PTHREAD_STATE_NONE:
		case PTHREAD_STATE_INIT:
			continue;
		default:
			break;
		}
		callback(&pthreads[i]);
	}
	callback(&pthread_idle);

	interrupt_enable(flags);
}
