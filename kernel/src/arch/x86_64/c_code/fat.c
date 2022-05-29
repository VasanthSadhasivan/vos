#include"fat.h"

file_descriptor_t fd_table[512];

uint8_t sectors_per_cluster;
uint64_t first_data_sector;
uint16_t bytes_per_sector;
uint16_t reserved_sector_count;
uint32_t root_directory_cluster;

block_device_t *main_partition;

void FAT_init() {
	retrieve_main_partition();
	parse_main_partition();

	my_memset((uint8_t *) fd_table, 0x00, 512 * sizeof(file_descriptor_t));
}

void retrieve_main_partition() {
	for(main_partition = block_device_head; main_partition; main_partition = main_partition-> next) {
		if (main_partition -> type == PARTITION_TYPE) {
			//Hacky way to grab the first partition and assume its the main one.
			break;
		}
	}

	if (main_partition && main_partition -> type != PARTITION_TYPE) {
		printk("No main partition found. Halting.");
		asm volatile("hlt");
	}

	if ((block_device_t *) 0x00 == main_partition) {
		printk("No main partition found. Halting.");
		asm volatile("hlt");
	}
}


void parse_main_partition(){
	uint8_t block [512];
	uint8_t fat_table_count;
	uint32_t total_sector_count;
	uint32_t sectors_per_fat;
	uint16_t fsinfo_sector;
	uint8_t drive_number;
	uint32_t cluster_search_start_sector;

	main_partition -> read_block(main_partition, 0, block);
	bytes_per_sector = ((uint16_t *)(block + 0x0B))[0];
	sectors_per_cluster = ((uint8_t *)(block + 0x0D))[0];
	reserved_sector_count = ((uint16_t *)(block + 0x0E))[0];
	fat_table_count = block[0x10];

	if(((uint16_t *)(block + 0x13))[0] != 0) {
		total_sector_count = ((uint16_t *)(block + 0x13))[0];
	}else {
		total_sector_count = ((uint32_t *)(block + 0x20))[0];
	}

	sectors_per_fat = ((uint32_t *)(block + 0x24))[0];
	root_directory_cluster = ((uint32_t *)(block + 0x2C))[0];
	fsinfo_sector = ((uint16_t *)(block + 0x30))[0];
	drive_number = block[0x40];

	if (block[0x42] != 0x28 && block[0x42] != 0x29) {
		printk("Volume Boot Record at 0x42 offset is not 28 or 29.");
		return;
	}

	first_data_sector = reserved_sector_count + fat_table_count * sectors_per_fat;
	//first_sector_of_cluster(n) => ((n-2) * sectors_per_cluster) + first_data_sector
	//fat_sector_number(input_cluster_n) => reserved_sector_count + n * 4 / bytes_per_sector
		//fat_sector_offset(input_cluster_n) => (n*4) % bytes_per_sector	

	main_partition -> read_block(main_partition, fsinfo_sector, block);

	if(((uint32_t *) block)[0] != 0x41615252) {
		printk("Invalid fsinfo lead signature. Halting.");
		asm volatile("hlt");
	}

//	if ((total_sector_count - (reserved_sector_count + fat_table_count * sectors_per_fat))/sectors_per_cluster < 65525) {
//		printk("Not enough clusters for a fat32 system. Halting.");
//		asm volatile("hlt");
//	}
}

uint64_t get_first_sector_of_cluster(uint32_t n) {
	return ((n - 2) * sectors_per_cluster) + first_data_sector;
}

uint64_t get_fat_sector_number(uint32_t n) {
	return reserved_sector_count + (n * 4) / bytes_per_sector;
}

uint64_t get_fat_sector_offset(uint64_t n) {
	return (n * 4) % bytes_per_sector;
}

