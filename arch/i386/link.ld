ENTRY(_start)

SECTIONS {
	. = 1M;

	. = ALIGN(4K);
	.text : {
		*(.multiboot)
		*(.text)
	}

	. = ALIGN(4K);
	.rodata : {
		application_init_begin = .;
		KEEP(*(.application.init))
		application_init_end = .;
		shell_cmd_begin = .;
		KEEP(*(.shell.cmd))
		shell_cmd_end = .;
		*(.rodata*)
	}

	. = ALIGN(4K);
	.data : {
		*(.data)
	}

	. = ALIGN(4K);
	.bss : {
		*(.bss)
		*(COMMON)
		*(.boot_stack)
	}

	kernel_end = .;
}
