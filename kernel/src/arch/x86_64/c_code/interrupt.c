#include"interrupt.h"

gate_descriptor idt64[256];
idt_descriptor idtr;
Handler handlers[256];


void pic_setup() {
	inb(PIC1_DATA);
	inb(PIC2_DATA);

	outb(0x11, PIC1_COMMAND);
	outb(0x11, PIC2_COMMAND);

	outb(0x20, PIC1_DATA);
	outb(0x28, PIC2_DATA);

	outb(0x04, PIC1_DATA);
	outb(0x02, PIC2_DATA);
	
	outb(0x01, PIC1_DATA);
	outb(0x01, PIC2_DATA);

	outb(0xFF, PIC1_DATA);
	outb(0xFF, PIC2_DATA);
}

void mask_interrupt(uint8_t interrupt_number) {
	uint16_t mask;

	if(interrupt_number < 0x20){
		return;
	}

	interrupt_number = interrupt_number - 0x20;

	if(interrupt_number >= 0x10){
		return;
	}

	mask = inb(PIC1_DATA);
	mask |= (inb(PIC2_DATA) <<8);
	mask |= (1 << interrupt_number);
	
	outb(0x11, PIC1_COMMAND);
	outb(0x11, PIC2_COMMAND);

	outb(0x20, PIC1_DATA);
	outb(0x28, PIC2_DATA);

	outb(0x04, PIC1_DATA);
	outb(0x02, PIC2_DATA);
	
	outb(0x01, PIC1_DATA);
	outb(0x01, PIC2_DATA);
	
	outb((uint8_t) mask, PIC1_DATA);
	outb((uint8_t) (mask >> 8), PIC2_DATA);
}

void unmask_interrupt(uint8_t interrupt_number) {
	uint16_t mask;

	if(interrupt_number < 0x20){
		return;
	}

	interrupt_number = interrupt_number - 0x20;

	if(interrupt_number >= 0x10){
		return;
	}

	mask = inb(PIC1_DATA);
	mask |= (inb(PIC2_DATA) <<8);
	mask &= ~(1 << interrupt_number);
	
	if(interrupt_number >= 0x8){
		mask &= ~(1 << 2); 
	}


	outb(0x11, PIC1_COMMAND);
	outb(0x11, PIC2_COMMAND);

	outb(0x20, PIC1_DATA);
	outb(0x28, PIC2_DATA);

	outb(0x04, PIC1_DATA);
	outb(0x02, PIC2_DATA);
	
	outb(0x01, PIC1_DATA);
	outb(0x01, PIC2_DATA);

	outb((uint8_t) mask, PIC1_DATA);
	outb((uint8_t) (mask >> 8), PIC2_DATA);
}


void isr_set_handler(uint8_t interrupt_number, Handler handler) {
	handlers[interrupt_number] = handler;
}

void set_idt_stub(uint8_t interrupt_number, Handler assembly_stub) {
	idt64[interrupt_number].offset1 = (uint16_t) ((uint64_t) assembly_stub);
	idt64[interrupt_number].offset2 = (uint16_t) (((uint64_t)assembly_stub) >> 16);
	idt64[interrupt_number].offset3 = (uint32_t) (((uint64_t)assembly_stub) >> 32);
}

void isr_setup() {
	uint16_t i;
	system_segment_descriptor *tss_descriptor;

	idtr.size = sizeof(gate_descriptor) * 256 - 1;
	idtr.offset = (uint64_t) idt64;	
	my_memset((char *) handlers, 0, sizeof(Handler));

	for(i = 0; i < 256; i++) {
		set_idt_stub((uint8_t) i, HandleGeneric);
		idt64[i].segment_selector = 0x08;
		if(i < 32) {
			idt64[i].group_of_data = 0x8E02;
		} else if(i == 0x80) {
			idt64[i].group_of_data = 0x8E03;	
		}else{
			idt64[i].group_of_data = 0x8E01;
		}
		idt64[i].reserved = 0x0;
	}

	set_idt_stubs();

	pic_setup();
	printk("Setup PIC\n");

	tss_descriptor = (system_segment_descriptor *) (&gdt64 + 8*2);
	tss_descriptor -> limit1 = 0x68;
	tss_descriptor -> limit2 = 0;
	tss_descriptor -> base1 = (((uint64_t)&tss) & 0xFFFFFF);
	tss_descriptor -> base2 = (((uint64_t)&tss) & 0xFF000000) >> 24;
	tss_descriptor -> base3 = (((uint64_t)&tss) & 0xFFFFFFFF00000000) >> 32;
	tss_descriptor -> access = 0x89;
	tss_descriptor -> flags = 0x02;

	asm volatile("movw %0, %%ax; ltr %%ax"
		      :
		      :"n"((uint16_t)0x10)
		      :"%ax");

	asm volatile("lidt %0" : : "m"(idtr));
	printk("Loaded interrupt table\n");
}

void isr(uint64_t interrupt_number){
	//printk("Interrupt Triggered: %d\n", interrupt_number);
	
	// No handler setup in IDT entry
	if (interrupt_number == 256) {
		printk("Handler not set in IDT: %d\n", interrupt_number);
		return;
	}

	// No C handler set up to run for isr
	if(0x00 == handlers[interrupt_number]) {
		printk("Handler not set in C list: %d\n", interrupt_number);
		return;
	}else{
		handlers[interrupt_number]();
	}

	if(interrupt_number >= 0x20) {
		if (interrupt_number >= 0x28) {
			outb(0x20, PIC2_COMMAND);
		}

		outb(0x20, PIC1_COMMAND);
	}

}


void handle_timer() {
	//printk("Inside timer interrupt");	
}
