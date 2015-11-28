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

#include <stdint.h>
#include <interrupt.h>
#include <time.h>
#include <stdio.h>
#include <stddef.h>

#include <sys/io.h>

enum {
	CMOS_PORT_REG	= 0x70,
	CMOS_PORT_DATA	= 0x71
};

enum {
	CMOS_REG_SEC	= 0x00,
	CMOS_REG_MIN	= 0x02,
	CMOS_REG_HOUR	= 0x04,
	CMOS_REG_MDAY	= 0x07,
	CMOS_REG_MON	= 0x08,
	CMOS_REG_YEAR	= 0x09,
	CMOS_REG_A	= 0x0a,
	CMOS_REG_B	= 0x0b
};

enum {
	CMOS_REG_A_UPDATE = 0x80
};

enum {
	CMOS_REG_B_24H	= 0x02,
	CMOS_REG_B_BIN	= 0x04
};

enum {
	CMOS_REG_HOUR_PM	= 0x80
};

static inline void io_delay(void)
{
	outb(0, 0x80);
}

static uint8_t cmos_read_reg(uint8_t reg)
{
	uint8_t b;
	unsigned long flags;

	flags = interrupt_disable();
	outb(reg, CMOS_PORT_REG);
	io_delay();
	b = inb(CMOS_PORT_DATA);
	interrupt_enable(flags);

	return b;
}

static void __cmos_read_tm(struct tm *tm)
{
	while (cmos_read_reg(CMOS_REG_A) & CMOS_REG_A_UPDATE) {
	}
	tm->tm_sec = cmos_read_reg(CMOS_REG_SEC);
	tm->tm_min = cmos_read_reg(CMOS_REG_MIN);
	tm->tm_hour = cmos_read_reg(CMOS_REG_HOUR);
	tm->tm_mday = cmos_read_reg(CMOS_REG_MDAY);
	tm->tm_mon = cmos_read_reg(CMOS_REG_MON);
	tm->tm_year = cmos_read_reg(CMOS_REG_YEAR);
}

static uint8_t bcd2bin(uint8_t bcd)
{
	return (bcd >> 4) * 10 + (bcd & 0x0f);
}

static void cmos_read_tm(struct tm *tm)
{
	struct tm tmp;
	uint8_t b;
	bool pm;

	do {
		__cmos_read_tm(&tmp);
		__cmos_read_tm(tm);
	} while (tmp.tm_sec != tm->tm_sec || tmp.tm_min != tm->tm_min ||
		 tmp.tm_hour != tm->tm_hour || tmp.tm_mday != tm->tm_mday ||
		 tmp.tm_mon != tm->tm_mon || tmp.tm_year != tm->tm_year);

	b = cmos_read_reg(CMOS_REG_B);
	pm = tm->tm_hour & CMOS_REG_HOUR_PM;
	tm->tm_hour &= ~CMOS_REG_HOUR_PM;
	if (!(b & CMOS_REG_B_BIN)) {
		tm->tm_sec = bcd2bin(tm->tm_sec);
		tm->tm_min = bcd2bin(tm->tm_min);
		tm->tm_hour = bcd2bin(tm->tm_hour);
		tm->tm_mday = bcd2bin(tm->tm_mday);
		tm->tm_mon = bcd2bin(tm->tm_mon);
		tm->tm_year = bcd2bin(tm->tm_year);
	}
	--tm->tm_mon;
	if (!(b & CMOS_REG_B_24H)) {
		if (pm) {
			if (tm->tm_hour != 12)
				tm->tm_hour += 12;
		} else if (tm->tm_hour == 12) {
			tm->tm_hour = 0;
		}
	}
	tm->tm_year += 100;
}

void cmos_init(void)
{
	struct tm tm;
	struct timeval tv;

	cmos_read_tm(&tm);
	printf("Date: %04d-%02d-%02dT%02d:%02d:%02dZ\n",
	       tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
	       tm.tm_hour, tm.tm_min, tm.tm_sec);
	tv.tv_sec = mktime(&tm);
	tv.tv_usec = 0;
	settimeofday(&tv, NULL);
}
