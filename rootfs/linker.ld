SECTIONS
{
  	. = 0x0;
  	.text : {
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

  	/DISCARD/ : {
		*(.eh_frame .note.GNU-stack)
	}
}