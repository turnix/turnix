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
 * Intel 8259A
 */

#include <pic.h>
#include <sys/io.h>

enum {
	PIC_PORT_CMD_MASTER	= 0x20,
	PIC_PORT_CMD_SLAVE	= 0xa0,
	PIC_PORT_DATA_MASTER	= 0x21,
	PIC_PORT_DATA_SLAVE	= 0xa1
};

enum {
	PIC_IC4			= 0x01,
	PIC_INIT		= 0x10,
	PIC_IV_OFFSET_MASTER	= 32,
	PIC_IV_OFFSET_SLAVE	= 40,
	PIC_UPM			= 0x01,
	PIC_CASCADE_IRQ		= 0x02,
	PIC_EOI			= 0x20,
	PIC_IRQ_NUM		= 8
};

void pic_ack(unsigned int irq)
{
	if (irq < PIC_IV_OFFSET_SLAVE + 8) {
		if (irq >= PIC_IV_OFFSET_SLAVE)
			outb(PIC_EOI, PIC_PORT_CMD_SLAVE);
		outb(PIC_EOI, PIC_PORT_CMD_MASTER);
	}
}

void pic_init()
{
	outb(PIC_INIT | PIC_IC4, PIC_PORT_CMD_MASTER);
	outb(PIC_INIT | PIC_IC4, PIC_PORT_CMD_SLAVE);
	outb(PIC_IV_OFFSET_MASTER, PIC_PORT_DATA_MASTER);
	outb(PIC_IV_OFFSET_SLAVE, PIC_PORT_DATA_SLAVE);
	outb(1 << PIC_CASCADE_IRQ, PIC_PORT_DATA_MASTER);
	outb(PIC_CASCADE_IRQ, PIC_PORT_DATA_SLAVE);
	outb(PIC_UPM, PIC_PORT_DATA_MASTER);
	outb(PIC_UPM, PIC_PORT_DATA_SLAVE);
	outb(~(1 << PIC_CASCADE_IRQ), PIC_PORT_DATA_MASTER);
	outb(0xff, PIC_PORT_DATA_SLAVE);
}

void pic_disable(unsigned int irq)
{
	irq -= PIC_IV_OFFSET_MASTER;
	if (irq != PIC_CASCADE_IRQ) {
		if (irq < PIC_IRQ_NUM) {
			outb(inb(PIC_PORT_DATA_MASTER) | (1 << irq),
			     PIC_PORT_DATA_MASTER);
		} else {
			outb(inb(PIC_PORT_DATA_SLAVE) | (1 << (irq - PIC_IRQ_NUM)),
			     PIC_PORT_DATA_SLAVE);
		}
	}
}

void pic_enable(unsigned int irq)
{
	irq -= PIC_IV_OFFSET_MASTER;
	if (irq != PIC_CASCADE_IRQ) {
		if (irq < PIC_IRQ_NUM) {
			outb(inb(PIC_PORT_DATA_MASTER) & ~(1 << irq),
			     PIC_PORT_DATA_MASTER);
		} else {
			outb(inb(PIC_PORT_DATA_SLAVE) & ~(1 << (irq - PIC_IRQ_NUM)),
			     PIC_PORT_DATA_SLAVE);
		}
	}
}
