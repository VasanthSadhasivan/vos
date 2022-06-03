#include"paging.h"

pt1_entry_t *current_page_table_base = (pt1_entry_t *) 0;

void setup_user_paging(pt1_entry_t *user_page_table_base) {
	pt1_entry_t *previous_page_table_base = current_page_table_base;
	current_page_table_base = user_page_table_base;
	my_memset((uint8_t *) user_page_table_base, 0x00, 4096);
	create_identity_map();
	create_section_start_mappings();
	current_page_table_base = previous_page_table_base;
}

void setup_paging() {
	kernel_page_table_base = (pt1_entry_t *) pfalloc();
	current_page_table_base = kernel_page_table_base;
	my_memset((uint8_t *) kernel_page_table_base, 0x00, 4096);

	create_identity_map();
	create_section_start_mappings();
	isr_set_handler(14, on_page_fault);
	virtualization_start(((uint64_t) kernel_page_table_base));// << 12);
}

void create_identity_map() {
	uint64_t i;

	for(i = 4096; i < (uint64_t) max_ram(); i+= 4096){
		make_pt_valid((uint8_t *) i);
		create_pt_mapping((uint8_t *)i, (uint8_t *)i);
	}
}

void make_pt_valid(uint8_t *virtual) {
	pt2_entry_t *pt2_base;
	pt3_entry_t *pt3_base;
	pt4_entry_t *pt4_base;

	uint16_t pt1_index = (((uint64_t) virtual) >> 39) & 0b111111111;
	uint16_t pt2_index = (((uint64_t) virtual) >> 30) & 0b111111111;
	uint16_t pt3_index = (((uint64_t) virtual) >> 21) & 0b111111111;
	uint16_t pt4_index = (((uint64_t) virtual) >> 12) & 0b111111111;

	pt1_entry_t pt1_value = current_page_table_base[pt1_index];
	if (pt1_value.present == 0) {
		pt2_base = (pt2_entry_t *) pfalloc();
		my_memset((uint8_t *) pt2_base, 0x00, 4096);

		current_page_table_base[pt1_index].present = 1;
		current_page_table_base[pt1_index].rw = 1;
		current_page_table_base[pt1_index].user_supervisor = 0;
		current_page_table_base[pt1_index].write_through = 0;
		current_page_table_base[pt1_index].page_cache_disabled = 1;
		current_page_table_base[pt1_index].accessed = 0;
		current_page_table_base[pt1_index].zero = 0;
		current_page_table_base[pt1_index].base_address = ((uint64_t) pt2_base) >> 12;
	} else {
		pt2_base = (pt2_entry_t *) (((uint64_t) pt1_value.base_address) << 12);
	}

	pt2_entry_t pt2_value = pt2_base[pt2_index];
	if (pt2_value.present == 0) {
		pt3_base = (pt3_entry_t *) pfalloc();
		my_memset((uint8_t *) pt3_base, 0x00, 4096);
		
		pt2_base[pt2_index].present = 1;
		pt2_base[pt2_index].rw = 1;
		pt2_base[pt2_index].user_supervisor = 0;
		pt2_base[pt2_index].write_through = 0;
		pt2_base[pt2_index].page_cache_disabled = 1;
		pt2_base[pt2_index].accessed = 0;
		pt2_base[pt2_index].zero = 0;
		pt2_base[pt2_index].base_address = ((uint64_t) pt3_base) >> 12;
	} else {
		pt3_base = (pt3_entry_t *) (((uint64_t) pt2_value.base_address) << 12);
	}

	pt3_entry_t pt3_value = pt3_base[pt3_index];
	if (pt3_value.present == 0) {
		pt4_base = (pt4_entry_t *) pfalloc();
		my_memset((uint8_t *) pt4_base, 0x00, 4096);

		pt3_base[pt3_index].present = 1;
		pt3_base[pt3_index].rw = 1;
		pt3_base[pt3_index].user_supervisor = 0;
		pt3_base[pt3_index].write_through = 0;
		pt3_base[pt3_index].page_cache_disabled = 1;
		pt3_base[pt3_index].accessed = 0;
		pt3_base[pt3_index].zero = 0;
		pt3_base[pt3_index].base_address = ((uint64_t) pt4_base) >> 12;
	} else {
		pt4_base = (pt4_entry_t *) (((uint64_t) pt3_value.base_address) << 12);
	}
	
	pt4_base[pt4_index].avl1 = 1;
}

