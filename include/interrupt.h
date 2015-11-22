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

#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <config.h>
#include <stdint.h>
#include <stdbool.h>

enum {
	CPU_FLAG_IF = 0x200
};

struct interrupt_context {
#if CONFIG_USERSPACE
	uint16_t gs, __pad0, fs, __pad1, es, __pad2, ds, __pad3;
#endif
	uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
	uint32_t irq, error;
	uint32_t eip;
	uint16_t cs, __pad4;
	uint32_t eflags;
#if CONFIG_USERSPACE
	uint32_t user_esp;
	uint16_t user_ss;
	uint16_t __pad5;
#endif
};

extern bool in_irq;

typedef void interrupt_handler_t(struct interrupt_context *ctx);

interrupt_handler_t *interrupt_register(unsigned int irq,
					interrupt_handler_t *handler);
void interrupt_init(void);

static inline unsigned long interrupt_disable(void)
{
	unsigned long flags;

	asm volatile("pushf\n\t"
		     "pop %0\n\t"
		     "cli"
		     : "=rm"(flags)
		     :
		     : "memory");

	return flags;
}

static inline void interrupt_enable(unsigned long flags)
{
	asm volatile("push %0\n\t"
		     "popf"
		     :
		     : "g"(flags)
		     : "memory", "cc");
}

#endif  /* INTERRUPT_H */
