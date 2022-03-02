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

  	/* Adjust the address for the data segment to the next page */
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

  	/DISCARD/ : {
		*(.eh_frame .note.GNU-stack)
	}
}

__stack_top = 0x20000000;
__bss_size = (__bss_end - __bss_start) >> 3;