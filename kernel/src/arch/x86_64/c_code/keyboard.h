#ifndef KEYBOARD_H__
#define KEYBOARD_H__

#include"port.h"
#include"my_stdio.h"
#include"interrupt.h"
#include"kthread.h"
#include<stdint.h>

#define DEBUG 0 


uint8_t read_ps2_data();
uint16_t read_ps2_data_nonblocking();
uint8_t read_ps2_control();
void write_ps2_data(uint8_t);
void write_ps2_control(uint8_t);
void write_keyboard_data(uint8_t);
void initialize_ps2();
void initialize_keyboard();
int8_t *read_keyboard();
int8_t *read_keyboard_nonblocking();
void handle_keyboard();
int8_t getc();
#endif

