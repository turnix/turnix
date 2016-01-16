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

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <stdbool.h>
#include <limits.h>

void abort(void)
{
	printf("kernel panic!\n");
	for (;;) {
	}
}

enum {
	__STRTOUL_F_NEG = 0x1,
	__STRTOUL_F_INV = 0x2,
	__STRTOUL_F_OVF = 0x4,
};

static unsigned long __strtoul(const char *nptr, char **endptr, int base,
			      int *flags)
{
	/* sanity check */
	if (nptr == NULL || (base < 2 && base != 0) || base > 36) {
		*flags |= __STRTOUL_F_INV;
		return 0;
	}

	/* skip white space */
	while (isspace(*(unsigned char *)nptr))
		++nptr;

	/* parse the optional sign */
	switch (*nptr) {
	case '-':
		*flags |= __STRTOUL_F_NEG;
		/* fallthrought */
	case '+':
		++nptr;
		break;
	case '\0':
		*flags |= __STRTOUL_F_INV;
		return 0;
	}

	/* detect the base */
	if (base == 0) {
		if (*nptr == '0') {
			if (tolower(*(unsigned char *)(nptr + 1)) == 'x')
				base = 16;
			else
				base = 8;
		} else {
			base = 10;
		}
	}

	/* remove the optional prefix for base16 */
	if (base == 16 && *nptr == '0' &&
	    tolower(*(unsigned char *)(nptr + 1)) == 'x') {
		nptr += 2;
	}

	bool parsed = false;
	unsigned long val = 0;
	while (*nptr) {
		int c = *(unsigned char *)nptr;

		if (isdigit(c))
			c = c - '0';
		else if (isalpha(c))
			c = tolower(c) - ('a' - 10);
		else
			break;
		if (c >= base)
			break;
		parsed = true;
		if (val > (ULONG_MAX - c) / base)
			*flags |= __STRTOUL_F_OVF;
		val = val * base + c;
		++nptr;
	}

	if (!parsed) {
		*flags |= __STRTOUL_F_INV;
		return 0;
	}
	if (endptr)
		*endptr = (char *)nptr;

	return val;
}

long int strtol(const char *nptr, char **endptr, int base)
{
	int flags = 0;
	unsigned long v;

	v = __strtoul(nptr, endptr, base, &flags);
	if (flags & __STRTOUL_F_INV) {
		if (endptr)
			*endptr = (char *)nptr;
		errno = EINVAL;
		return 0;
	}

	if ((flags & __STRTOUL_F_OVF) == 0) {
		if (flags & __STRTOUL_F_NEG) {
			if (v > (unsigned long)LONG_MAX + 1)
				flags |= __STRTOUL_F_OVF;
		} else {
			if (v > (unsigned long)LONG_MAX)
				flags |= __STRTOUL_F_OVF;
		}
	}
	if (flags & __STRTOUL_F_OVF) {
		errno = ERANGE;
		return (flags & __STRTOUL_F_NEG) ? LONG_MIN : LONG_MAX;
	}

	return (flags & __STRTOUL_F_NEG) ? (-v) : v;
}

unsigned long int strtoul(const char *nptr, char **endptr, int base)
{
	int flags = 0;
	unsigned long v;

	v = __strtoul(nptr, endptr, base, &flags);
	if (flags & __STRTOUL_F_INV) {
		if (endptr)
			*endptr = (char *)nptr;
		errno = EINVAL;
		return 0;
	}

	if (flags & __STRTOUL_F_OVF) {
		errno = ERANGE;
		return ULONG_MAX;
	}

	return (flags & __STRTOUL_F_NEG) ? (-v) : v;
}
