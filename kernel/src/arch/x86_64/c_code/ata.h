#ifndef ATA_H__
#define ATA_H__

#include"port.h"
#include"my_stdio.h"
#include"block.h"
#include"heap.h"
#include"kthread.h"

typedef struct {
	block_device_t parent;
	uint16_t bus_port;
	uint16_t control_port;
	uint8_t is_slave;
} ata_device_t;

typedef struct {
	ata_device_t parent;
	uint64_t lba_offset;
	uint64_t block_count;
} partition_device_t;

typedef struct __attribute__((packed)) {
	uint64_t ignore;
	uint32_t partition_base;
	uint32_t sector_count;
} partition_entry_t;

void setup_ata_devices();
void enumerate_pci();
void register_ata_drives();
void register_ata_device_in_bus(uint16_t bus_port, uint16_t control_port);
void probe_ata_drives();
void probe_ata_drive(block_device_t *self);
void ns_delay(uint16_t bus_port);
uint64_t read_ata_drive(block_device_t *self, uint64_t lba, uint8_t *dest);
uint64_t write_ata_drive(block_device_t *self, uint64_t lba, uint8_t *source);
void on_primary_ata();
void on_secondary_ata();
uint64_t get_drive_size(uint16_t bus_port, uint16_t control_port, uint8_t is_slave);
uint64_t write_partition(block_device_t *self, uint64_t lba, uint8_t *source);
uint64_t read_partition(block_device_t *self, uint64_t lba, uint8_t *source);

#endif
