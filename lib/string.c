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

#include <string.h>
#include <stdint.h>
#include <assert.h>

int memcmp(const void *s1, const void *s2, size_t n)
{
	const unsigned char *u1 = s1, *u2 = s2;

	while (n-- > 0) {
		if (*u1 > *u2)
			return 1;
		else if (*u1 < *u2)
			return -1;
		++u1;
		++u2;
	}

	return 0;
}

void *memcpy(void *dest, const void *src, size_t n)
{
	char *cdest = dest;
	const char *csrc = src;

	while (n-- > 0)
		*cdest++ = *csrc++;

	return dest;
}

void *memmove(void *dest, const void *src, size_t n)
{
	char *cdest = dest;
	const char *csrc = src;

	if (cdest < csrc) {
		while (n-- > 0)
			*cdest++ = *csrc++;
	} else if (cdest > csrc) {
		cdest += n;
		csrc += n;
		while (n-- > 0)
			*cdest-- = *csrc--;
	}

	return dest;
}

void *memset(void *s, int c, size_t n)
{
	unsigned char *u = s;

	while (n-- > 0)
		*u++ = c;

	return s;
}

size_t strlen(const char *s)
{
	size_t n = 0;

	while (*s++ != '\0')
		++n;

	return n;
}

char *strcpy(char *dest, const char *src)
{
	char *old_dest = dest;

	while ((*dest++ = *src++)) {
	}

	return old_dest;
}

int strcmp(const char *s1, const char *s2)
{
	do {
		if (*s1++ != *s2++)
			return (uint8_t)s1[-1] > (uint8_t)s2[-1] ? 1 : -1;
	} while (s1[-1] != '\0');

	return 0;
}

char *strchr(const char *s, int c)
{
	do {
		if ((unsigned char)*s == c)
			return (char *)s;
	} while (*s++);

	return NULL;
}

char *strtok_r(char *str, const char *delim, char **saveptr)
{
	char *ptr;

	assert(delim);
	assert(*delim);

	if (!str) {
		if (!saveptr)
			return NULL;
		str = *saveptr;
	}

	for (ptr = str; *ptr && strchr(delim, (unsigned char)*ptr); ++ptr) {
	}
	if (*ptr) {
		str = ptr++;
		for (; !strchr(delim, (unsigned char)*ptr); ++ptr) {
		}
		if (*ptr)
			*ptr++ = '\0';
	} else {
		str = NULL;
	}
	if (saveptr)
		*saveptr = ptr;

	return str;
}
