#include"yield.h"
void yield() {
	uint64_t two = 2;

	asm volatile("mov %0, %%r12;"
        "int %1;" : : "r"(two), "n"(0x80) : "r12");
}
