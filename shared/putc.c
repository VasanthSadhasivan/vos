#include"putc.h"
void putc(uint8_t character) {
	uint64_t one = 1;
	uint64_t character_long = character;

	asm volatile("mov %0, %%r12;"
	"mov %1, %%r13;"
	"int %2;" : : "r"(one), "r"(character_long), "n"(0x80) : "r12", "r13");
}
