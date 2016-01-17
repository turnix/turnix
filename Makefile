KERNEL := turnix
CFLAGS += -Wall -Werror -Wextra -O2 -nostdlib -nostdinc -fno-builtin \
	  -fno-stack-protector -nostartfiles -nodefaultlibs -std=gnu99 \
	  -Iinclude -ffreestanding
KERNEL_RELEASE_VCS := $(shell git describe --dirty 2>/dev/null | sed -e 's/^[0-9.]*//')
KERNEL_VERSION := $(shell date --iso-8601=seconds)
ARCH ?= i386
CFLAGS += -DKERNEL='"${KERNEL}"'
CFLAGS += -DKERNEL_RELEASE_VCS='"${KERNEL_RELEASE_VCS}"'
CFLAGS += -DKERNEL_VERSION='"${KERNEL_VERSION}"'
CFLAGS += -DKERNEL_ARCH='"${ARCH}"'
include arch/${ARCH}/Makefile
LDFLAGS += -T ${LINK_SCRIPT}
CC := ${CROSS_COMPILE}gcc
LD := ${CROSS_COMPILE}ld
AS := ${CROSS_COMPILE}as
NM := ${CROSS_COMPILE}nm
OBJDUMP := ${CROSS_COMPILE}objdump
READELF := ${CROSS_COMPILE}readelf
SIZE := ${CROSS_COMPILE}size
STRIP := ${CROSS_COMPILE}strip
GDB := gdb
SHELL := /bin/bash
QEMU ?= qemu-system-${ARCH}
APPLICATION ?= applications/shell.o
export SHELLOPTS := errexit:pipefail

include Config.mk

ifeq (${CONFIG_DEBUG},1)
	CFLAGS += -g
else
	CFLAGS += -fdata-sections -ffunction-sections -DNDEBUG
	LDFLAGS += --gc-sections --print-gc-sections
endif

COBJS += kernel/main.o lib/string.o lib/stdio.o lib/stdlib.o \
	 kernel/interrupt.o lib/hexdump.o kernel/timer.o lib/circular_buffer.o \
	 kernel/pthread.o lib/readline.o ${APPLICATION} lib/time.o \
	 kernel/utsname.o
DEPS = $(COBJS:.o=.d)
OBJS = ${ASMOBJS} ${COBJS}

.DELETE_ON_ERROR:

.PHONY: all qemu dump gdb loc clean arch-clean cscope tags size strip .FORCE

all: ${OUTPUT}

${KERNEL}.sym: ${KERNEL}.elf
	$(NM) -nC $< > $@

${KERNEL}.elf: ${OBJS} ${LINK_SCRIPT}
	$(LD) -o $@ $(filter %.o,$^) ${LDFLAGS}

.c.o:
	$(CC) -c -o $@ $< ${CFLAGS}

.S.o:
	$(CC) -c -o $@ $< ${CFLAGS}

%.o: %.c %.d

%.d: %.c include/config.h
	$(CC) -M $< ${CFLAGS} | \
		sed -e 's;^\(.*\)\.o\(.*\);$(@D)/\1.o $(@D)/\1.d\2;' > $@

kernel/utsname.o: .FORCE

include/config.h: Config.mk
	$(shell echo "/* DO NOT EDIT IT */" >$@)
	$(shell echo "#ifndef CONFIG_H" >>$@)
	$(shell echo "#define CONFIG_H" >>$@)
	$(foreach cfg,$(filter CONFIG_%,${.VARIABLES}),$(shell echo '#define $(cfg) $($(cfg))' >>$@))
	$(shell echo "#endif  /* CONFIG_H */" >>$@)

-include $(DEPS)

qemu:
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

clean: arch-clean
	$(RM) ${OBJS} ${DEPS}
	$(RM) ${OUTPUT} *.elf *.sym
	$(RM) include/config.h
