#ifndef HEAP_H__
#define HEAP_H__

#include<stdint.h>
#include"my_stdlib.h"
#include"paging.h"

typedef struct allocated_block_header_s_t {
	uint64_t size;
} __attribute__((packed)) allocated_block_header_t;

typedef struct free_linked_list_entry_s_t {
	struct free_linked_list_entry_s_t *next;
	allocated_block_header_t *addr;
} __attribute__((packed)) free_linked_list_entry_t;

void kmem_init();
uint8_t *kmalloc(uint64_t size);
void kfree(uint8_t *ptr);
free_linked_list_entry_t *create_new_entry(allocated_block_header_t *addr, free_linked_list_entry_t *next);
void grow(uint16_t power_of_2);
uint8_t log_2_roof(uint64_t i);
uint64_t div_4096(uint64_t value);

#endif
