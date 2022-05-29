#include"paging.h"

block_t kernel[256];
uint8_t kernel_len;
block_t usable[256];
uint8_t usable_len;
uint8_t *head;

void setup_frame_allocator(multiboot_info_header_t *multiboot_info_header) {
	parse_multiboot(multiboot_info_header);
	start_frame_link_list();
	isr_set_handler(255, on_page_replacement);
}

void parse_multiboot(multiboot_info_header_t *multiboot_info_header) {
	generic_tag_header_t *tag;

	for(	tag = (generic_tag_header_t *) (((uint8_t *) multiboot_info_header) + 8);
		tag < (generic_tag_header_t *) (((uint8_t *) multiboot_info_header) + (multiboot_info_header -> total_size));
		tag = (generic_tag_header_t *) (((uint8_t *)tag) + next_8th(tag -> size))) {
		
		if (tag -> type == 6) {
			parse_memory_map((uint8_t *) tag);
		} else if (tag -> type == 9) {
			parse_elf((uint8_t *) tag);
		}
	}

}

void parse_memory_map(uint8_t *mmap_tag) {
	mmap_tag_header_t *tag_header_p = (mmap_tag_header_t *) mmap_tag;
	mmap_entry_header_t *entry_p;

	usable_len = 0;

	for(	entry_p = (mmap_entry_header_t *) (((uint8_t *)tag_header_p) + 16);
		entry_p < (mmap_entry_header_t *) ((uint8_t *) mmap_tag + (tag_header_p -> size));
		entry_p =  (mmap_entry_header_t *) ((uint8_t *) entry_p + (tag_header_p -> entry_size))) {
		
		if(entry_p -> type == 1) {
			printk("Available memory block of %p -> %p\n", entry_p -> base_address, entry_p -> base_address + entry_p -> length);
			usable[usable_len].start = (uint8_t *) (entry_p -> base_address);
			usable[usable_len].end = (uint8_t *) (entry_p -> base_address + entry_p -> length);
			usable_len += 1;
		}else{

			//printk("UnAvailable memory block of %p -> %p\n", entry_p -> base_address, entry_p -> base_address + entry_p -> length);
		}
	}
}

void parse_elf(uint8_t *elf_tag) {
	elf_tag_header_t *tag_header_p = (elf_tag_header_t *) elf_tag;
	elf_section_header_t * elf_section_header_p;

	kernel_len = 0;

	for(	elf_section_header_p = (elf_section_header_t *) (((uint8_t *) tag_header_p) + sizeof(elf_tag_header_t));
		elf_section_header_p < (elf_section_header_t *) (elf_tag + tag_header_p -> size);
		elf_section_header_p = (elf_section_header_t *) (((uint8_t *) elf_section_header_p) + tag_header_p -> entsize)){
		if (elf_section_header_p -> sh_type != 0) {
			printk("Kernel is loaded in %p -> %p\n", elf_section_header_p -> sh_addr, elf_section_header_p -> sh_addr + elf_section_header_p -> sh_size);
			kernel[kernel_len].start = (uint8_t *) (elf_section_header_p -> sh_addr);
			kernel[kernel_len].end = (uint8_t *) (elf_section_header_p -> sh_addr + elf_section_header_p -> sh_size);
			kernel_len += 1;
		}
	}
}

uint32_t next_8th(uint32_t x) {
	return x + ((8 - (x % 8))%8);
}

void start_frame_link_list() {
	uint8_t *next;
	uint8_t *current;

	head = get_next_valid(0);
	current = head;

	while (1) {
		next = get_next_valid(current);
		if (((uint8_t *) -1)  == next) {
			add_link(current, (uint8_t *) -1);
			return;
		}
		add_link(current, next);
		current = next;
	}
}

uint8_t *get_next_valid(uint8_t *location) {
	location = get_next_frame(location + 4096);

	while(!is_out_of_bounds(location)) {
		if (is_usable(location)) {
			if(is_kernel(location)) {
				location = past_kernel_location(location);
			} else {
				return location;
			}
		} else {
			location = next_usable_location(location);
		}
	}

	return (uint8_t *) -1;
}

uint8_t is_out_of_bounds(uint8_t *location) {
	if (location >= max_ram()) {
		return 0xFF;
	}

	return 0x00;
}

uint8_t *max_ram() {
	uint8_t i, *last_allowable;

	last_allowable = (uint8_t *) 0;
	
	for(i = 0; i < usable_len; i++) {
		if(usable[i].end > last_allowable) {
			last_allowable = usable[i].end;
		}
	}

	return last_allowable;
}

uint8_t is_usable(uint8_t *location) {
	uint8_t i;

	for(i = 0; i < usable_len; i++) {
		if (location >= usable[i].start && (location + 4095) < usable[i].end) {
			return 0xFF;
		}
	}

	return 0x00;
}

uint8_t is_kernel(uint8_t *location) {
	uint8_t i;

	for(i = 0; i < kernel_len; i++) {
		if ((location + 4095) >= kernel[i].start && location < kernel[i].end) {
			return 0xFF;
		}
	}

	return 0x00;
}

uint8_t *next_usable_location(uint8_t *location) {
	uint8_t i;
	uint8_t closest_block_index;
	uint64_t closest_block_dist;

	location = location + 1;
	
	while(1) {
		closest_block_index = 0x00;
		closest_block_dist =  -1;

		for(i = 0; i < usable_len; i++) {
			if (location >= usable[i].start && (location +4095) < usable[i].end) {
				return get_next_frame(location);
			} else {
				if(usable[i].start - location < closest_block_dist) {
					closest_block_dist = usable[i].start - location;
					closest_block_index = i;
				}
			}
		}

		if(i==0 && usable[0].start <  (uint8_t *) closest_block_dist) {
			return (uint8_t *) -1;
		}
		
		location = usable[closest_block_index].start;
	}
	
	return (uint8_t *) -1;
}

uint8_t *past_kernel_location(uint8_t *location) {
	uint8_t i;

	location = location + 1;

	for(i = 0; i < kernel_len; i++) {
		if((location + 4095) >= kernel[i].start && location < kernel[i].end) {
			return get_next_frame(kernel[i].end);
		}
	}

	return (uint8_t *) -1;
}

uint8_t *get_next_frame(uint8_t *addr) {
	return addr + ((4096 - (((uint64_t)addr) % 4096)) % 4096);
}

void add_link(uint8_t *current, uint8_t *next) {
	((uint8_t **) current)[0] = next;
}

uint8_t *pfalloc() {
	uint8_t *old_head;
	
	old_head = head;

	if(head == ((uint8_t *) -1)) {
		asm volatile("int %0" : : "n"(0xFF));
		return (uint8_t *) -1;
	}

	head = ((uint8_t **) head)[0];

	add_link(old_head, (uint8_t *) -1);
	return old_head;
}

void add_to_tail(uint8_t *pf) {
	uint8_t *curr;
       
	curr = head;

	while(((uint8_t **)curr)[0] != ((uint8_t *) -1)){
		curr = ((uint8_t **)curr)[0];
	}
	add_link(pf, (uint8_t *) -1);
	add_link(curr, pf);
}

void pffree(uint8_t *pf) {
	if(((uint64_t)pf) % 4096 != 0) {
		return;
	}
	
	add_to_tail(pf);
	//add_link(pf, head);
	//head = pf;
}

void on_page_replacement() {
	printk("Enered Page Replacement Interrupt. Has not yet been implemented\n");
}
