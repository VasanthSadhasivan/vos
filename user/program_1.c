#include"program_1.h"

int main(int argx, char **argv) {
	uint16_t i;

	putc('c');
	putc('c');
	putc('a');
	putc('b');
	putc('c');
	for (i = 0; i < 10; i++) {
		putc(getc());
	}
      asm volatile("int %0;" : : "n"(0x81));
	
}
