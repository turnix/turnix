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

#include <circular_buffer.h>

size_t circular_buffer_write(struct circular_buffer *cb, const void *_data,
			     size_t size)
{
	size_t len = 0;
	const uint8_t *data = _data;

	while (size-- > 0 && !circular_buffer_full(cb)) {
		cb->buffer[cb->write] = *data++;
		++len;
		cb->write = (cb->write + 1) % cb->size;
		cb->len++;
	}

	return len;
}

size_t circular_buffer_read(struct circular_buffer *cb, void *_buffer,
			    size_t size)
{
	size_t len = 0;
	uint8_t *buffer = _buffer;

	while (size-- > 0 && !circular_buffer_empty(cb)) {
		*buffer++ = cb->buffer[cb->read];
		++len;
		cb->read = (cb->read + 1) % cb->size;
		cb->len--;
	}

	return len;
}
