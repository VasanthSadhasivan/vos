#include"ata.h"

uint16_t PRIMARY_BASE;
uint16_t PRIMARY_CTRL;
uint16_t SECONDARY_BASE;
uint16_t SECONDARY_CTRL;

void setup_ata_devices() {
	BLK_init();
	enumerate_pci();
	register_ata_drives();
	probe_ata_drives();	
	
	isr_set_handler(0x2E, on_primary_ata);
	isr_set_handler(0x2F, on_secondary_ata);
	unmask_interrupt(0x2E);
	unmask_interrupt(0x2F);


}

void enumerate_pci() {
	uint8_t i;
	uint8_t device_number;
	uint16_t bus_number;
	uint8_t register_offset;
	uint32_t pci_register;
	uint32_t functions;
	
	for(bus_number = 0; bus_number < 256; bus_number ++) {
		for(device_number = 0; device_number < 32; device_number ++) {
			printk("Grabbing data for Bus: %d, Device: %d\n", bus_number, device_number);

			register_offset = 0x0C;
			outl(device_number << 11 | bus_number << 16 | 1 << 31 | register_offset, 0xCF8);
			pci_register = inl(0xCFC);
			functions = (pci_register >> 16) & 0x80 ? 8 : 1;
			for(i = 0; i < functions; i++) {
				register_offset = 0x08;
				outl(device_number << 11 | bus_number << 16 | 1 << 31 | register_offset | i << 8, 0xCF8);
				pci_register = inl(0xCFC);
				if((pci_register>>24) == 0x01 && ((pci_register >> 16) & 0xFF) == 0x01){ 
					printk("Grabbed ATA devicei\n");
					register_offset = 0x10;
					outl(device_number << 11 | bus_number << 16 | 1 << 31 | register_offset | i << 8, 0xCF8);
					pci_register = inl(0xCFC);
					printk("PRIMARY BASE: %x\n", (pci_register & 0xFFFFFFFC) + 0x1F0 * (!pci_register));
					PRIMARY_BASE = (pci_register & 0xFFFFFFFC) + 0x1F0 * (!pci_register);
					register_offset = 0x14;
					outl(device_number << 11 | bus_number << 16 | 1 << 31 | register_offset | i << 8, 0xCF8);
					pci_register = inl(0xCFC);
					printk("PRIMARY CTRL: %x\n", (pci_register & 0xFFFFFFFC) + 0x3F6 * (!pci_register));
					PRIMARY_CTRL = (pci_register & 0xFFFFFFFC) + 0x3F6 * (!pci_register);
					register_offset = 0x18;
					outl(device_number << 11 | bus_number << 16 | 1 << 31 | register_offset | i << 8, 0xCF8);
					pci_register = inl(0xCFC);
					printk("SECONDARY BASE: %x\n", (pci_register & 0xFFFFFFFC) + 0x170 * (!pci_register));
					SECONDARY_BASE = (pci_register & 0xFFFFFFFC) + 0x170 * (!pci_register);
					register_offset = 0x1C;
					outl(device_number << 11 | bus_number << 16 | 1 << 31 | register_offset | i << 8, 0xCF8);
					pci_register = inl(0xCFC);
					printk("SECONDARY CTRL: %x\n", (pci_register & 0xFFFFFFFC) + 0x376 * (!pci_register));
					SECONDARY_CTRL = (pci_register & 0xFFFFFFFC) + 0x376 * (!pci_register);
					return;
				}
			}
		}
	}
}

void register_ata_drives() {
	register_ata_device_in_bus(PRIMARY_BASE, PRIMARY_CTRL);
	register_ata_device_in_bus(SECONDARY_BASE, SECONDARY_CTRL);
}

