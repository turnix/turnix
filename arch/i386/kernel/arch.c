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

#include <arch.h>
#include <text_buffer.h>
#include <gdt.h>
#include <pit.h>
#include <cmos.h>
#include <keyboard.h>
#include <pthread.h>
#include <kernel.h>

#if CONFIG_SWI
enum {
	IRQ_SYSTEM_CALL = 0x80
};

static void system_call(struct interrupt_context *ctx)
{
	(void)ctx;
}
#endif

void arch_early_init(void)
{
	text_buffer_init();
}

void arch_init(void)
{
	gdt_init();
	interrupt_init();
	pit_init();
	cmos_init();
	keyboard_init();
#if CONFIG_SWI
	interrupt_register(IRQ_SYSTEM_CALL, system_call);
#endif
}

void arch_pthread_init(pthread_t th, void (*wrapper)(void *(*)(void *), void *),
		       void *(*start_routine)(void *), void *arg)
{
	unsigned long *stack;
	struct interrupt_context *ctx;

	stack = (unsigned long *)((char *)th->stack_addr + th->stack_size);
	stack[-1] = (unsigned long)arg;
	stack[-2] = (unsigned long)start_routine;
	stack[-3] = (unsigned long)abort;
#if CONFIG_USERSPACE
	ctx = (struct interrupt_context *)(stack - 1) - 1;
	ctx->gs = ctx->fs = ctx->es = ctx->ds = KERNEL_DS;
#else
	ctx = (struct interrupt_context *)(stack - 3) - 1;
#endif
	ctx->eip = (unsigned long)wrapper;
	ctx->ebp = (unsigned long)(stack - 1);
	ctx->esp = (unsigned long)(stack - 3);
	ctx->cs = KERNEL_CS;
	ctx->eflags = CPU_FLAG_IF;
	th->context.esp = (unsigned long)ctx;
}
