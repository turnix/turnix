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

.set ALIGN_ON_PAGE, 1<<0
.set MEMINFO,       1<<1
.set FLAGS,         ALIGN_ON_PAGE | MEMINFO
.set MAGIC,         0x1BADB002
.set CHECKSUM,      -(MAGIC + FLAGS)

.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

.section .text
.global _start
_start:
	/* fill the stack with STACK_FILL */
	cld
	movl $STACK_FILL, idle_stack_bottom
	mov $idle_stack_bottom, %esi
	mov $CONFIG_IDLE_STACK_SIZE, %ecx
	shr $2, %ecx
	dec %ecx
	lea 4(%esi), %edi
	rep; movsl
	mov $idle_stack_top, %esp
	push $0
	popf
	push 8(%ebx)
	push 4(%ebx)
	push (%ebx)
	mov %esp, %eax
	push %eax
	call main
	cli

/* change the background to blue */
	mov $2000, %eax
	mov $0xb8000, %ebx
1:
	mov (%ebx), %cx
	and $0xfff, %cx
	or $0x1000, %cx
	mov %cx, (%ebx)
	add $2, %ebx
	dec %eax
	jnz 1b

/* show a message */
	mov $0x1400, %ax
	mov $0xb8000, %ebx
	mov $.Lmsg, %ecx
1:
	mov (%ecx), %al
	test %al, %al
	jz 1f
	mov %ax, (%ebx)
	add $2, %ebx
	inc %ecx
	jmp 1b
1:
	hlt
	jmp .

.Lmsg:
.string "HANG!"

.section .boot_stack, "aw", @nobits
.global idle_stack_bottom
idle_stack_bottom:
.skip CONFIG_IDLE_STACK_SIZE
idle_stack_top:
