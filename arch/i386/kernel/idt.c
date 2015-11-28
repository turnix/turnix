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

#include <idt.h>
#include <stdint.h>
#include <string.h>
#include <arch.h>
#include <kernel.h>

enum id_type {
	ID_TYPE_INTERRUPT_GATE = 6,
	ID_TYPE_TRAP_GATE = 7
};

struct id {
	uint16_t	base_addr_low;
	uint16_t	seg_sel;
	uint8_t		zero;
	uint8_t		type:3;
	uint8_t		gate_32bit:1;
	uint8_t		zero_1:1;
	uint8_t		privilege:2;
	uint8_t		present:1;
	uint16_t	base_addr_high;
} __attribute__((aligned(8)));

#define IRQ(irq) extern void isr##irq(void);
#include <irq.h>
#undef IRQ

static struct id idt[IRQ_MAX + 1];

static const struct {
	uint16_t	limit;
	struct id	*base;
} __attribute__((packed)) idt_ptr = {
	.limit	= sizeof(idt) - 1,
	.base	= idt
};

typedef void isr_t(void);

static isr_t *isr[] = {
#define IRQ(irq) [irq] = isr##irq,
#include <irq.h>
#undef IRQ
};

static inline void id_set(struct id *id, void (*addr)(void), uint8_t privilege,
			  enum id_type type)
{
	id->base_addr_low = (uint32_t)addr;
	id->seg_sel = KERNEL_CS;
	id->zero = 0;
	id->type = type;
	id->gate_32bit = 1;
	id->zero_1 = 0;
	id->privilege = privilege;
	id->present = 1;
	id->base_addr_high = ((uint32_t)addr) >> 16;
}

static inline void setup_isr(int irq, void (*addr)(void), uint8_t privilege,
			     enum id_type type)
{
	id_set(idt + irq, addr, privilege, type);
}

void idt_init(void)
{
	size_t i;

	memset(idt, 0, sizeof(idt));

	for (i = 0; i < ARRAY_SIZE(isr); ++i) {
		if (isr[i])
			setup_isr(i, isr[i], 0, ID_TYPE_INTERRUPT_GATE);
	}

	asm volatile("lidt %0"
		     :
		     : "m"(idt_ptr)
		     : "memory");
}
