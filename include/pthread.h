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

#ifndef PTHREAD_H
#define PTHREAD_H

#include <assert.h>
#include <errno.h>
#include <sched.h>
#include <time.h>

enum {
	PTHREAD_FLAG_DETACH = 0x01,
};

enum {
	PTHREAD_CREATE_JOINABLE,
	PTHREAD_CREATE_DETACHED
};

typedef struct pthread	*pthread_t;

TAILQ_HEAD(pthread_queue, pthread);

struct wait {
	pthread_t		thread;
	TAILQ_ENTRY(, wait)	link;
};

TAILQ_HEAD(wait_queue, wait);

extern pthread_t	pthread_next;

static inline int pthread_equal(pthread_t t1, pthread_t t2)
{
	return t1 == t2;
}

void schedule(void);

void wake_up(pthread_t th);

void pthread_init(void);

struct pthread_attr {
	void	*stack_addr;
	size_t	stack_size;
	uint8_t	priority;
	uint8_t	flags;
};
typedef struct pthread_attr pthread_attr_t;

static inline int pthread_attr_init(pthread_attr_t *attr)
{
	attr->stack_addr = NULL;
	attr->stack_size = 0;
	attr->priority = SCHED_RR_PRIORITY_DEFAULT;
	attr->flags = 0;

	return 0;
}

static inline int pthread_attr_destroy(pthread_attr_t *attr)
{
	(void)attr;

	return 0;
}

static inline int pthread_attr_setstack(pthread_attr_t *attr, void *stackaddr,
					size_t stacksize)
{
	attr->stack_addr = stackaddr;
	attr->stack_size = stacksize;

	return 0;
}

static inline int pthread_attr_setschedparam(pthread_attr_t *attr,
					     const struct sched_param *param)
{
	assert(param->sched_priority >= SCHED_RR_PRIORITY_MIN);
	assert(param->sched_priority <= SCHED_RR_PRIORITY_MAX);

	attr->priority = param->sched_priority;

	return 0;
}

static inline int pthread_attr_setdetachstate(pthread_attr_t *attr, int state)
{
	switch (state) {
	case PTHREAD_CREATE_JOINABLE:
		attr->flags &= ~PTHREAD_FLAG_DETACH;
		break;
	case PTHREAD_CREATE_DETACHED:
		attr->flags |= PTHREAD_FLAG_DETACH;
		break;
	default:
		return EINVAL;
	}

	return 0;
}

int pthread_setschedprio(pthread_t thread, int priority);

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
		   void *(*start_routine)(void *), void *arg);

void pthread_exit(void *retval);

int pthread_detach(pthread_t thread);

int pthread_join(pthread_t thread, void **retval);

int pthread_yield(void);

#define pthread_self() (pthread_current)

int pthread_getname_np(pthread_t thread, char *buf, size_t size);

int pthread_setname_np(pthread_t thread, const char *name);

struct pthread_mutexattr {
};

typedef struct pthread_mutexattr pthread_mutexattr_t;

static inline int pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
	(void)attr;

	return 0;
}

static inline int pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
	(void)attr;

	return 0;
}

struct pthread_mutex {
	volatile int		lock;
	struct wait_queue	wq;
};

typedef struct pthread_mutex pthread_mutex_t;

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_trylock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);

struct pthread_condattr {
};

typedef struct pthread_condattr pthread_condattr_t;

static inline int pthread_condattr_init(pthread_condattr_t *attr)
{
	(void)attr;

	return 0;
}

static inline int pthread_condattr_destroy(pthread_condattr_t *attr)
{
	(void)attr;

	return 0;
}

struct pthread_cond {
	struct wait_queue wq;
};

typedef struct pthread_cond pthread_cond_t;

int pthread_cond_init(pthread_cond_t *cond, pthread_condattr_t *attr);
int pthread_cond_destroy(pthread_cond_t *cond);
int pthread_cond_signal(pthread_cond_t *cond);
int pthread_cond_broadcast(pthread_cond_t *cond);
int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex,
			   const struct timespec *abstime);
static inline int pthread_cond_wait(pthread_cond_t *cond,
				    pthread_mutex_t *mutex)
{
	return pthread_cond_timedwait(cond, mutex, NULL);
}

void pthread_foreach(void (*callback)(pthread_t));

#endif  /* PTHREAD_H */
