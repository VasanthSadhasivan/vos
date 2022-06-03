#include"user.h"

void start_user_process(uint8_t *filename) {
	kthread_function_t *thread_loc;
	pt1_entry_t *user_page_table;

	user_page_table = (pt1_entry_t *) pfalloc();
	setup_user_paging(user_page_table);
	
	thread_loc = load_elf_separate_pt(filename, user_page_table);

	PROC_create_thread(thread_loc, user_page_table, 0x00);
}
