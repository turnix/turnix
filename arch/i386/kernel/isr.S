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

#define __ASSEMBLY__
#include "kernel.h"

.section .text

.macro isr irq
.global isr\irq
isr\irq:
	push $0x0
	push $\irq
	jmp isr_comm
.endm

.macro isr_with_error irq
.global isr\irq
isr\irq:
	push $\irq
	jmp isr_comm
.endm

.macro save_context
	pusha
#if CONFIG_USERSPACE
	push %ds
	push %es
	push %fs
	push %gs

	mov $KERNEL_DS, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
#endif
.endm

#if !CONFIG_SWI
.global arch_context_switch
arch_context_switch:
	pop %eax
	pushf
	push %cs
	push %eax
	push $0
	push $0
	save_context
	mov pthread_current, %eax
	mov pthread_next, %ebx
	jmp __arch_context_switch
#endif

isr_comm:
	save_context

	mov %esp, %eax
	push %eax
	call interrupt_dispatch
	pop %eax

	mov pthread_next, %ebx
	test %ebx, %ebx
	jnz no_schedule
	call __schedule
no_schedule:
	mov pthread_current, %eax
	mov pthread_next, %ebx
	cmp %eax, %ebx
	je restore
__arch_context_switch:
	test %eax, %eax
	jz no_save
	mov %esp, (%eax)
no_save:
	mov (%ebx), %esp
	mov %ebx, pthread_current

restore:
#if CONFIG_USERSPACE
	pop %gs
	pop %fs
	pop %es
	pop %ds
#endif
	popa
	add $0x8, %esp
	iret

#define IRQ(irq) isr irq
#define IRQ_WITH_ERROR(irq) isr_with_error irq
#include <irq.h>
