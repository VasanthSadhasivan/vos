#include"kthread.h"

kthread_t *PROC_context;
uint64_t proc_id_counter;

void PROC_init() {
	PROC_head = (kthread_t *) 0;
	PROC_context = (kthread_t *) 0;
	proc_id_counter = 0;
	isr_set_handler(0xFE, on_yield);
	isr_set_handler(0xFD, on_PROC_run);
	isr_set_handler(0xFC, on_kexit);
	isr_set_handler(0xFB, on_return);
	isr_set_handler(0xFA, on_block);

	PROC_external_queue_init();
}

kthread_t *PROC_create_kthread(kthread_function_t thread_loc, void *arg) {
        uint16_t cs, ss;
	kthread_t *curr;
	kthread_t *thread_p;
        
	asm volatile("mov %%cs, %0" : "=r"(cs));

	thread_p = (kthread_t *) kmalloc(sizeof(kthread_t));
	my_memset((uint8_t *)thread_p, 0x00, sizeof(kthread_t));
	thread_p -> rbp = (uint64_t) (kmalloc(5*1024) + 5 * 1024);
	thread_p -> rsp = thread_p -> rbp;
	thread_p -> rip = (uint64_t) thread_loc;
	thread_p -> rdi = (uint64_t) arg;
	thread_p -> cs = cs;
	thread_p -> ss = 0;
	thread_p -> next = (kthread_t *) 0;
	thread_p -> pid = ++proc_id_counter;
	thread_p -> rflags = 0x246;
	thread_p -> page_table_base = kernel_page_table_base;

	if(PROC_head == (kthread_t *) 0) {
		PROC_head = thread_p;
	} else {
		for(curr = PROC_head; curr -> next != (kthread_t *) 0; curr = curr -> next);
		curr -> next = thread_p;
	}

	return thread_p;
}

kthread_t *PROC_create_thread(kthread_function_t thread_loc, pt1_entry_t *user_page_table_base, void *arg) {
        uint16_t cs, ss;
	kthread_t *curr;
	kthread_t *thread_p;
        
	cs = 0x13;
	ss = 0x1B;

	thread_p = (kthread_t *) kmalloc(sizeof(kthread_t));
	my_memset((uint8_t *)thread_p, 0x00, sizeof(kthread_t));
	thread_p -> rbp = (uint64_t) 0x60000000000 - 1;
	thread_p -> rsp = thread_p -> rbp;
	thread_p -> rip = (uint64_t) thread_loc;
	thread_p -> rdi = (uint64_t) arg;
	thread_p -> cs = cs;
	thread_p -> ss = ss;
	thread_p -> next = (kthread_t *) 0;
	thread_p -> pid = ++proc_id_counter;
	thread_p -> rflags = 0x246;
	thread_p -> page_table_base = user_page_table_base;

	if(PROC_head == (kthread_t *) 0) {
		PROC_head = thread_p;
	} else {
		for(curr = PROC_head; curr -> next != (kthread_t *) 0; curr = curr -> next);
		curr -> next = thread_p;
	}

	return thread_p;
}

void PROC_reschedule() {
	kthread_t *curr;
	kthread_t *temp;

	if (PROC_head != (kthread_t *) 0) {
		for(curr = PROC_head; curr -> next != (kthread_t *) 0; curr = curr -> next);
		curr -> next = PROC_head;
		temp = PROC_head;
		PROC_head = PROC_head -> next;
		temp -> next = (kthread_t *) 0;
	}
}

void PROC_run() {
	PROC_context = (kthread_t *) kmalloc(sizeof(kthread_t));
	my_memset((uint8_t *) PROC_context, 0x00, sizeof(kthread_t));
	asm volatile("int %0" : : "n"(0xFD));
}

void yield() {
	asm volatile("int %0" : : "n"(0xFE));
}

void on_yield() {
	uint64_t *isr_rbp;

	asm volatile("mov %%rbp, %0" : "=r"(isr_rbp));
	isr_rbp = *((uint64_t **) isr_rbp);

	save_registers(isr_rbp, PROC_head);

	PROC_reschedule();

	load_registers(isr_rbp, PROC_head);
}

