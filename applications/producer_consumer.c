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

static uint32_t stack_producer[256];
static uint32_t stack_consumer[256];
static pthread_mutex_t mutex;
static pthread_cond_t cond_empty;
static pthread_cond_t cond_full;
static uint32_t *pcount;

static void *producer(void *args)
{
	uint32_t count = 0;

	(void)args;
	for (;;) {
		pthread_mutex_lock(&mutex);
		while (pcount)
			pthread_cond_wait(&cond_full, &mutex);
		pcount = &count;
		pthread_cond_signal(&cond_empty);
		pthread_mutex_unlock(&mutex);
		++count;
	}

	return NULL;
}

static void *consumer(void *args)
{
	uint32_t count;

	(void)args;
	for (;;) {
		pthread_mutex_lock(&mutex);
		while (!pcount)
			pthread_cond_wait(&cond_empty, &mutex);
		count = *pcount;
		pcount = NULL;
		pthread_cond_signal(&cond_full);
		pthread_mutex_unlock(&mutex);
		printf("%u\n", count);
	}

	return NULL;
}

static void producer_consumer_init(void)
{
	pthread_attr_t attr;
	pthread_t tid;

	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond_empty, NULL);
	pthread_cond_init(&cond_full, NULL);

	pthread_attr_init(&attr);
	pthread_attr_setstack(&attr, stack_producer, sizeof(stack_producer));
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&tid, &attr, producer, NULL);
	pthread_setname_np(tid, "producer");
	pthread_attr_destroy(&attr);

	pthread_attr_init(&attr);
	pthread_attr_setstack(&attr, stack_consumer, sizeof(stack_consumer));
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&tid, &attr, consumer, NULL);
	pthread_setname_np(tid, "consumer");
	pthread_attr_destroy(&attr);
}
application_init(producer_consumer_init);
