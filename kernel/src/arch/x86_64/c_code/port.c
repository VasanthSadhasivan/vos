#include"port.h"

void outb(uint8_t value, uint16_t port) {
	asm volatile ( "outb %0, %1" : : "a"(value), "Nd"(port) );
}

uint8_t inb(uint16_t port) {
	uint8_t value;

	asm volatile ( "inb %1, %0"
			: "=a"(value)
			: "Nd"(port) );
	return value;
}

void outw(uint16_t value, uint16_t port) {
	asm volatile ( "outw %0, %1" : : "a"(value), "Nd"(port) );
}

uint16_t inw(uint16_t port) {
	uint16_t value;

	asm volatile ( "inw %1, %0"
			: "=a"(value)
			: "Nd"(port) );
	return value;
}

void outl(uint32_t value, uint16_t port) {
	asm volatile ( "outl %0, %1" : : "a"(value), "Nd"(port) );
}

uint32_t inl(uint16_t port) {
	uint32_t value;

	asm volatile ( "inl %1, %0"
			: "=a"(value)
			: "Nd"(port) );
	return value;
}