void on_PROC_run() {
	uint64_t *isr_rbp;

	asm volatile("mov %%rbp, %0" : "=r"(isr_rbp));
	isr_rbp = *((uint64_t **) isr_rbp);

	save_registers(isr_rbp, PROC_context);

	load_registers(isr_rbp, PROC_head);
}

void on_kexit() {
	uint64_t *isr_rbp;

	asm volatile("mov %%rbp, %0" : "=r"(isr_rbp));
	isr_rbp = *((uint64_t **) isr_rbp);

	load_registers(isr_rbp, PROC_head);
}

void on_return() {
	uint64_t *isr_rbp;

	asm volatile("mov %%rbp, %0" : "=r"(isr_rbp));
	isr_rbp = *((uint64_t **) isr_rbp);

	if(PROC_context == (kthread_t *) 0) {
		printk("Original Context Missing! Halting\n");
		asm volatile("hlt");
	}

	load_registers(isr_rbp, PROC_context);
	PROC_context = (kthread_t *) 0;
}

void kexit() {
	kthread_t *old_head = PROC_head;
	PROC_head = old_head -> next;
	kfree((uint8_t *) old_head -> rbp - 5 * 1024);
	kfree((uint8_t *) old_head);
	
	if (PROC_head == (kthread_t *) 0) {
		asm volatile("int %0" : : "n"(0xFB));
	} else { 
		asm volatile("int %0" : : "n"(0xFC));
	}
}

void save_registers(uint64_t *isr_rbp, kthread_t *thread_p) {
	// TODO: exit user mode if in user
        thread_p -> gs = (isr_rbp + 2)[0];
        thread_p -> fs = (isr_rbp + 3)[0];
        thread_p -> es = (isr_rbp + 4)[0];
        thread_p -> ds = (isr_rbp + 5)[0];
        thread_p -> r15 = (isr_rbp + 6)[0];
        thread_p -> r14 = (isr_rbp + 7)[0];
        thread_p -> r13 = (isr_rbp + 8)[0];
        thread_p -> r12 = (isr_rbp + 9)[0];
        thread_p -> r11 = (isr_rbp + 10)[0];
        thread_p -> r10 = (isr_rbp + 11)[0];
        thread_p -> r9 = (isr_rbp + 12)[0];
        thread_p -> r8 = (isr_rbp + 13)[0];
        thread_p -> rdi = (isr_rbp + 14)[0];
        thread_p -> rsi = (isr_rbp + 15)[0];
        thread_p -> rbp = (isr_rbp + 16)[0];
        thread_p -> rbx = (isr_rbp + 17)[0];
        thread_p -> rdx = (isr_rbp + 18)[0];
        thread_p -> rcx = (isr_rbp + 19)[0];
        thread_p -> rax = (isr_rbp + 20)[0];
	
	thread_p -> rip = (isr_rbp + 21)[0];
	thread_p -> cs = (uint16_t) (isr_rbp + 22)[0];
	thread_p -> rflags = (isr_rbp + 23)[0];
	thread_p -> rsp = (isr_rbp + 24)[0];
	thread_p -> ss = (uint16_t) (isr_rbp + 25)[0];
	
	if(thread_p -> page_table_base == 0x00) {
		thread_p -> page_table_base = kernel_page_table_base;
	}
}

