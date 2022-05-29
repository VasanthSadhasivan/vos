#ifndef INTERRUPT_H__

#define INTERRUPT_H__

#include"my_stdio.h"
#include"port.h"
#include"my_stdlib.h"
#include"interrupt_stub_setting.h"

#include<stdint.h>

#define PIC1 0x20
#define PIC2 0xA0
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1 + 1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2 + 1)
#define CLI_IF if(interrupts_enabled()){asm volatile("cli"); local_interrupt_flag = 0xFF;}
#define STI_IF if(local_interrupt_flag){ asm volatile("sti"); local_interrupt_flag = 0x00;}

#define CLI asm volatile("cli")
#define STI asm volatile("sti")

typedef struct __attribute__((packed)) {
	uint16_t offset1;
	uint16_t segment_selector;
	uint16_t group_of_data;
	uint16_t offset2;
	uint32_t offset3;
	uint32_t reserved;
} gate_descriptor;

typedef struct __attribute__((packed)){
	uint16_t size;
	uint64_t offset;
} idt_descriptor;

typedef struct __attribute__((packed)) { 
        uint64_t limit1 : 16; 
        uint64_t base1 : 24; 
        uint64_t access : 8; 
        uint64_t limit2 : 4; 
        uint64_t flags : 4; 
        uint64_t base2 : 8; 
        uint64_t base3 : 32; 
        uint64_t reserved : 32; 
} system_segment_descriptor; 


typedef void (*Handler)();

void pic_setup();
void isr_setup();
void isr(uint64_t);
void isr_set_handler(uint8_t, Handler);
void set_idt_stub(uint8_t, Handler);
void handle_timer();
void mask_interrupt(uint8_t);
void unmask_interrupt(uint8_t);

extern void HandleGeneric();
extern  uint8_t  gdt64;
extern  uint8_t  tss;
extern uint8_t interrupts_enabled();

#endif
