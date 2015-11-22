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

#ifndef SYS_QUEUE_H
#define SYS_QUEUE_H

#define TAILQ_HEAD(name, type) \
struct name { \
	struct type	*first; \
	struct type	**ptail; \
}

#define TAILQ_HEAD_INITIALIZER(head) \
{ \
	.first = NULL, \
	.ptail = &(head).first \
}

#define TAILQ_INIT(head) \
do { \
	(head)->first = NULL; \
	(head)->ptail = &(head)->first; \
} while (0)

#define TAILQ_ENTRY(name, type) \
struct name { \
	struct type	*next; \
	struct type	**pprev; \
}

#define TAILQ_ENTRY_INIT(entry) \
do { \
	(entry)->pprev = NULL; \
} while (0)

#define TAILQ_ENTRY_EMPTY(entry) ((entry)->pprev == NULL)

#define TAILQ_EMPTY(head) ((head)->first == NULL)

#define TAILQ_FIRST(head) ((head)->first)

#define TAILQ_INSERT_HEAD(head, entry, member) \
do { \
	(entry)->member.next = (head)->first; \
	(entry)->member.pprev = &(head)->first; \
	if ((head)->first) \
		(head)->first->member.pprev = &(entry)->member.next; \
	else \
		(head)->ptail = &(entry)->member.next; \
	(head)->first = (entry); \
} while (0)

#define TAILQ_INSERT_TAIL(head, entry, member) \
do { \
	(entry)->member.next = NULL; \
	(entry)->member.pprev = (head)->ptail; \
	*((head)->ptail) = (entry); \
	(head)->ptail = &(entry)->member.next; \
} while (0)

#define TAILQ_REMOVE(head, entry, member) \
do { \
	if ((entry)->member.next) \
		(entry)->member.next->member.pprev = (entry)->member.pprev; \
	else \
		(head)->ptail = (entry)->member.pprev; \
	*((entry)->member.pprev) = (entry)->member.next; \
} while (0)

#define TAILQ_FOREACH(it, head, member) \
for ((it) = (head)->first; (it); (it) = (it)->member.next)

#endif  /* SYS_QUEUE_H */