uint64_t get_drive_size(uint16_t bus_port, uint16_t control_port, uint8_t is_slave) {
	uint64_t max_sectors = 0;

	if (is_slave) {
		outb(0xB0, bus_port + 6);
	} else {
		outb(0xA0, bus_port + 6);
	}

	outb(0x27, bus_port + 7);
	
	max_sectors |= (uint64_t) inb(bus_port + 3) << 0;
	max_sectors |= (uint64_t) inb(bus_port + 4) << 16;
	max_sectors |= (uint64_t) inb(bus_port + 5) << 32;

	outb(0x80, control_port);

	max_sectors |= (uint64_t) inb(bus_port + 3) << (0 + 8);
	max_sectors |= (uint64_t) inb(bus_port + 4) << (16 + 8);
	max_sectors |= (uint64_t) inb(bus_port + 5) << (32 + 8);

	outb(0x00, control_port);

	return max_sectors;
}

void register_ata_device_in_bus(uint16_t bus_port, uint16_t control_port) {
	uint8_t status;
	uint16_t i;

	ata_device_t *master_ata_device = (ata_device_t *) kmalloc(sizeof(ata_device_t)); 
	ata_device_t *slave_ata_device = (ata_device_t *) kmalloc(sizeof(ata_device_t)); 
	
	master_ata_device -> parent.type = ATA_TYPE;
	slave_ata_device -> parent.type = ATA_TYPE;

	// bus master
	outb(0xA0, bus_port + 6);
	outw(0x00, bus_port + 3);
	outw(0x00, bus_port + 4);
	outw(0x00, bus_port + 5);
	outb(0xEC, bus_port + 7);
	if (0x00 == (status = inb(bus_port + 7))){ 
		// device dne
	} else {
		while((status = inb(bus_port + 7)) & 0x80);
		if (inb(bus_port + 4)) {
			// not ata
		} else if(inb(bus_port + 5)) {
			// not ata
		} else {
			status = inb(bus_port + 7);
			while((status & 0x08) == 0 && (status & 0x01) == 0){
				status = inb(bus_port + 7);
			}
			
			if(status & 0x01) {
				// not ata
			} else {
				// ata
				master_ata_device -> bus_port = bus_port;
				master_ata_device -> control_port = control_port;
				for(i=0; i < 256; i++){
					inw(master_ata_device -> bus_port);
				}
				master_ata_device -> is_slave = 0;
				master_ata_device -> parent.read_block = &read_ata_drive;
				master_ata_device -> parent.write_block = &write_ata_drive;
				master_ata_device -> parent.block_size = 512;
				master_ata_device -> parent.total_length = get_drive_size(bus_port, control_port, 0);
				if(master_ata_device -> parent.total_length == 0) {
					kfree((uint8_t *) master_ata_device);
				} else {
					BLK_register((block_device_t *) master_ata_device);
				}
			}
		}
	}

	// bus slave
	outb(0xB0, bus_port + 6);
	outw(0x00, bus_port + 3);
	outw(0x00, bus_port + 4);
	outw(0x00, bus_port + 5);
	outb(0xEC, bus_port + 7);
	if (0x00 == (status = inb(bus_port + 7))){ 
		// device dne
	} else {
		while((status = inb(bus_port + 7)) & 0x80);
		if (inb(bus_port + 4)) {
			// not ata
		} else if(inb(bus_port + 5)) {
			// not ata
		} else {
			status = inb(bus_port + 7);
			while((status & 0x08) == 0 && (status & 0x01) == 0){
				status = inb(bus_port + 7);
			}
			
			if(status & 0x01) {
				// not ata
			} else {
				// ata
				// register device
				slave_ata_device -> bus_port = bus_port;
				slave_ata_device -> control_port = control_port;
				for(i=0; i < 256; i++){
					inw(slave_ata_device -> bus_port);
				}
				slave_ata_device -> is_slave = 1;
				slave_ata_device -> parent.read_block = &read_ata_drive;
				slave_ata_device -> parent.write_block = &write_ata_drive;
				slave_ata_device -> parent.block_size = 512;
				slave_ata_device -> parent.total_length = get_drive_size(bus_port, control_port, 1);

				if(slave_ata_device -> parent.total_length == 0) {
					kfree((uint8_t *) slave_ata_device);
				} else {
					BLK_register((block_device_t *) slave_ata_device);
				}
			}
		}
	}
}

