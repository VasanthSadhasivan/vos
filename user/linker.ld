ENTRY(main)

SECTIONS {
	. = 0x20000000000;
	
	.text : {
		*(.text)
	}
}
