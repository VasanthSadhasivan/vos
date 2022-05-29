#include"syscall.h"

void syscall_init() {
        //Unmask syscall interrupt
        unmask_interrupt(0x80);

        //Set syscall handler
        isr_set_handler(0x80, on_int80);
}

void on_int80() {
        uint64_t *isr_rbp;
	uint64_t r12;
	uint64_t r13;

        asm volatile("mov %%rbp, %0" : "=r"(isr_rbp));
        isr_rbp = *((uint64_t **) isr_rbp);
        
	r12 = (isr_rbp + 9)[0];
	r13 = (isr_rbp + 8)[0];
	
	switch(r12) {
		case 0:
			((uint8_t *)r13)[0] = getc();
			break;
		case 1:
			printk("%c", (uint8_t) r13);
			break;
		default:
			break;
	}
}

