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

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <application.h>

enum {
	PHILOSOPHERS_NUM = 5
};

static uint32_t stack[PHILOSOPHERS_NUM][256];
static pthread_mutex_t forks[PHILOSOPHERS_NUM];

static void *philosopher(void *args)
{
	size_t id = (size_t)args;
	size_t first_fork = id;
	size_t second_fork = id + 1;

	if (second_fork >= PHILOSOPHERS_NUM) {
		first_fork = 0;
		second_fork = id;
	}
	pthread_mutex_lock(&forks[first_fork]);
	pthread_mutex_lock(&forks[second_fork]);
	printf("philosopher %lu starts to eat\n", id);
	sleep(1);
	printf("philosopher %lu finished eating\n", id);
	pthread_mutex_unlock(&forks[second_fork]);
	pthread_mutex_unlock(&forks[first_fork]);

	return NULL;
}

static void philosophers_init(void)
{
	pthread_attr_t attr;
	pthread_t tid;
	size_t i;

	for (i = 0; i < PHILOSOPHERS_NUM; ++i)
		pthread_mutex_init(&forks[i], NULL);

	for (i = 0; i < PHILOSOPHERS_NUM; ++i) {
		pthread_attr_init(&attr);
		pthread_attr_setstack(&attr, stack[i], sizeof(stack[0]));
		pthread_create(&tid, &attr, philosopher, (void*)i);
		pthread_setname_np(tid, "philo");
		pthread_detach(tid);
		pthread_attr_destroy(&attr);
	}
}
application_init(philosophers_init);