void ns_delay(uint16_t bus_port) {
	uint8_t i;

	for(i = 0; i < 15; i++) {
		inb(bus_port + 7);
	}
}

uint64_t send_ata_command(uint16_t bus_port, uint8_t is_slave, uint64_t lba, uint8_t command_byte) {
	if(is_slave) {
		outb(0xB0, bus_port + 6);
	}else {
		outb(0xA0, bus_port + 6);
	}

	outb(1 << 5 | 1 << 7 | (is_slave) << 4 | 1 << 6, bus_port + 6);
	ns_delay(bus_port);
	outb(0, bus_port + 2);
	outb(0xFF & (lba >> (3 * 8)), bus_port + 3);
	outb(0xFF & (lba >> (4 * 8)), bus_port + 4);
	outb(0xFF & (lba >> (5 * 8)), bus_port + 5);
	outb(1, bus_port + 2);
	outb(0xFF & (lba >> (0 * 8)), bus_port + 3);
	outb(0xFF & (lba >> (1 * 8)), bus_port + 4);
	outb(0xFF & (lba >> (2 * 8)), bus_port + 5);
	outb(command_byte, bus_port + 7);
}

uint64_t read_ata_drive(block_device_t *self, uint64_t lba, uint8_t *dest) {
	uint8_t local_interrupt_flag = 0x00;

	CLI_IF;

	ata_device_t *ata_device = (ata_device_t *) self;
	
	// PROC block on ata device
	if(ata_device -> bus_port == PRIMARY_BASE) {
		if(PROC_primary_ata_queue.write_i == PROC_primary_ata_queue.read_i){
			send_ata_command(ata_device -> bus_port, ata_device -> is_slave, lba, 0x24);
		}

		((command_t *) PROC_primary_ata_queue.buffer)[PROC_primary_ata_queue.write_i].command_byte = 0x24;
		((command_t *) PROC_primary_ata_queue.buffer)[PROC_primary_ata_queue.write_i].pointer = dest;
		((command_t *) PROC_primary_ata_queue.buffer)[PROC_primary_ata_queue.write_i].lba = lba;
		((command_t *) PROC_primary_ata_queue.buffer)[PROC_primary_ata_queue.write_i].bus_port = ata_device -> bus_port;
		((command_t *) PROC_primary_ata_queue.buffer)[PROC_primary_ata_queue.write_i].is_slave = ata_device -> is_slave;
		PROC_primary_ata_queue.write_i++;

               	PROC_block_on(&PROC_primary_ata_queue, 1);
	} else {
		if(PROC_secondary_ata_queue.write_i == PROC_secondary_ata_queue.read_i){
			send_ata_command(ata_device -> bus_port, ata_device -> is_slave, lba, 0x24);
		}

		((command_t *) PROC_secondary_ata_queue.buffer)[PROC_secondary_ata_queue.write_i].command_byte = 0x24;
		((command_t *) PROC_secondary_ata_queue.buffer)[PROC_secondary_ata_queue.write_i].pointer = dest;
		((command_t *) PROC_secondary_ata_queue.buffer)[PROC_secondary_ata_queue.write_i].lba = lba;
		((command_t *) PROC_secondary_ata_queue.buffer)[PROC_secondary_ata_queue.write_i].bus_port = ata_device -> bus_port;
		((command_t *) PROC_secondary_ata_queue.buffer)[PROC_secondary_ata_queue.write_i].is_slave = ata_device -> is_slave;
		PROC_secondary_ata_queue.write_i++;

               	PROC_block_on(&PROC_secondary_ata_queue, 1);
	}

	STI_IF;
	return 512;
}

