LINK_SCRIPT= arch/i386/link.ld
CFLAGS += -m32 -Iarch/i386/include
LDFLAGS += -melf_i386
CROSS_COMPILE =
ASMOBJS += arch/i386/boot/multiboot.o arch/i386/kernel/isr.o
COBJS += arch/i386/kernel/idt.o arch/i386/kernel/gdt.o \
	 arch/i386/drivers/text_buffer.o arch/i386/drivers/pic.o \
	 arch/i386/drivers/pit.o arch/i386/drivers/keyboard.o \
	 arch/i386/drivers/cmos.o arch/i386/kernel/interrupt.o \
	 arch/i386/kernel/arch.o
OUTPUT := ${KERNEL}.iso
${KERNEL}.iso: ${KERNEL}.elf ${KERNEL}.sym arch/i386/boot/grub.cfg.in
	test -d iso/boot/grub || mkdir -p iso/boot/grub
	cp ${KERNEL}.elf iso/boot/
	cp ${KERNEL}.sym iso/boot/
	sed 's/@KERNEL@/${KERNEL}/g' arch/i386/boot/grub.cfg.in > iso/boot/grub/grub.cfg
	grub-mkrescue -o $@ --product-name ${KERNEL} iso

arch/i386/kernel/idt.o: arch/i386/include/irq.h

arch/i386/kernel/idt.d: arch/i386/include/irq.h

arch/i386/kernel/isr.o: arch/i386/include/irq.h include/config.h \
	include/kernel.h include/stddef.h

arch/i386/boot/multiboot.o: include/kernel.h include/stddef.h include/config.h

arch/i386/kernel/interrupt.o: arch/i386/include/irq.h

arch/i386/include/irq.h: arch/i386/gen_irq.sh
	./arch/i386/gen_irq.sh > $@

arch-clean:
	$(RM) arch/i386/include/irq.h
	$(RM) -r iso
