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

/*
 * Intel 8253/8254
 */

#include <config.h>
#include <pit.h>
#include <interrupt.h>
#include <stdio.h>
#include <pic.h>
#include <timer.h>

#include <sys/io.h>

enum {
	PIT_HZ		= 1193180,
	PIT_IRQ		= 32,
	PIT_PORT_CTRL	= 0x43,
	PIT_CHAN	= 0,
	PIT_LSB_MSB	= 3,
	PIT_SQUARE	= 3,
	PIT_PORT_CHAN	= 0x40
};

static void pic_handler(struct interrupt_context *ctx)
{
	(void)ctx;
	timer_update();
}

void pit_init(void)
{
	int divisor = PIT_HZ / CONFIG_HZ;

	outb((PIT_CHAN << 6) | (PIT_LSB_MSB << 4) | (PIT_SQUARE << 1),
	     PIT_PORT_CTRL);
	outb(divisor, PIT_PORT_CHAN);
	outb(divisor >> 8, PIT_PORT_CHAN);

	interrupt_register(PIT_IRQ, pic_handler);
	pic_enable(PIT_IRQ);
}
