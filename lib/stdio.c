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

#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>

struct print_context {
	int	pad;
	size_t	min_width;
};

static const char * const number_table = "0123456789abcdef";
static const char * const number_table_upper = "0123456789ABCDEF";

static int print_str(struct print_context *ctx, const char *str)
{
	char c;
	size_t n = strlen(str);

	while (n < ctx->min_width) {
		putchar(ctx->pad);
		++n;
	}

	while ((c = *str++) != '\0')
		putchar((unsigned char)c);

	return n;
}

static int __print_number(struct print_context *ctx, unsigned int neg,
			  unsigned int n, unsigned int base, const char *table)
{
	char buf[12];  /* it is enough for int */
	char *ptr;

	buf[sizeof(buf) - 1] = '\0';
	ptr = &buf[sizeof(buf) - 1];
	if (n == 0) {
		*(--ptr) = '0';
	} else {
		do {
			*(--ptr) = table[n % base];
			n /= base;
		} while (n != 0);
	}
	if (neg)
		*(--ptr) = '-';

	return print_str(ctx, ptr);
}

static inline int print_unsigned(struct print_context *ctx,
				 unsigned int n, unsigned int base)
{
	return __print_number(ctx, 0, n, base, number_table);
}

static int print_number(struct print_context *ctx, int n, unsigned int base)
{
	int neg;

	if (n < 0) {
		neg = 1;
		n = -n;
	} else {
		neg = 0;
	}

	return __print_number(ctx, neg, n, base, number_table);
}

static int print_char(struct print_context *ctx, int c)
{
	size_t n = 1;

	while (n < ctx->min_width)
		putchar(ctx->pad);
	putchar(c);

	return n;
}

int printf(const char *format, ...)
{
	char c;
	int n = 0;
	va_list ap;
	struct print_context ctx;

	va_start(ap, format);
	while ((c = *format++) != '\0') {
		if (c != '%') {
			putchar((unsigned char)c);
			++n;
			continue;
		}
		ctx.pad = ' ';
		ctx.min_width = 0;
		c = *format++;
		for (;;) {
			switch (c) {
			case '\0':
				return n;
			case '%':
				n += print_char(&ctx, '%');
				break;
			case 'i':
			case 'd':
				n += print_number(&ctx, va_arg(ap, int), 10);
				break;
			case 'X':
				n += __print_number(&ctx, 0,
						    va_arg(ap, unsigned int),
						    16, number_table_upper);
				break;
			case 'p':
				ctx.min_width = 0;
				n += print_str(&ctx, "0x");
				ctx.pad = '0';
				ctx.min_width = 8;
				/* fall through */
			case 'x':
				n += print_unsigned(&ctx,
						    va_arg(ap, unsigned int),
						    16);
				break;
			case 'u':
				n += print_unsigned(&ctx,
						    va_arg(ap, unsigned int),
						    10);
				break;
			case 'o':
				n += print_unsigned(&ctx,
						    va_arg(ap, unsigned int),
						    8);
				break;
			case 's':
				n += print_str(&ctx, va_arg(ap, char *));
				break;
			case 'c':
				n += print_char(&ctx, va_arg(ap, int));
				break;
			case 'l':
				/* Ignore l, and don't support ll. */
				c = *format++;
				continue;
			case '0':
				ctx.pad = '0';
				c = *format++;
				if (c == '\0')
					return n;
				/* fall through */
			default:
				if (isdigit(c)) {
					do {
						ctx.min_width = ctx.min_width * 10 + (c - '0');
					} while (isdigit(c = *format++));
					continue;
				} else {
					n += print_char(&ctx, (unsigned char)c);
				}
				break;
			}
			break;
		}
	}
	va_end(ap);

	return n;
}