uint32_t fat_lookup(uint32_t cluster) {
	uint64_t sector_number;
	uint64_t sector_offset;
	uint64_t block_number;
	uint64_t block_offset;
	uint8_t block[512];
	short_entry_t *short_entry;
	long_entry_t *long_entry;

	sector_number = get_fat_sector_number(cluster);
	sector_offset = get_fat_sector_offset(cluster);

	block_number = sector_number * bytes_per_sector / 512 + sector_offset / 512;
	block_offset = sector_offset % 512;
	
	main_partition -> read_block(main_partition, block_number, block);

	return ((uint32_t *) (block + block_offset))[0];
}

void read_cluster(uint32_t cluster, uint8_t *cluster_buffer) {
	uint32_t i;
	uint64_t block_number;

	block_number = get_first_sector_of_cluster(cluster) * bytes_per_sector / 512;

	for(i = 0; i < bytes_per_sector * sectors_per_cluster / 512; i++) {
		main_partition -> read_block(main_partition, block_number + i, cluster_buffer + i * 512);
	}
}

void convert_entry_filename(short_entry_t short_entry, uint8_t *buffer) {
	uint8_t name[12];
	uint8_t i;
	uint8_t name_i;

	my_memcpy(name, short_entry.dir_name, 11);
	my_memset(name, 0x00, 12);

	name_i = 0;
	for(i = 0; i < 11; i++){
		if (short_entry.dir_name[i] == ' ' && short_entry.dir_name[i + 1] != ' ') {
			if (short_entry.dir_name[i+1]) {
				name[name_i] = '.';
				name_i += 1;

			}
		} else if (short_entry.dir_name[i] == ' '){
		} else {
			name[name_i] = short_entry.dir_name[i];
			name_i += 1;
		}
	}
	
	my_memcpy(buffer, name, 12);
}

void convert_utf16_to_utf8(uint16_t *utf16_string, uint8_t *utf8_string) {
	uint64_t i;
	
	for(i = 0; i < 256; i++) {
		utf8_string[i] = (uint8_t) utf16_string[i];
	}
}

void print_entry_name(short_entry_t short_entry) {
	uint8_t name[12];

	convert_entry_filename(short_entry, name);

	printk("Filename: %s\n", name);
}

void print_long_entry_name(uint16_t *long_file_name) {
	uint16_t *val;

	printk("Filename: ");
	for(val = long_file_name; val[0] != 0x0000; val += 1) {
		printk("%c", (uint8_t) val[0]);
	}
	printk("\n");
}

void read_entire_directory(uint32_t cluster) {
	uint8_t *cluster_buffer;
	uint32_t i;
	short_entry_t *short_entries;
	long_entry_t *long_entries;
	uint16_t long_file_name[256];
	uint8_t long_name_flag;

	my_memset((uint8_t *) long_file_name, 0x00, 256 * sizeof(uint16_t));
	cluster_buffer = kmalloc(bytes_per_sector * sectors_per_cluster);

	while(cluster <  0xffffff8) {
		read_cluster(cluster, cluster_buffer);
		short_entries = (short_entry_t *) cluster_buffer;
		long_entries = (long_entry_t *) cluster_buffer;
		for(i = 0; i < bytes_per_sector * sectors_per_cluster / sizeof(short_entry_t); i++) {
			if (short_entries[i].dir_name[0] == 0x00 || short_entries[i].dir_name[0] == 0xE5){
				continue;
			}

			if (short_entries[i].dir_name[0] == '.' && short_entries[i].dir_name[1] == ' '){
				continue;
			} else if (short_entries[i].dir_name[0] == '.' && short_entries[i].dir_name[1] == '.' && short_entries[i].dir_name[2] == ' ') {
				continue;
			}

			if (long_entries[i].ldir_attr == 0x0F) {
				long_name_flag = 0xFF;
				my_memcpy((uint8_t *) long_file_name + ((long_entries[i].ldir_ord & (~0x40)) - 1) * 26, long_entries[i].ldir_name1, 10);
				my_memcpy((uint8_t *) long_file_name + ((long_entries[i].ldir_ord & (~0x40)) - 1) * 26 + 10, long_entries[i].ldir_name2, 12);
				my_memcpy((uint8_t *) long_file_name + ((long_entries[i].ldir_ord & (~0x40)) - 1) * 26 + 22, long_entries[i].ldir_name3, 4);
			}
			
			if (!long_name_flag && long_entries[i].ldir_attr != 0x0F) {
				convert_entry_filename(short_entries[i], (uint8_t *) long_file_name);
				printk("%s\n", long_file_name);
				my_memset((uint8_t *) long_file_name, 0x00, 256 * sizeof(uint16_t));
			}

			if (long_name_flag && long_entries[i].ldir_attr != 0x0F) {
				long_name_flag = 0x00;
				print_long_entry_name(long_file_name);
				my_memset((uint8_t *) long_file_name, 0x00, 256 * sizeof(uint16_t));
			}
		
			if (short_entries[i].dir_attr == 0x10){
				read_entire_directory(short_entries[i].dir_fstcluslo + (short_entries[i].dir_fstclushi << 8));
			}
		}

		cluster = fat_lookup(cluster);
	}
}

