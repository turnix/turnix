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

#include <interrupt.h>
#include <irq.h>
#include <text_buffer.h>
#include <stdio.h>
#include <pic.h>
#include <idt.h>

static interrupt_handler_t *interrupt_handler[IRQ_MAX + 1];

static void interrupt_default_handler(struct interrupt_context *ctx)
{
	text_buffer_init();
	printf("Unhandled IRQ: %d, error: %08x\n", ctx->irq, ctx->error);
#if CONFIG_USERSPACE
	printf("gs: %04x, fs: %04x, es: %04x, ds: %04x\n",
	       ctx->gs, ctx->fs, ctx->es, ctx->ds);
#endif
	printf("edi: %08x, esi: %08x, ebp: %08x, esp: %08x\n"
	       "ebx: %08x, edx: %08x, ecx: %08x, eax: %08x\n",
	       ctx->edi, ctx->esi, ctx->ebp, ctx->esp,
	       ctx->ebx, ctx->edx, ctx->ecx, ctx->eax);
	printf("eip: %08x, cs: %04x, eflags: %08x\n", ctx->eip, ctx->cs,
	       ctx->eflags);
#if CONFIG_USERSPACE
	if (ctx->cs != KERNEL_CS) {
		printf("user_esp: %08x, user_ss: %04x\n",
		       ctx->user_esp, ctx->user_ss);
	}
#endif

	arch_halt();
}

interrupt_handler_t *interrupt_register(unsigned int irq,
					interrupt_handler_t *handler)
{
	interrupt_handler_t *old_handler;

	if (irq > IRQ_MAX)
		return (interrupt_handler_t *)-1;
	if (!handler)
		handler = interrupt_default_handler;
	old_handler = interrupt_handler[irq];
	interrupt_handler[irq] = handler;

	return old_handler;
}

void interrupt_dispatch(struct interrupt_context *ctx)
{
	in_irq = true;
	interrupt_handler[ctx->irq](ctx);
	in_irq = false;
	if (ctx->irq >= 32)
		pic_ack(ctx->irq);
}

void interrupt_init(void)
{
	int i;

	for (i = 0; i <= IRQ_MAX; ++i)
		interrupt_handler[i] = interrupt_default_handler;
	idt_init();
	pic_init();
}