uint64_t write_ata_drive(block_device_t *self, uint64_t lba, uint8_t *source) {
	uint8_t local_interrupt_flag = 0x00;

	ata_device_t *ata_device = (ata_device_t *) self;
	
	CLI_IF;

	// PROC block on ata device
	if(ata_device -> bus_port == PRIMARY_BASE) {
		if(PROC_primary_ata_queue.write_i == PROC_primary_ata_queue.read_i){
			send_ata_command(ata_device -> bus_port, ata_device -> is_slave, lba, 0x34);
		}

		((command_t *) PROC_primary_ata_queue.buffer)[PROC_primary_ata_queue.write_i].command_byte = 0x34;
		((command_t *) PROC_primary_ata_queue.buffer)[PROC_primary_ata_queue.write_i].pointer = source;
		((command_t *) PROC_primary_ata_queue.buffer)[PROC_primary_ata_queue.write_i].lba = lba;
		((command_t *) PROC_primary_ata_queue.buffer)[PROC_primary_ata_queue.write_i].bus_port = ata_device -> bus_port;
		((command_t *) PROC_primary_ata_queue.buffer)[PROC_primary_ata_queue.write_i].is_slave = ata_device -> is_slave;
		PROC_primary_ata_queue.write_i++;

               	PROC_block_on(&PROC_primary_ata_queue, 1);
	} else {
		if(PROC_secondary_ata_queue.write_i == PROC_secondary_ata_queue.read_i){
			send_ata_command(ata_device -> bus_port, ata_device -> is_slave, lba, 0x24);
		}

		((command_t *) PROC_secondary_ata_queue.buffer)[PROC_secondary_ata_queue.write_i].command_byte = 0x24;
		((command_t *) PROC_secondary_ata_queue.buffer)[PROC_secondary_ata_queue.write_i].pointer = source;
		((command_t *) PROC_secondary_ata_queue.buffer)[PROC_secondary_ata_queue.write_i].lba = lba;
		((command_t *) PROC_secondary_ata_queue.buffer)[PROC_secondary_ata_queue.write_i].bus_port = ata_device -> bus_port;
		((command_t *) PROC_secondary_ata_queue.buffer)[PROC_secondary_ata_queue.write_i].is_slave = ata_device -> is_slave;
		PROC_secondary_ata_queue.write_i++;

               	PROC_block_on(&PROC_secondary_ata_queue, 1);
	}

	STI_IF;
	return 512;
}

uint64_t write_partition(block_device_t *self, uint64_t lba, uint8_t *source) {
	partition_device_t *partition_device = (partition_device_t *) self;
	
	return write_ata_drive(self, partition_device -> lba_offset + lba, source);
}

uint64_t read_partition(block_device_t *self, uint64_t lba, uint8_t *dest) {
	partition_device_t *partition_device = (partition_device_t *) self;
	
	return read_ata_drive(self, partition_device -> lba_offset + lba, dest);
}

void probe_ata_drives() {
	block_device_t *curr;

	if(block_device_head) {
		for(curr = block_device_head; curr; curr = curr -> next) {
			if (curr -> type == ATA_TYPE) {
				probe_ata_drive(curr);
				if (((ata_device_t *) curr) -> is_slave == 0) {
					outb(0x00, ((ata_device_t *) curr) -> control_port);
				}
			}
		}
	} else {
		printk("No block devices in list to probe at all. Halting.");
		asm volatile("hlt");
	}
}

