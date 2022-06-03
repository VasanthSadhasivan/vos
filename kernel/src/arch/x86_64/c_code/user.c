#include"user.h"

pt1_entry_t *new_user_page_table() {
	pt1_entry_t *user_page_table;

	user_page_table = (pt1_entry_t *) pfalloc();
	setup_user_paging(user_page_table);

	return user_page_table;
}

kthread_function_t *load_user_elf(uint8_t *filename, pt1_entry_t *user_page_table) {
	kthread_function_t *thread_loc;
	
	thread_loc = load_elf_separate_pt(filename, user_page_table);

	return thread_loc;
}