void create_pt_mapping(uint8_t *virtual, uint8_t *physical) {
	uint8_t is_user_accessible;
	pt2_entry_t *pt2_base;
	pt3_entry_t *pt3_base;
	pt4_entry_t *pt4_base;

	uint16_t pt1_index = (((uint64_t) virtual) >> 39) & 0b111111111;
	uint16_t pt2_index = (((uint64_t) virtual) >> 30) & 0b111111111;
	uint16_t pt3_index = (((uint64_t) virtual) >> 21) & 0b111111111;
	uint16_t pt4_index = (((uint64_t) virtual) >> 12) & 0b111111111;

	pt1_entry_t pt1_value = current_page_table_base[pt1_index];

	is_user_accessible = (virtual >= (uint8_t *) 0x30000000000 && virtual < (uint8_t *) 0x40000000000) || (virtual >= (uint8_t *) 0x50000000000 && virtual < (uint8_t *) 0x60000000000) || (virtual >= (uint8_t *) 0x60000000000);

	if (pt1_value.present == 0) {
		pt2_base = (pt2_entry_t *) pfalloc();
		my_memset((uint8_t *) pt2_base, 0x00, 4096);

		current_page_table_base[pt1_index].present = 1;
		current_page_table_base[pt1_index].rw = 1;
		if (is_user_accessible){
			current_page_table_base[pt1_index].user_supervisor = 1;
		} else {
			current_page_table_base[pt1_index].user_supervisor = 0;
		}
		current_page_table_base[pt1_index].write_through = 0;
		current_page_table_base[pt1_index].page_cache_disabled = 1;
		current_page_table_base[pt1_index].accessed = 0;
		current_page_table_base[pt1_index].zero = 0;
		current_page_table_base[pt1_index].base_address = ((uint64_t) pt2_base) >> 12;
	} else {
		pt2_base = (pt2_entry_t *) (((uint64_t) pt1_value.base_address) << 12);
	}

	pt2_entry_t pt2_value = pt2_base[pt2_index];
	if (pt2_value.present == 0) {
		pt3_base = (pt3_entry_t *) pfalloc();
		my_memset((uint8_t *) pt3_base, 0x00, 4096);
		
		pt2_base[pt2_index].present = 1;
		pt2_base[pt2_index].rw = 1;
		if (is_user_accessible){
			pt2_base[pt2_index].user_supervisor = 1;
		} else {
			pt2_base[pt2_index].user_supervisor = 0;
		}
		pt2_base[pt2_index].write_through = 0;
		pt2_base[pt2_index].page_cache_disabled = 1;
		pt2_base[pt2_index].accessed = 0;
		pt2_base[pt2_index].zero = 0;
		pt2_base[pt2_index].base_address = ((uint64_t) pt3_base) >> 12;
	} else {
		pt3_base = (pt3_entry_t *) (((uint64_t) pt2_value.base_address) << 12);
	}

	pt3_entry_t pt3_value = pt3_base[pt3_index];
	if (pt3_value.present == 0) {
		pt4_base = (pt4_entry_t *) pfalloc();
		my_memset((uint8_t *) pt4_base, 0x00, 4096);

		pt3_base[pt3_index].present = 1;
		pt3_base[pt3_index].rw = 1;
		if (is_user_accessible){
			pt3_base[pt3_index].user_supervisor = 1;
		} else {
			pt3_base[pt3_index].user_supervisor = 0;
		}
		pt3_base[pt3_index].write_through = 0;
		pt3_base[pt3_index].page_cache_disabled = 1;
		pt3_base[pt3_index].accessed = 0;
		pt3_base[pt3_index].zero = 0;
		pt3_base[pt3_index].base_address = ((uint64_t) pt4_base) >> 12;
	} else {
		pt4_base = (pt4_entry_t *) (((uint64_t) pt3_value.base_address) << 12);
	}

	if(pt4_base[pt4_index].present == 1) {
		printk("Page Already exists, write permission error\n");
		asm volatile("hlt");
	}

	if(virtual >= (uint8_t *) 0x20000000000 && virtual < (uint8_t *) 0x60000000000){
		//User stack top at 6<<40
		//Kernel Stack for each process at 5<40
		//User Threads at 3<<40 -> 4<<40
		//Kernel Threads at 2<<40 -> 3<<40
	} else if(pt4_base[pt4_index].avl1 == 0) {
		printk("Page has not been allocated yet\n");
		asm volatile("hlt");
	}

	pt4_base[pt4_index].present = 1;
	pt4_base[pt4_index].rw = 1;
	if (is_user_accessible){
		pt4_base[pt4_index].user_supervisor = 1;
	} else {
		pt4_base[pt4_index].user_supervisor = 0;
	}
	pt4_base[pt4_index].write_through = 0;
	pt4_base[pt4_index].page_cache_disabled = 1;
	pt4_base[pt4_index].accessed = 0;
	pt4_base[pt4_index].dirty = 0;
	pt4_base[pt4_index].base_address = ((uint64_t) physical) >> 12;
}

void create_section_start_mappings() {
	uint64_t i;

	i = 1;
	i = i << 40;
	//Create kernel Heap
	//make_pt_valid((uint8_t *) i);
	//create_pt_mapping((uint8_t *) i, pfalloc());

	//Create kernel Stack
	i = i - 4096;
	make_pt_valid((uint8_t *) i);
	create_pt_mapping((uint8_t *) i, pfalloc());
}

void on_page_fault() {
	uint64_t cr2;

	asm volatile("mov %%cr2, %0" : "=r"(cr2));

	if((cr2>>12) == 0) {
		if(cr2 == 0) {
			printk("Null Pointer Error, Halting!\n");
			asm volatile("hlt");
		}
	} else {
		create_pt_mapping((uint8_t *) cr2, pfalloc());
	}
}

void load_new_page_table(pt1_entry_t *page_table_base) {
	current_page_table_base = page_table_base;
}

void my_memcpy_separate_pt(unsigned char *destination, unsigned char *source, unsigned long long len, uint8_t *page_table_base){
        unsigned long long i;
	pt1_entry_t *old_page_table_base;

        for(i = 0; i < len; i++) {
		old_page_table_base = current_page_table_base;
		load_new_page_table((pt1_entry_t *) page_table_base);
		put_value_in_separate_pt(destination + i, source[i], (pt1_entry_t *) page_table_base);
		load_new_page_table(old_page_table_base);
        }
}
