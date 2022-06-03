ENTRY(main)

SECTIONS {
	. = 0x30000000000;
	
	.text : {
		*(.text)
	}
}
