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

#ifndef CTYPE_H
#define CTYPE_H

static inline int islower(int c)
{
	return c >= 'a' && c <= 'z';
}

static inline int isupper(int c)
{
	return c >= 'A' && c <= 'Z';
}

static inline int isalpha(int c)
{
	return isupper(c) || islower(c);
}

static inline int isdigit(int c)
{
	return c >= '0' && c <= '9';
}

static inline int isalnum(int c)
{
	return isalpha(c) || isdigit(c);
}

static inline int iscntrl(int c)
{
	return (c >= 0 && c <= 0x1f) || c == 0x7f;
}

static inline int isgraph(int c)
{
	return c >= '!' && c <= '~';
}

static inline int isprint(int c)
{
	return c >= ' ' && c <= '~';
}

static inline int ispunct(int c)
{
	return isgraph(c) && !isalnum(c);
}

static inline int isspace(int c)
{
	switch (c) {
	case ' ':
	case '\f':
	case '\n':
	case '\r':
	case '\t':
	case '\v':
		return 1;
	default:
		return 0;
	}
}

static inline int isxdigit(int c)
{
	return isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

static inline int isblank(int c)
{
	return c == ' ' || c == '\t';
}

static inline int toupper(int c)
{
	if (islower(c))
		c = c - 'a' + 'A';

	return c;
}

static inline int tolower(int c)
{
	if (isupper(c))
		c = c - 'A' + 'a';

	return c;
}

#endif  /* CTYPE_H */