void read_root_directory() {
	read_entire_directory(root_directory_cluster);
}

uint32_t get_size(uint8_t *filename, uint32_t cluster) {
	uint8_t *cluster_buffer;
	uint32_t i;
	short_entry_t *short_entries;
	long_entry_t *long_entries;
	uint16_t long_file_name[256];
	uint8_t long_name_flag;

	my_memset((uint8_t *) long_file_name, 0x00, 256 * sizeof(uint16_t));
	cluster_buffer = kmalloc(bytes_per_sector * sectors_per_cluster);

	while(cluster <  0xffffff8) {
		read_cluster(cluster, cluster_buffer);
		short_entries = (short_entry_t *) cluster_buffer;
		long_entries = (long_entry_t *) cluster_buffer;
		for(i = 0; i < bytes_per_sector * sectors_per_cluster / sizeof(short_entry_t); i++) {
			if (short_entries[i].dir_name[0] == 0x00 || short_entries[i].dir_name[0] == 0xE5){
				continue;
			}

			if (short_entries[i].dir_name[0] == '.' && short_entries[i].dir_name[1] == ' '){
				continue;
			} else if (short_entries[i].dir_name[0] == '.' && short_entries[i].dir_name[1] == '.' && short_entries[i].dir_name[2] == ' ') {
				continue;
			}

			if (long_entries[i].ldir_attr == 0x0F) {
				long_name_flag = 0xFF;
				my_memcpy((uint8_t *) long_file_name + ((long_entries[i].ldir_ord & (~0x40)) - 1) * 26, long_entries[i].ldir_name1, 10);
				my_memcpy((uint8_t *) long_file_name + ((long_entries[i].ldir_ord & (~0x40)) - 1) * 26 + 10, long_entries[i].ldir_name2, 12);
				my_memcpy((uint8_t *) long_file_name + ((long_entries[i].ldir_ord & (~0x40)) - 1) * 26 + 22, long_entries[i].ldir_name3, 4);
			}
			
			if (!long_name_flag && long_entries[i].ldir_attr != 0x0F) {
				convert_entry_filename(short_entries[i], (uint8_t *) long_file_name);
				if (my_memcmp((uint8_t *) long_file_name, filename, my_strlen(filename) + 1) == 0) {
					kfree(cluster_buffer);
					return short_entries[i].dir_filesize;
				}
				my_memset((uint8_t *) long_file_name, 0x00, 256 * sizeof(uint16_t));
			}

			if (long_name_flag && long_entries[i].ldir_attr != 0x0F) {
				long_name_flag = 0x00;
				convert_utf16_to_utf8(long_file_name, (uint8_t *) long_file_name);
				if (my_memcmp((uint8_t *) long_file_name, filename, my_strlen(filename) + 1) == 0) {
					kfree(cluster_buffer);
					return short_entries[i].dir_filesize;
				}
				my_memset((uint8_t *) long_file_name, 0x00, 256 * sizeof(uint16_t));
			}
		}

		cluster = fat_lookup(cluster);
	}
	
	kfree(cluster_buffer);
	return 0;
}

