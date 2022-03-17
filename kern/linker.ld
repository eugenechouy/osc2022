SECTIONS
{
  	. = 0x80000;
  	.text : {
    	KEEP(*(.text.entry))
		*(.text .text.* .gnu.linkonce.t.*)
	}

  	.rodata : {
		*(.rodata .rodata.* .gnu.linkonce.r.*)
	}

	. = ALIGN(0x1000);

	/* The data segment */
	.data : {
		*(.data)
	}

	.bss ALIGN(16) (NOLOAD) : {
		PROVIDE(__bss_start = .);
		*(.bss)
		PROVIDE(__bss_end = .);
		BYTE(0)
	}

	. = ALIGN(0x1000);

	__heap_start = .;

  	/DISCARD/ : {
		*(.eh_frame .note.GNU-stack)
	}
}

__stack_top = 0x20000000;
__heap_end = __heap_start + 0x3200000;
__bss_size = (__bss_end - __bss_start) >> 3;