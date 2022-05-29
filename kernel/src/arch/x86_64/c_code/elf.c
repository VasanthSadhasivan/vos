#include"elf.h"

kthread_function_t *load_elf(uint8_t *file_location, uint8_t *address) {
	uint64_t e_shoff;
	kthread_function_t *entry;
	uint16_t e_shnum;
	uint16_t e_shentsize;	
	
	uint8_t *shentry;
	uint16_t fd;
	uint16_t i;

	uint8_t *section_virtual_address;
	uint64_t section_file_offset;
	uint64_t section_size;

	fd = open(file_location);

	lseek(fd, 0x28, SEEK_SET);
	read(fd, 8, (uint8_t *) &e_shoff);
	lseek(fd, 0x18, SEEK_SET);
	read(fd, 8, (uint8_t *) &entry);
	lseek(fd, 0x3C, SEEK_SET);
	read(fd, 2, (uint8_t *) &e_shnum);
	lseek(fd, 0x3A, SEEK_SET);
	read(fd, 2, (uint8_t *) &e_shentsize);

	shentry = kmalloc(e_shentsize);
	for(i = 0; i < e_shnum; i++) {
		lseek(fd, e_shoff + i * e_shentsize, SEEK_SET);
		read(fd, e_shentsize, shentry);
		if(((uint32_t *) (shentry + 0x04))[0] == 0) {
			continue;
		}

		section_virtual_address = (uint8_t *) ((uint64_t *) (shentry + 0x10))[0];
		
		if(section_virtual_address == 0) {
			continue;
		}

		section_file_offset = ((uint64_t *) (shentry + 0x18))[0];
		section_size = ((uint64_t *) (shentry + 0x20))[0];
		lseek(fd, section_file_offset, SEEK_SET);
		read(fd, section_size, section_virtual_address);
	}

	close(fd);
	kfree(shentry);

	return entry;
}
