#ifndef SERIAL_H__
#define SERIAL_H__
#include"port.h"
#include"interrupt.h"

#define PORT 0x3F8

typedef struct __attribute__((packed)) {
	uint8_t buffer[16];
	uint8_t consumer_i;
	uint8_t producer_i;
} ser_state_t;

void SER_Init();
uint8_t SER_Write(uint8_t data);
void on_interrupt();
uint8_t full();
uint8_t empty();
void transmit();
void init_serial();
#endif