uint32_t get_cluster(uint8_t *filename, uint32_t cluster) {
	uint8_t *cluster_buffer;
	uint32_t i;
	short_entry_t *short_entries;
	long_entry_t *long_entries;
	uint16_t long_file_name[256];
	uint8_t long_name_flag;

	my_memset((uint8_t *) long_file_name, 0x00, 256 * sizeof(uint16_t));
	cluster_buffer = kmalloc(bytes_per_sector * sectors_per_cluster);

	while(cluster <  0xffffff8) {
		read_cluster(cluster, cluster_buffer);
		short_entries = (short_entry_t *) cluster_buffer;
		long_entries = (long_entry_t *) cluster_buffer;
		for(i = 0; i < bytes_per_sector * sectors_per_cluster / sizeof(short_entry_t); i++) {
			if (short_entries[i].dir_name[0] == 0x00 || short_entries[i].dir_name[0] == 0xE5){
				continue;
			}

			if (short_entries[i].dir_name[0] == '.' && short_entries[i].dir_name[1] == ' '){
				continue;
			} else if (short_entries[i].dir_name[0] == '.' && short_entries[i].dir_name[1] == '.' && short_entries[i].dir_name[2] == ' ') {
				continue;
			}

			if (long_entries[i].ldir_attr == 0x0F) {
				long_name_flag = 0xFF;
				my_memcpy((uint8_t *) long_file_name + ((long_entries[i].ldir_ord & (~0x40)) - 1) * 26, long_entries[i].ldir_name1, 10);
				my_memcpy((uint8_t *) long_file_name + ((long_entries[i].ldir_ord & (~0x40)) - 1) * 26 + 10, long_entries[i].ldir_name2, 12);
				my_memcpy((uint8_t *) long_file_name + ((long_entries[i].ldir_ord & (~0x40)) - 1) * 26 + 22, long_entries[i].ldir_name3, 4);
			}
			
			if (!long_name_flag && long_entries[i].ldir_attr != 0x0F) {
				convert_entry_filename(short_entries[i], (uint8_t *) long_file_name);
				if (my_memcmp((uint8_t *) long_file_name, filename, my_strlen(filename) + 1) == 0) {
					kfree(cluster_buffer);
					return short_entries[i].dir_fstcluslo | short_entries[i].dir_fstclushi << 16;
				}
				my_memset((uint8_t *) long_file_name, 0x00, 256 * sizeof(uint16_t));
			}

			if (long_name_flag && long_entries[i].ldir_attr != 0x0F) {
				long_name_flag = 0x00;
				convert_utf16_to_utf8(long_file_name, (uint8_t *) long_file_name);
				if (my_memcmp((uint8_t *) long_file_name, filename, my_strlen(filename) + 1) == 0) {
					kfree(cluster_buffer);
					return short_entries[i].dir_fstcluslo | short_entries[i].dir_fstclushi << 16;
				}
				my_memset((uint8_t *) long_file_name, 0x00, 256 * sizeof(uint16_t));
			}
		}

		cluster = fat_lookup(cluster);
	}
	
	kfree(cluster_buffer);
	return cluster;
}

uint16_t get_available_fd(){
	uint16_t i;

	for(i = 0; i < 512; i++) {
		if (fd_table[i].cluster == 0x00) {
			return i;
		}
	}

	return 512;
}

