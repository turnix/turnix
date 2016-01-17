/*
 * Copyright (c) 2016 Changli Gao <xiaosuo@gmail.com>
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

#include <errno.h>
#include <string.h>
#include <version.h>
#include <stringify.h>
#include <ctype.h>

#include <sys/utsname.h>

#ifndef KERNEL_RELEASE_VCS
#define KERNEL_RELEASE_VCS ""
#endif  /* KERNEL_RELEASE_VCS */

#define KERNEL_RELEASE __stringify(KERNEL_VERSION_MAJOR) "." \
		       __stringify(KERNEL_VERSION_MINOR) "." \
		       __stringify(KERNEL_VERSION_PATCH) \
		       KERNEL_RELEASE_VCS

int uname(struct utsname *buf)
{
	if (!buf) {
		errno = EFAULT;
		return -1;
	}

	strcpy(buf->sysname, KERNEL);
	if (buf->sysname[0])
		buf->sysname[0] = toupper((unsigned char)buf->sysname[0]);
	strcpy(buf->nodename, "local");
	strcpy(buf->release, KERNEL_RELEASE);
	strcpy(buf->version, KERNEL_VERSION);
	strcpy(buf->machine, KERNEL_ARCH);

	return 0;
}
