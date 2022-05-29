#include"serial.h"

ser_state_t SER_state;
uint8_t SER_initialized;
uint8_t SER_initialied = 0;

void SER_Init() {
	SER_state.producer_i = 0;
	SER_state.consumer_i = 0;

	init_serial();

	//Unmask COM1 interrupt
	unmask_interrupt(0x24);

	//Set COM1 handler
	isr_set_handler(0x24, on_interrupt);

	SER_initialized = 0xFF;
}

uint8_t SER_Write(uint8_t data) {
	uint8_t local_interrupt_flag = 0x00;
	if(SER_initialized){
		CLI_IF;

		if (full()) {
			STI_IF;
			return 1;
		}

		SER_state.buffer[SER_state.producer_i] = data;
	
		if (empty()) {
			transmit();
		}

		SER_state.producer_i = (SER_state.producer_i + 1) % 16;
	
		STI_IF;
	}
	return 0;
}

void on_interrupt() {
	uint8_t local_interrupt_flag = 0x00;
	uint8_t iir;

	CLI_IF;
	
	iir = inb(PORT + 2);

	if ((iir & 0b1110) == 0b110) {
		inb(PORT + 5);
		STI_IF;
		return;
	}
	
	SER_state.consumer_i = (SER_state.consumer_i + 1 ) % 16;
	if (empty()) {
		STI_IF;
		return;
	}

	transmit();

	STI_IF;
	return;
}

uint8_t full() {
	return (
			((SER_state.consumer_i == 0) && (SER_state.producer_i == 16-1)) 
			||
			(SER_state.producer_i == (SER_state.consumer_i - 1))
	       );
}

uint8_t empty() {
	return (SER_state.producer_i == SER_state.consumer_i);
}

void transmit() {
	outb(SER_state.buffer[SER_state.consumer_i], PORT);
}

void init_serial() {
	outb(0x00, PORT + 3);
	outb(0x00, PORT + 1); //Disable all interrupts
	outb(0x80, PORT + 3);
	outb(0x03, PORT + 0);
	outb(0x00, PORT + 1);
	outb(0x03, PORT + 3);
	outb(0xC7, PORT + 2);
	outb(0x02, PORT + 1);
}