uint16_t open(uint8_t *filename) {
	char *token;
	uint32_t prev_cluster = root_directory_cluster;
	uint16_t fd_table_i;
	uint32_t file_size;

	token = strtok(filename, "/");	
	do{
		if (prev_cluster >= 0xffffff8) {
			printk("File could not be opened\n");
			return 512;
		}
		file_size = get_size(token, prev_cluster);
		prev_cluster = get_cluster(token, prev_cluster);
	} while(token = strtok(0x00, "/"));

	fd_table_i = get_available_fd();
	
	if (fd_table_i >= 512) {
		printk("No open File Descriptors\n");
		return 512;
	}

	fd_table[fd_table_i].cluster = prev_cluster;
	fd_table[fd_table_i].start_cluster = prev_cluster;
	fd_table[fd_table_i].cluster_offset = 0;
	fd_table[fd_table_i].file_size = file_size;

	return fd_table_i;
}

uint16_t close(uint16_t fd) {
	if(fd >= 512) {
		printk("FD out of bounds\n");
	}

	fd_table[fd].cluster = 0;
	fd_table[fd].start_cluster = 0;
	fd_table[fd].cluster_offset = 0;
	fd_table[fd].file_size = 0;
}

uint64_t lseek(uint8_t fd, uint64_t size, uint8_t whence) {
	uint8_t i;
	uint32_t cluster_count;
	uint32_t cluster;
	uint32_t cluster_offset;
	uint32_t cluster_write_offset;
	uint64_t read_count;
	uint64_t temp;

	if(fd >= 512) {
		printk("FD out of bounds\n");
		return 0;
	}

	if(fd_table[fd].cluster == 0) {
		printk("FD is not open\n");
		return 0;
	}

	if(whence == SEEK_SET) {
		fd_table[fd].cluster = fd_table[fd].start_cluster;
		fd_table[fd].cluster_offset = 0;
		fd_table[fd].file_offset = 0;
	}

	if (fd_table[fd].file_offset + size > fd_table[fd].file_size) {
		size = fd_table[fd].file_size - fd_table[fd].file_offset;
	}

	read_count = 0;
	cluster_count = (size + bytes_per_sector * sectors_per_cluster - 1) / (bytes_per_sector * sectors_per_cluster);
	cluster = fd_table[fd].cluster;
	cluster_offset = fd_table[fd].cluster_offset;
	cluster_write_offset = (bytes_per_sector * sectors_per_cluster - cluster_offset) % bytes_per_sector * sectors_per_cluster;

	if (cluster_offset != 0) {
		if(cluster >= 0xffffff8) {
			printk("File fully read\n");
                        return 0;
		}

		if (size < (bytes_per_sector * sectors_per_cluster - cluster_offset)) {
			fd_table[fd].cluster_offset += size;
			fd_table[fd].file_offset += size;
			return size;
		} else {
			read_count += bytes_per_sector * sectors_per_cluster - cluster_offset;
			size -= bytes_per_sector * sectors_per_cluster - cluster_offset;
			cluster_count = (size + bytes_per_sector * sectors_per_cluster - 1) / (bytes_per_sector * sectors_per_cluster);
			fd_table[fd].cluster = fat_lookup(cluster);
			cluster = fd_table[fd].cluster;
			fd_table[fd].cluster_offset = 0;
			cluster_offset = fd_table[fd].cluster_offset;
		}
	}

	for(i = 0; i < cluster_count; i++) {
		if(cluster >= 0xffffff8) {
			printk("File fully read\n");
			return 0;
		}

		if (i == cluster_count - 1) {
			temp = size % (bytes_per_sector * sectors_per_cluster);
			if (temp == 0) {
				temp = bytes_per_sector * sectors_per_cluster;
				cluster = fat_lookup(cluster);
				cluster_offset = 0;
			} else {
				cluster_offset = temp;
			}

			read_count += temp;
		} else {

			read_count += bytes_per_sector * sectors_per_cluster;
			cluster = fat_lookup(cluster);
		}
	}

	fd_table[fd].cluster = cluster;
	fd_table[fd].cluster_offset = cluster_offset;
	fd_table[fd].file_offset += read_count;

	return read_count;
}

