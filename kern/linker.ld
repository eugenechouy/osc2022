SECTIONS
{
	. = 0xffff000000000000;
  	. += 0x80000;
	__kernel_start = .;
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
	__kernel_pgd = .;
	.data.kpgd : {
		. += (515 * 0x1000);
	}

	__heap_start = .;

  	/DISCARD/ : {
		*(.eh_frame .note.GNU-stack)
	}
}

__stack_kernel_top 	= 0x20000000;
__stack_user_top 	= 0x30000000;
__bss_size = (__bss_end - __bss_start) >> 3;