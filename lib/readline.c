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

#include <stddef.h>
#include <limits.h>
#include <stdio.h>

#include <readline/readline.h>

static unsigned char readline_buffer[LINE_MAX];

/* It isn't thread-safe. */
char *readline(const char *prompt)
{
	int c;
	size_t n = 0;

	printf("%s", prompt ? prompt : "");

	while (n < sizeof(readline_buffer) - 1) {
		c = getchar();
		switch (c) {
		case 0x08: /* backspace */
			if (n > 0) {
				putchar(c);
				--n;
			}
			break;
		case '\t':
			break;
		case '\n':
			putchar(c);
			goto out;
		default:
			putchar(c);
			readline_buffer[n++] = c;
		}
	}
out:
	readline_buffer[n] = '\0';

	return (char *)readline_buffer;
}
