#include"getc.h"
uint8_t getc() {
	uint64_t character;
	uint64_t zero = 0;

	asm volatile("mov %0, %%r12;"
	"mov %1, %%r13;"
        "int %2;" : : "r"(zero), "r"(&character), "n"(0x80) : "r12", "r13");
	return (uint8_t) character;
}