uint64_t read(uint8_t fd, uint64_t size, uint8_t *buffer) {
	uint8_t i;
	uint8_t *cluster_buffer;
	uint32_t cluster_count;
	uint32_t cluster;
	uint32_t cluster_offset;
	uint32_t cluster_write_offset;
	uint64_t read_count;
	uint64_t temp;

	if (size == 0) {
		return 0;
	}
	
	if(fd >= 512) {
		printk("FD out of bounds\n");
		return 0;
	}

	if(fd_table[fd].cluster == 0) {
		printk("FD is not open\n");
		return 0;
	}

	if (fd_table[fd].file_offset + size > fd_table[fd].file_size) {
		size = fd_table[fd].file_size - fd_table[fd].file_offset;
	}

	read_count = 0;
	cluster_buffer = kmalloc(bytes_per_sector * sectors_per_cluster);
	cluster_count = (size + bytes_per_sector * sectors_per_cluster - 1) / (bytes_per_sector * sectors_per_cluster);
	cluster = fd_table[fd].cluster;
	cluster_offset = fd_table[fd].cluster_offset;
	cluster_write_offset = (bytes_per_sector * sectors_per_cluster - cluster_offset) % bytes_per_sector * sectors_per_cluster;

	if (cluster_offset != 0) {
		if(cluster >= 0xffffff8) {
			printk("File fully read\n");
                        kfree(cluster_buffer);
                        return 0;
		}

		read_cluster(cluster, cluster_buffer);
		if (size < (bytes_per_sector * sectors_per_cluster - cluster_offset)) {
			my_memcpy(buffer, cluster_buffer + cluster_offset, size);
			fd_table[fd].cluster_offset += size;
			kfree(cluster_buffer);
			fd_table[fd].file_offset += size;
			return size;
		} else {
			my_memcpy(buffer, cluster_buffer + cluster_offset, bytes_per_sector * sectors_per_cluster - cluster_offset);
			read_count += bytes_per_sector * sectors_per_cluster - cluster_offset;
			size -= bytes_per_sector * sectors_per_cluster - cluster_offset;
			cluster_count = (size + bytes_per_sector * sectors_per_cluster - 1) / (bytes_per_sector * sectors_per_cluster);
			fd_table[fd].cluster = fat_lookup(cluster);
			cluster = fd_table[fd].cluster;
			fd_table[fd].cluster_offset = 0;
			cluster_offset = fd_table[fd].cluster_offset;
		}
	}

	for(i = 0; i < cluster_count; i++) {
		if(cluster >= 0xffffff8) {
			printk("File fully read\n");
			kfree(cluster_buffer);
			return 0;
		}

		read_cluster(cluster, cluster_buffer);
		if (i == cluster_count - 1) {
			temp = size % (bytes_per_sector * sectors_per_cluster);
			if (temp == 0) {
				temp = bytes_per_sector * sectors_per_cluster;
				cluster = fat_lookup(cluster);
				cluster_offset = 0;
				my_memcpy(buffer + i * (bytes_per_sector * sectors_per_cluster) + cluster_write_offset, cluster_buffer, bytes_per_sector * sectors_per_cluster);
			} else {
				cluster_offset = temp;
				my_memcpy(buffer + i * (bytes_per_sector * sectors_per_cluster) + cluster_write_offset, cluster_buffer, cluster_offset);
			}

			read_count += temp;
		} else {

			my_memcpy(buffer + i * (bytes_per_sector * sectors_per_cluster) + cluster_write_offset, cluster_buffer, bytes_per_sector * sectors_per_cluster);
			read_count += bytes_per_sector * sectors_per_cluster;
			cluster = fat_lookup(cluster);
		}
	}

	fd_table[fd].cluster = cluster;
	fd_table[fd].cluster_offset = cluster_offset;
	fd_table[fd].file_offset += read_count;

	kfree(cluster_buffer);
	return read_count;
}
