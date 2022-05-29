#include"vga_driver.h"

static unsigned short *vgaBuff = (unsigned short*) VGA_BASE;
static unsigned short cursor = 0;
static unsigned short color = 0x4f00;

void VGA_clear() {
	unsigned short i;
	for (i = 0; i < 1600; i++) {
		vgaBuff[i] = 0;
	}
}

void VGA_display_char(char a) {
	uint8_t local_interrupt_flag = 0x00;

	CLI_IF;
	
	if(a == '\0') {
		STI_IF;
		return;
	}

	SER_Write((uint8_t) a);

	if (a == '\n' || a == '\r') {
		if (cursor >= 1520) {
			my_memcpy((char *)VGA_BASE, ((char *)VGA_BASE) + 80 * sizeof(unsigned short), sizeof(unsigned short) * (1600 - 80));
			my_memset(((char *)VGA_BASE) + 1520 * sizeof(unsigned short), 0x00, 80 * sizeof(unsigned short));
			cursor = 1520;	
		}else{
			cursor = (80 * (1 + (cursor / 80))) % 1600;
		}
	} else{
		vgaBuff[cursor] = color | a;
		if (cursor == 1599) {
			my_memcpy((char *)VGA_BASE, ((char *)VGA_BASE + 80 * sizeof(unsigned short)), 1520 * sizeof(unsigned short));
			my_memset(((char *) VGA_BASE + 1520 * sizeof(unsigned short)), 0x00, 80 * sizeof(unsigned short));
			cursor = 1520;
		} else {
			cursor = (cursor + 1) % 1600;
		}
	}

	STI_IF;
}

void VGA_display_string(const char *a) {
	unsigned long i;
	
	for (i = 0; i < strlen((char *) a); i++) {
		VGA_display_char(a[i]);
	}
}

int VGA_row_count() {
	return 20;
}

int VGA_col_count() {
	return 80;
}

void VGA_display_attr_char(int x, int y, char c, int fg, int bg) {
	vgaBuff[x + y * 80] = (fg << 8 | bg << 12) | c;
}

