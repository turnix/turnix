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

#include <multiboot.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <application.h>
#include <interrupt.h>
#include <arch.h>

extern init_func_t * const application_init_begin[];
extern init_func_t * const application_init_end[];
extern unsigned int kernel_end;

int main(struct multiboot_info *info)
{
	init_func_t * const *func;

	arch_early_init();

	if ((info->flags & 1) == 0) {
		printf("no memory info\n");
		abort();
	}
	printf("RAM:\n");
	info->mem_lower *= 1024;
	info->mem_upper *= 1024;
	info->mem_upper += 1024 * 1024;
	printf("  lower: %08x - %08x\n", 0, info->mem_lower);
	printf("  upper: %08x - %08x\n", 1024 * 1024,info->mem_upper);
	printf("  heap:  %08x - %08x\n", (uint32_t)&kernel_end,
	       info->mem_upper);

	arch_init();
	pthread_init();

	in_irq = 1;
	for (func = application_init_begin; func < application_init_end;
	     ++func) {
		(**func)();
	}
	in_irq = 0;

	arch_enable_interrupt();
	pthread_yield();

	for (;;)
		arch_halt();

	return 0;
}