void probe_ata_drive(block_device_t *self) {
	ata_device_t *ata_device = (ata_device_t *) self;
	partition_device_t *partition_devices[4];
	partition_entry_t *partition_entries;
	uint8_t mbr[512];
	uint8_t status_register;
	uint16_t i;

	send_ata_command(ata_device -> bus_port, ata_device -> is_slave, 0x00, 0x24);

	status_register = inb(ata_device -> bus_port + 0x07);

	if(status_register == 0x00) {
		return;
	}

	while((status_register & 0x08) == 0x00 && (status_register & 0x01) == 0) {
		status_register = inb(ata_device -> bus_port + 0x07);
	}

	if(status_register & 0x08) {
		for(i = 0; i < 256; i++) {
			((uint16_t *) mbr)[i] = inw(ata_device -> bus_port);
		}

	}else{
		printk("ATA drive cannot be probed. Halting.");
		asm volatile("hlt");
	}

	partition_entries = (partition_entry_t *) (mbr + 446);

	for(i = 0; i < 4; i++) {
		partition_devices[i] = (partition_device_t *) kmalloc(sizeof(partition_device_t));
	
		partition_devices[i] -> lba_offset = partition_entries[i].partition_base;
		partition_devices[i] -> block_count = partition_entries[i].sector_count;
		((ata_device_t *) partition_devices[i]) -> bus_port = ata_device -> bus_port;
		((ata_device_t *) partition_devices[i]) -> control_port = ata_device -> control_port;
		((ata_device_t *) partition_devices[i]) -> is_slave = ata_device -> is_slave;
		((block_device_t *) partition_devices[i]) -> type = PARTITION_TYPE;
		((block_device_t *) partition_devices[i]) -> read_block = read_partition;
		((block_device_t *) partition_devices[i]) -> write_block = write_partition;
		((block_device_t *) partition_devices[i]) -> block_size = self -> block_size;
		((block_device_t *) partition_devices[i]) -> total_length = self -> total_length;
		if(partition_devices[i] -> block_count == 0) {
			kfree((uint8_t *) partition_devices[i]);
		} else {
			BLK_register((block_device_t *) partition_devices[i]);
	
		}
		printk("Partition: %d, Partition Base: %d, Sector Count: %d\n", i, partition_devices[i] -> lba_offset, partition_devices[i] -> block_count);
	}
}

void on_primary_ata() {
	uint8_t status_register;
	command_t command;
	uint16_t i;

	status_register = inb(PRIMARY_BASE + 7);	
	
	if(status_register & 0x01) {
		printk("ERROR Detected in Ata primary drive.");
		asm volatile("hlt");
		return;
	}

	if(PROC_primary_ata_queue.read_i == PROC_primary_ata_queue.write_i) {
		PROC_unblock_head(&PROC_primary_ata_queue);
		return;
	}

	if(status_register & 0x08) {
		        command = ((command_t *) PROC_primary_ata_queue.buffer)[PROC_primary_ata_queue.read_i++];
			
			if (command.command_byte == 0x24) {
				for(i = 0; i < 256; i++) {
					((uint16_t *)command.pointer)[i] = inw(PRIMARY_BASE);
				}
			} else {
				for(i = 0; i < 256; i++) {
					outw(((uint16_t *)command.pointer)[i], PRIMARY_BASE);
				}
			}

			if(PROC_primary_ata_queue.write_i != PROC_primary_ata_queue.read_i) {
				send_ata_command(command.bus_port, command.is_slave, command.lba, command.command_byte);
			}

			PROC_unblock_head(&PROC_primary_ata_queue);
	}
}

void on_secondary_ata() {
	uint8_t status_register;
	command_t command;
	uint16_t i;

	status_register = inb(SECONDARY_BASE + 7);	
	
	if(status_register & 0x01) {
		printk("ERROR Detected in Ata primary drive.");
		asm volatile("hlt");
		return;
	}

	if(PROC_secondary_ata_queue.read_i == PROC_secondary_ata_queue.write_i) {
		PROC_unblock_head(&PROC_secondary_ata_queue);
		return;
	}

	if(status_register & 0x08) {
		        command = ((command_t *) PROC_secondary_ata_queue.buffer)[PROC_secondary_ata_queue.read_i++];
			
			if (command.command_byte == 0x24) {
				for(i = 0; i < 256; i++) {
					((uint16_t *)command.pointer)[i] = inw(SECONDARY_BASE);
				}
			} else {
				for(i = 0; i < 256; i++) {
					outw(((uint16_t *)command.pointer)[i], SECONDARY_BASE);
				}
			}

			if(PROC_secondary_ata_queue.write_i != PROC_secondary_ata_queue.read_i) {
				send_ata_command(command.bus_port, command.is_slave, command.lba, command.command_byte);
			}

			PROC_unblock_head(&PROC_secondary_ata_queue);
	}
}
