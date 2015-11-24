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

${KERNEL}.iso: ${KERNEL}.elf ${KERNEL}.sym grub.cfg.in
	test -d iso/boot/grub || mkdir -p iso/boot/grub
	cp ${KERNEL}.elf iso/boot/
	cp ${KERNEL}.sym iso/boot/
	sed 's/@KERNEL@/${KERNEL}/g' grub.cfg.in > iso/boot/grub/grub.cfg
	grub-mkrescue -o $@ --product-name ${KERNEL} iso

${KERNEL}.sym: ${KERNEL}.elf
	$(NM) -nC $< > $@

${KERNEL}.elf: ${OBJS} link.ld
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

qemu: ${KERNEL}.elf
	$(QEMU) -m 16M -kernel ${KERNEL}.elf

dump: ${KERNEL}.elf
	$(OBJDUMP) -d ${KERNEL}.elf | less

gdb: ${KERNEL}.elf
	$(QEMU) -s -S -kernel ${KERNEL}.elf &
	$(GDB) ${KERNEL}.elf -ex 'target remote 127.0.0.1:1234'

loc:
	find . -name '*.[hcS]' | xargs wc -l | sort -n

tags:
	ctags -R

cscope:
	find . -name '*.[hcS]' > cscope.files
	cscope -bk

size: ${KERNEL}.elf
	@$(SIZE) -A ${KERNEL}.elf

strip: ${KERNEL}.elf
	$(STRIP) -R .comment -g ${KERNEL}.elf

clean:
	$(RM) ${OBJS} ${DEPS}
	$(RM) *.iso *.elf *.sym
	$(RM) -r iso
	$(RM) include/irq.h
	$(RM) include/config.h
