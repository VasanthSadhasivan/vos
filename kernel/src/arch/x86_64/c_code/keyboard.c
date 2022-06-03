#include"keyboard.h"

uint8_t prev_read = 0;
static const int8_t* scancode_map[] = {
0, "F9", 0, "F5", "F3", "F1", "F2", "F12", 0, "F10", "F8", "F6", "F4", "\t", "`", 0, 0, "LeftAlt", "LeftShift", 0, "LeftControl", "q", "1", 0, 0, 0, "z", "s", "a", "w", "2", 0, 0, "c", "x", "d", "e", "4", "3", 0, 0, " ", "v", "f", "t", "r", "5", 0, 0, "n", "b", "h", "g", "y", "6", 0, 0, 0, "m", "j", "u", "7", "8", 0, 0, ",", "k", "i", "o", "0", "9", 0, 0, ".", "/", "l", ";", "p", "-", 0, 0, 0, "\'", 0, "[", "=", 0, 0, "CapsLock", "RightShift", "Enter\n", "]", 0, "\\", 0, 0, 0, 0, 0, 0, 0, 0, "Backspace", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Escape", 0, "F11", 0, 0, 0, 0, 0, "ScrollLock", 0, 0, 0, 0, "F7", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


uint8_t read_ps2_data() {
	uint8_t response_byte;

	do{
		response_byte = read_ps2_control();
	}while((response_byte & 0x01) == 0x0);
	
	response_byte = inb(0x60);

	if(DEBUG)
		printk("Read ps2 data: %x\n", response_byte);

	return response_byte;
}

uint16_t read_ps2_data_nonblocking() {
        uint8_t response_byte;

	if((read_ps2_control() & 0x01) == 0){
		return 0;
	}	

        response_byte = inb(0x60);

        if(DEBUG)
                printk("Read ps2 data: %x\n", response_byte);

        return response_byte;
}

uint8_t read_ps2_control() {
	uint8_t response_byte;

	response_byte = inb(0x64);

	if(DEBUG)
		printk("Read ps2 control: %x\n", response_byte);

	return response_byte;
}

void write_ps2_data(uint8_t value) {
	while(inb(0x64) & 0x02);
	outb(value, 0x60);

	if(DEBUG)
		printk("Write ps2 data: %x\n", value);
}

void write_ps2_control(uint8_t value) {
	while(inb(0x64) & 0x02);
	outb(value, 0x64);

	if(DEBUG)
		printk("Write ps2 control: %x\n", value);
}

void write_keyboard_data(uint8_t value) {
	uint8_t response_byte;

	do {
		write_ps2_data(value);
		response_byte = read_ps2_data();
	} while(response_byte != 0xFA);

	if(DEBUG)
		printk("Write keyboard data: %x\n", value);
}

void initialize_ps2() {
	uint8_t response_byte;
	uint8_t temp_byte;

	//Disable ps/2 both controllers
	write_ps2_control(0xAD);
	write_ps2_control(0xA7);

	//Control byte to retrieve config byte
	write_ps2_control(0x20);

	//Read config byte
	response_byte = read_ps2_data();
	
	//Modify response byte and disable clk and interrupt for second 1ps2 controler
	response_byte |= 0x21;
	response_byte &= 0xAD;

	//Next byte is configuration update byte
	write_ps2_control(0x60);
	
        //Send configuration byte
        write_ps2_data(response_byte);

	//Reenable first ps2
	write_ps2_control(0xAE);
}

void initialize_keyboard() {
	uint8_t response_byte;
	
	asm volatile("cli");
	do{
		//Reset keyboard device
		write_ps2_data(0xFF);
		response_byte = read_ps2_data();
	//loop until self-test passed
	}while(response_byte != 0xFA);

	while(read_ps2_data() != 0xAA);

	do{
		//Testing Echo
		write_ps2_data(0xEE);
		response_byte = read_ps2_data();
	//loop until can retrieve echo
	}while(response_byte != 0xEE);
	
	//Scan code setting command
	write_keyboard_data(0xF0);

	//Set scan code sub command to set 2
	write_keyboard_data(0x02);

	//Unmask keyboard interrupt
	unmask_interrupt(0x21);

	//Set keyboard handler
	isr_set_handler(0x21, handle_keyboard);
	
	//Enable scanning
	write_keyboard_data(0xF4);
	
	asm volatile("sti");	
}

int8_t *read_keyboard() {
	uint8_t scan_code;
	int8_t *value;
	
	//Read keyboard
	scan_code = read_ps2_data();
	
	value = (int8_t *) scancode_map[scan_code];
	
	read_ps2_data();
	read_ps2_data();
	
	return value;
}

int8_t *read_keyboard_nonblocking() {
        uint16_t scan_code;
        int8_t *value;

        //Read keyboard
        scan_code = read_ps2_data_nonblocking();
	if (scan_code == 0xFA || scan_code == 0xF0){
		return "";
	}

	if (prev_read) {
		prev_read = 0;
		return "";
	}

	prev_read = 1;
	value = (int8_t *) scancode_map[scan_code];

        return value;
}

void handle_keyboard() {
	//printk("Inside keyboard Interrupt");

	((uint8_t *) PROC_keyboard_queue.buffer)[PROC_keyboard_queue.write_i++] = (uint8_t)  read_keyboard_nonblocking()[0];
	PROC_unblock_head(&PROC_keyboard_queue);
}

int8_t getc() {
	uint8_t local_interrupt_flag = 0x00;
	CLI_IF;

	while(PROC_keyboard_queue.read_i == PROC_keyboard_queue.write_i) {
		PROC_block_on(&PROC_keyboard_queue, 1);
		CLI_IF;
	}
	
	STI_IF;
	return (uint8_t) ((uint8_t *) PROC_keyboard_queue.buffer)[PROC_keyboard_queue.read_i++];
}
