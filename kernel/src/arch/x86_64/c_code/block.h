#ifndef BLOCK_H__
#define BLOCK_H__

#include"my_stdio.h"

#define ATA_TYPE 0x00
#define PARTITION_TYPE 0x01

typedef struct block_device_t_s {
	uint8_t type;	
	uint64_t (*read_block) (struct block_device_t_s *self, uint64_t block_number, uint8_t *destination);
	uint64_t (*write_block) (struct block_device_t_s *self, uint64_t block_number, uint8_t *source);
	uint64_t block_size;
	uint64_t total_length;
	struct block_device_t_s *next;
} block_device_t;


void BLK_init();
void BLK_register(block_device_t *dev);

block_device_t *block_device_head;

#endif
