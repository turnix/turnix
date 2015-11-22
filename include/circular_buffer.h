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

#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct circular_buffer {
	size_t	read, write, len, size;
	uint8_t	*buffer;
};

static inline void circular_buffer_init(struct circular_buffer *cb,
					void *buffer, size_t size)
{
	cb->read = cb->write = cb->len = 0;
	cb->buffer = buffer;
	cb->size = size;
}

static inline bool circular_buffer_empty(const struct circular_buffer *cb)
{
	return cb->len == 0;
}

static inline bool circular_buffer_full(const struct circular_buffer *cb)
{
	return cb->len == cb->size;
}

static inline size_t circular_buffer_space(const struct circular_buffer *cb)
{
	return cb->size - cb->len;
}

size_t circular_buffer_write(struct circular_buffer *cb, const void *data,
			     size_t size);

size_t circular_buffer_read(struct circular_buffer *cb,
			    void *buffer, size_t size);

#endif  /* CIRCULAR_BUFFER_H */
