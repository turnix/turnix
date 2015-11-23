KERNEL := turnix
CFLAGS += -Wall -Werror -Wextra -O2 -m32 -nostdlib -nostdinc -fno-builtin \
	  -fno-stack-protector -nostartfiles -nodefaultlibs -std=gnu99 \
	  -Iinclude -ffreestanding
LDFLAGS += -T link.ld -melf_i386
CC := gcc
LD := ld
AS := as
NM := nm
OBJDUMP := objdump
READELF := readelf
QEMU := qemu-system-i386
GDB := gdb
SIZE := size
STRIP := strip
SHELL := /bin/bash
APPLICATION ?= applications/shell.o
export SHELLOPTS := errexit:pipefail

include Config.mk

ifeq (${CONFIG_DEBUG},1)
	CFLAGS += -g
else
	CFLAGS += -fdata-sections -ffunction-sections -DNDEBUG
	LDFLAGS += --gc-sections --print-gc-sections
endif

ASMOBJS = boot/multiboot.o kernel/isr.o
COBJS = kernel/main.o lib/string.o drivers/text_buffer.o lib/stdio.o \
	lib/stdlib.o kernel/gdt.o kernel/interrupt.o kernel/idt.o \
	lib/hexdump.o drivers/pic.o drivers/pit.o kernel/timer.o \
	lib/circular_buffer.o drivers/keyboard.o kernel/pthread.o \
	lib/readline.o ${APPLICATION} drivers/cmos.o lib/time.o
DEPS = $(COBJS:.o=.d)
OBJS = ${ASMOBJS} ${COBJS}

.DELETE_ON_ERROR:

.PHONY: all qemu dump gdb loc clean cscope tags size strip

all: ${KERNEL}.iso

iso/boot/${KERNEL}.sym: iso/boot/${KERNEL}.elf
	$(NM) -nC $< > $@

iso/boot/${KERNEL}.elf: ${OBJS} link.ld
	$(LD) -o $@ $(filter %.o,$^) ${LDFLAGS}

.c.o:
	$(CC) -c -o $@ $< ${CFLAGS}

.S.o:
	$(CC) -c -o $@ $< ${CFLAGS}

kernel/isr.o: include/irq.h include/config.h

kernel/idt.o: include/irq.h

include/irq.h: gen_irq.sh
	./gen_irq.sh > $@

kernel/idt.d: include/irq.h

%.o: %.c %.d

%.d: %.c include/config.h
	$(CC) -M $< ${CFLAGS} | \
		sed -e 's;^\(.*\)\.o\(.*\);$(@D)/\1.o $(@D)/\1.d\2;' > $@

include/config.h: Config.mk
	$(shell echo "/* DO NOT EDIT IT */" >$@)
	$(shell echo "#ifndef CONFIG_H" >>$@)
	$(shell echo "#define CONFIG_H" >>$@)
	$(foreach cfg,$(filter CONFIG_%,${.VARIABLES}),$(shell echo '#define $(cfg) $($(cfg))' >>$@))
	$(shell echo "#endif  /* CONFIG_H */" >>$@)

-include $(DEPS)

qemu:
	$(QEMU) -m 16M -kernel iso/boot/${KERNEL}.elf

dump:
	$(OBJDUMP) -d iso/boot/${KERNEL}.elf | less

gdb:
	$(QEMU) -s -S -kernel iso/boot/${KERNEL}.elf &
	$(GDB) iso/boot/${KERNEL}.elf -ex 'target remote 127.0.0.1:1234'

${KERNEL}.iso: iso/boot/${KERNEL}.elf iso/boot/${KERNEL}.sym
	grub-mkrescue -o $@ --product-name ${KERNEL} iso

loc:
	find . -name '*.[hcS]' | xargs wc -l | sort -n

tags:
	ctags -R

cscope:
	find . -name '*.[hcS]' > cscope.files
	cscope -bk

size:
	@$(SIZE) -A iso/boot/${KERNEL}.elf

strip:
	$(STRIP) -R .comment -g iso/boot/${KERNEL}.elf

clean:
	$(RM) ${OBJS} ${DEPS}
	$(RM) iso/boot/${KERNEL}.elf
	$(RM) iso/boot/${KERNEL}.sym
	$(RM) *.iso
	$(RM) include/irq.h
	$(RM) include/config.h
