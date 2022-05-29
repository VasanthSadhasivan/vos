#ifndef FAT_H__
#define FAT_H__

#include<stdint.h>
#include"string.h"
#include"block.h"
#include"heap.h"
#include"my_stdlib.h"

typedef struct __attribute__((packed)){
	uint8_t dir_name[11];
	uint8_t dir_attr;
	uint8_t dir_ntres;
	uint8_t dir_crttimetenth;
	uint16_t dir_crttime;
	uint16_t dir_crtdate;
	uint16_t dir_lstaccdate;
	uint16_t dir_fstclushi;
	uint16_t dir_wrttime;
	uint16_t dir_wrtdate;
	uint16_t dir_fstcluslo;
	uint32_t dir_filesize;
} short_entry_t;

typedef struct __attribute__((packed)){
	uint8_t ldir_ord;
	uint8_t ldir_name1[10];
	uint8_t ldir_attr;
	uint8_t ldir_type;
	uint8_t ldir_chksum;
	uint8_t ldir_name2[12];
	uint16_t ldir_fstcluslo;
	uint8_t ldir_name3[4];
} long_entry_t;

typedef struct {
	uint32_t start_cluster;
	uint32_t cluster;
	uint32_t cluster_offset;
	uint32_t file_offset;
	uint32_t file_size;
} file_descriptor_t;

void FAT_init();
void retrieve_main_partition();
void parse_main_partition();
void read_cluster(uint32_t cluster, uint8_t *cluster_buffer);
void convert_entry_filename(short_entry_t short_entry, uint8_t *buffer);
void convert_utf16_to_utf8(uint16_t *utf16_string, uint8_t *utf8_string);
void print_entry_name(short_entry_t short_entry);
void print_long_entry_name(uint16_t *long_file_name);
void read_entire_directory(uint32_t cluster);
void read_root_directory();
uint32_t get_size(uint8_t *filename, uint32_t cluster);
uint32_t get_cluster(uint8_t *filename, uint32_t cluster);
uint64_t get_first_sector_of_cluster(uint32_t n);
uint64_t get_fat_sector_number(uint32_t n);
uint64_t get_fat_sector_offset(uint64_t n);
uint32_t fat_lookup(uint32_t cluster);
uint16_t get_available_fd();
uint16_t open(uint8_t *filename);
uint16_t close(uint16_t fd);
uint64_t read(uint8_t fd, uint64_t size, uint8_t *buffer);
uint64_t lseek(uint8_t fd, uint64_t size, uint8_t whence);

#define SEEK_SET 0
#define SEEK_CUR 1


#endif
