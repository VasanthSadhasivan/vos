#include"block.h"

void BLK_init() {
	block_device_head = (block_device_t *) 0x00;
}

void BLK_register(block_device_t *dev) {
	//add to linked list
	dev -> next = block_device_head;
	block_device_head = dev;
}
