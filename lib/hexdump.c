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

#include <hexdump.h>
#include <stdio.h>
#include <ctype.h>

void hexdump(const void *buf, size_t size)
{
	const char *ptr = buf;
	size_t i;

	while (size > 0) {
		printf("%p:", ptr);
		for (i = 0; i < 16; ++i) {
			if (i == 8)
				putchar(' ');
			if (i < size)
				printf(" %02x", (unsigned char)ptr[i]);
			else
				printf("   ");
		}
		printf(" |");
		for (i = 0; i < 16; ++i) {
			if (i >= size)
				break;
			if (isprint((unsigned char)ptr[i]))
				putchar((unsigned char)ptr[i]);
			else
				putchar('.');
		}
		printf("|\n");

		if (size <= 16)
			break;
		ptr += 16;
		size -= 16;
	}
}
