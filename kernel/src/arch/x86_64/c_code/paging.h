#ifndef PAGING_H__
#define PAGING_H__

#include"framing.h"
#include"interrupt.h"

typedef struct __attribute__((packed)){
	uint64_t present : 1;
	uint64_t rw : 1;
	uint64_t user_supervisor : 1;
	uint64_t write_through : 1;
	uint64_t page_cache_disabled : 1;
	uint64_t accessed : 1;
	uint64_t ignore : 1;
	uint64_t zero : 2;
	uint64_t avl1 : 3;
	uint64_t base_address : 40;
	uint64_t avl2 : 11;
	uint64_t no_execute : 1;
} pt1_entry_t;

typedef struct __attribute__((packed)){
	uint64_t present : 1;
	uint64_t rw : 1;
	uint64_t user_supervisor : 1;
	uint64_t write_through : 1;
	uint64_t page_cache_disabled : 1;
	uint64_t accessed : 1;
	uint64_t ignore1 : 1;
	uint64_t zero : 1;
	uint64_t ignore2 : 1;
	uint64_t avl1 : 3;
	uint64_t base_address : 40;
	uint64_t avl2 : 11;
	uint64_t no_execute : 1;
} pt2_entry_t;

typedef struct __attribute__((packed)){
	uint64_t present : 1;
	uint64_t rw : 1;
	uint64_t user_supervisor : 1;
	uint64_t write_through : 1;
	uint64_t page_cache_disabled : 1;
	uint64_t accessed : 1;
	uint64_t ignore1 : 1;
	uint64_t zero : 1;
	uint64_t ignore2 : 1;
	uint64_t avl1 : 3;
	uint64_t base_address : 40;
	uint64_t avl2 : 11;
	uint64_t no_execute : 1;
} pt3_entry_t;

typedef struct __attribute__((packed)){
	uint64_t present : 1;
	uint64_t rw : 1;
	uint64_t user_supervisor : 1;
	uint64_t write_through : 1;
	uint64_t page_cache_disabled : 1;
	uint64_t accessed : 1;
	uint64_t dirty : 1;
	uint64_t pat : 1;
	uint64_t global : 1;
	uint64_t avl1 : 3;
	uint64_t base_address : 40;
	uint64_t avl2 : 11;
	uint64_t no_execute : 1;
} pt4_entry_t;

void setup_user_paging(pt1_entry_t *user_page_table_base);
void setup_paging();
void create_identity_map();
void create_pt_mapping(uint8_t *virtual, uint8_t *physical);
void create_section_start_mappings();
void on_page_fault();
void make_pt_valid(uint8_t *virtual);
void load_new_page_table(pt1_entry_t *page_table_base);
void my_memcpy_separate_pt(unsigned char *destination, unsigned char *source, unsigned long long len, uint8_t *page_table_base);

extern void virtualization_start(uint64_t cr3);
extern void put_value_in_separate_pt(uint8_t *destination, uint8_t value, pt1_entry_t *page_table_base);

pt1_entry_t *kernel_page_table_base;
pt1_entry_t *current_page_table_base;

#endif
