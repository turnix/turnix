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

#include <gdt.h>
#include <stdint.h>
#include <kernel.h>
#include <stringify.h>

struct gd {
	uint16_t	limit_low;
	uint16_t	base_addr_low;

	uint8_t		base_addr_mid;
	uint8_t		access:1;
	uint8_t		rw:1;
	uint8_t		dir_conform:1;
	uint8_t		exe:1;
	uint8_t		s:1;
	uint8_t		privilege:2;
	uint8_t		present:1;

	uint8_t		limit_high:4;
	uint8_t		user:1;
	uint8_t		unused:1;
	uint8_t		size:1;
	uint8_t		granularity:1;
	uint8_t		base_addr_high;
} __attribute__((aligned(8)));

static struct gd gdt[5];

static const struct {
	uint16_t	limit;
	struct gd	*base;
} __attribute__((packed)) gdt_ptr = {
	.limit	= sizeof(gdt) - 1,
	.base	= gdt
};

static inline void gd_set_base_addr(struct gd *gd, uint32_t base)
{
	gd->base_addr_low = base;
	gd->base_addr_mid = base >> 16;
	gd->base_addr_high = base >> 24;
}

static inline void gd_set_limit(struct gd *gd, uint32_t limit)
{
	gd->limit_low = limit;
	gd->limit_high = limit >> 16;
}

static inline void gd_zero(struct gd *gd)
{
	uint32_t *v = (uint32_t *)gd;
	v[0] = 0;
	v[1] = 0;
}

static inline void gd_set_code(struct gd *gd, uint8_t privilege,
			       uint32_t base, uint32_t limit)
{
	gd_set_base_addr(gd, base);
	gd_set_limit(gd, limit);
	gd->access = 0;
	gd->rw = 1;
	gd->dir_conform = 0;
	gd->exe = 1;
	gd->s = 1;
	gd->privilege = privilege;
	gd->present = 1;
	gd->user = 0;
	gd->unused = 0;
	gd->size = 1;
	gd->granularity = 1;
}

static inline void gd_set_data(struct gd *gd, uint8_t privilege,
			       uint32_t base, uint32_t limit)
{
	gd_set_base_addr(gd, base);
	gd_set_limit(gd, limit);
	gd->access = 0;
	gd->rw = 1;
	gd->dir_conform = 0;
	gd->exe = 0;
	gd->s = 1;
	gd->privilege = privilege;
	gd->present = 1;
	gd->user = 0;
	gd->unused = 0;
	gd->size = 1;
	gd->granularity = 1;
}

static inline void lgdt(void)
{
	asm volatile("lgdt %0\n"
		     "ljmp $" __stringify(KERNEL_CS) ", $1f\n"
		     "1:\n"
		     "mov $" __stringify(KERNEL_DS) ", %%eax\n"
		     "mov %%eax, %%ds\n"
		     "mov %%eax, %%ss\n"
		     "mov %%eax, %%es\n"
		     "mov %%eax, %%fs\n"
		     "mov %%eax, %%gs\n"
		     :
		     : "m"(gdt_ptr)
		     : "eax", "memory");
}

void gdt_init(void)
{
	gd_zero(&gdt[0]);
	gd_set_code(&gdt[1], 0, 0, 0xffffffff);
	gd_set_data(&gdt[2], 0, 0, 0xffffffff);
	gd_set_code(&gdt[3], 3, 0, 0xffffffff);
	gd_set_data(&gdt[4], 3, 0, 0xffffffff);

	lgdt();
}