void load_registers(uint64_t *isr_rbp, kthread_t *thread_p) {
        (isr_rbp + 2)[0] = thread_p -> gs; 
        (isr_rbp + 3)[0] = thread_p -> fs; 
        (isr_rbp + 4)[0] = thread_p -> es; 
        (isr_rbp + 5)[0] = thread_p -> ds; 
        (isr_rbp + 6)[0] = thread_p -> r15;
        (isr_rbp + 7)[0] = thread_p -> r14;
        (isr_rbp + 8)[0] = thread_p -> r13;
        (isr_rbp + 9)[0] = thread_p -> r12;
        (isr_rbp + 10)[0] = thread_p -> r11;
        (isr_rbp + 11)[0] = thread_p -> r10;
        (isr_rbp + 12)[0] = thread_p -> r9;
        (isr_rbp + 13)[0] = thread_p -> r8; 
        (isr_rbp + 14)[0] = thread_p -> rdi;
        (isr_rbp + 15)[0] = thread_p -> rsi;
        (isr_rbp + 16)[0] = thread_p -> rbp;
        (isr_rbp + 17)[0] = thread_p -> rbx;
        (isr_rbp + 18)[0] = thread_p -> rdx;
        (isr_rbp + 19)[0] = thread_p -> rcx;
        (isr_rbp + 20)[0] = thread_p -> rax;
	
	(isr_rbp + 21)[0] = thread_p -> rip;
	(isr_rbp + 22)[0] = thread_p -> cs;
	(isr_rbp + 23)[0] = thread_p -> rflags;
	(isr_rbp + 24)[0] = thread_p -> rsp;
	(isr_rbp + 25)[0] = thread_p -> ss;
	//TODO: Enter user mode
	load_new_page_table(thread_p -> page_table_base);
}

void PROC_external_queue_init() {
	PROC_keyboard_queue.procs = 0;
	PROC_keyboard_queue.read_i = 0;
	PROC_keyboard_queue.write_i = 0;
	PROC_keyboard_queue.buffer = kmalloc(256 * sizeof(uint8_t));

	PROC_primary_ata_queue.procs = 0;
	PROC_primary_ata_queue.read_i = 0;
	PROC_primary_ata_queue.write_i = 0;
	PROC_primary_ata_queue.buffer = kmalloc(256 * sizeof(command_t));
	
	PROC_secondary_ata_queue.procs = 0;
	PROC_secondary_ata_queue.read_i = 0;
	PROC_secondary_ata_queue.write_i = 0;
	PROC_secondary_ata_queue.buffer = kmalloc(256 * sizeof(command_t));
}

void PROC_block_on(external_queue_t *queue, uint8_t enable_ints) {
	if (PROC_context == (kthread_t *) 0 || PROC_head -> next == (kthread_t *) 0) {
		STI;
		asm volatile("hlt");
	}else {
		asm volatile("int %0" : : "n"(0xFA));
	}
}

void on_block() {
	kthread_t *old_head;
	kthread_t *thread_p;
	kthread_t *temp;
	uint64_t *isr_rbp;
	external_queue_t *queue;

	asm volatile("mov %%rbp, %0" : "=r"(isr_rbp));
	isr_rbp = *((uint64_t **) isr_rbp);
	queue = (external_queue_t *) (isr_rbp + 14)[0];

	thread_p = (kthread_t *) kmalloc(sizeof(kthread_t));
	save_registers(isr_rbp, thread_p);
	
	if(queue -> procs) {
		for(temp = queue -> procs; temp -> next; temp = temp -> next);
		temp -> next = thread_p;
		thread_p -> next = (kthread_t *) 0x00;
	} else {
		queue -> procs = thread_p;
		thread_p -> next = (kthread_t *) 0x00;
	}

	old_head = PROC_head;
	PROC_head = old_head -> next;
	kfree((uint8_t *) old_head);
	load_registers(isr_rbp, PROC_head);
}

void PROC_unblock_all(external_queue_t *queue) {
	while(queue -> procs) {
		PROC_unblock_head(queue);
	}
}

void PROC_unblock_head(external_queue_t *queue) {
	kthread_t *thread_to_add;
	kthread_t *curr;

	thread_to_add = queue -> procs;

	if (thread_to_add) {
		queue -> procs = thread_to_add -> next;
	}
	
	if (PROC_head) {
		for(curr = PROC_head; curr -> next; curr = curr -> next);

		curr -> next = thread_to_add;
		if (thread_to_add) {
			thread_to_add -> next = (kthread_t *) 0;
		}
	} else {
		PROC_head = thread_to_add;
	}
}
