#ifndef FRAMING_H__
#define FRAMING_H__

#include<stdint.h>
#include"my_stdio.h"
#include"my_stdlib.h"
#include"interrupt.h"

typedef struct __attribute__((packed)) {
	uint32_t total_size;
	uint32_t reserved;
} multiboot_info_header_t;

typedef struct __attribute__((packed)) {
	uint32_t type;
	uint32_t size;
} generic_tag_header_t;

typedef struct __attribute__((packed)) {
	uint32_t type;
	uint32_t size;
	uint32_t entry_size;
	uint32_t entry_version;
} mmap_tag_header_t;

typedef struct __attribute__((packed)) {
	uint64_t base_address;
	uint64_t length;
	uint32_t type;
	uint32_t reserved;
} mmap_entry_header_t;

typedef struct __attribute__((packed)) {
	uint32_t type;
	uint32_t size;
	uint32_t num;
	uint32_t entsize;
	uint32_t shndx;
} elf_tag_header_t;

typedef struct __attribute__((packed)) {
	uint32_t sh_name;
	uint32_t sh_type;
	uint64_t sh_flags;
	uint64_t sh_addr;
	uint64_t sh_offset;
	uint64_t sh_size;
} elf_section_header_t;

typedef struct __attribute__((packed)) {
	uint8_t *start;
	uint8_t *end;
} block_t;

void setup_frame_allocator(multiboot_info_header_t *multiboot_info_header);
void parse_multiboot(multiboot_info_header_t *multiboot_info_header);
void parse_memory_map(uint8_t *mmap_tag);
void parse_elf(uint8_t *elf_tag);
uint32_t next_8th(uint32_t x);
void start_frame_link_list();
uint8_t *get_next_valid(uint8_t *current);
uint8_t *get_next_frame(uint8_t *addr);
uint8_t is_out_of_bounds(uint8_t *location);
uint8_t *max_ram();
uint8_t is_usable(uint8_t *location);
uint8_t is_kernel(uint8_t *location);
uint8_t *past_kernel_location(uint8_t *location);
uint8_t *next_usable_location(uint8_t *location);
void add_link(uint8_t *current, uint8_t *next);
uint8_t *pfalloc();
void add_to_tail(uint8_t *pf);
void pffree(uint8_t *);
void on_page_replacement();

#endif
