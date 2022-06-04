#include"kernel.h"

void kernel_main(uint8_t *multiboot_info_header_p) {
	uint8_t i = 0;
	while(!i);
	
	setup_frame_allocator((multiboot_info_header_t *) multiboot_info_header_p);
	isr_setup();
	initialize_ps2();
	initialize_keyboard();
	SER_Init();
	setup_paging();
	while(1){
		asm volatile("hlt");
	}
}

void kernel_virtualized_main() {
	uint8_t *data;
	pt1_entry_t *user_page_table;

	printk("In Virtual Memory!\n");
	kmem_init();
	setup_ata_devices();
	PROC_init();
	FAT_init();
	syscall_init();

	//PROC_create_kthread(load_elf("/boot/user/program_1.elf"), data);
	user_page_table = new_user_page_table();
	PROC_create_thread(load_user_elf("/boot/user/program_1.elf", user_page_table), user_page_table, 0x00);
	user_page_table = new_user_page_table();
	PROC_create_thread(load_user_elf("/boot/user/program_2.elf", user_page_table), user_page_table, 0x00);
	PROC_run();
	//PROC_create_kthread(thread_1, data);
	//PROC_create_kthread(thread_2, data);
	//PROC_create_kthread(demo, data);
	//PROC_run();
	//PROC_init();
	//setup_snakes(1);

	printk("Back in Virtual Land\n");
	//read_root_directory();


	printk("Value at addr 0: %d\n", ((uint8_t *) 0)[0]);
	while(1);
}

void thread_1(void *value) {
	uint8_t i;

	while(1){printk("%c", getc());};

	kexit();
}

void thread_2(void *value) {
	uint8_t i;

	while(1){yield();};

	kexit();
}
