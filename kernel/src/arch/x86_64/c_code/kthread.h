#ifndef KTHREAD_H__
#define KTHREAD_H__

#include"interrupt.h"
#include"heap.h"

typedef struct kthread_t_s{
	struct kthread_t_s *next;
	uint64_t pid;
	uint64_t rax;
	uint64_t rcx;
	uint64_t rdx;
	uint64_t rbx;
	uint64_t rbp;
	uint64_t rsi;
	uint64_t rdi;
	uint64_t r8;
	uint64_t r9;
	uint64_t r10;
	uint64_t r11;
	uint64_t r12;
	uint64_t r13;
	uint64_t r14;
	uint64_t r15;
	uint16_t ds;
	uint16_t es;
	uint16_t fs;
	uint16_t gs;
	uint16_t ss;
	uint64_t rsp;
	uint64_t rflags;
	uint16_t cs;
	uint64_t rip;
} kthread_t;

typedef struct {
	kthread_t *procs;
	void * buffer;
	uint8_t read_i;
	uint8_t write_i;
} external_queue_t;

typedef struct {
	uint8_t command_byte;
	uint8_t *pointer;
	uint64_t lba;
	uint16_t bus_port;
	uint8_t is_slave;
} command_t;

typedef void (kthread_function_t) (void *);

void PROC_init();
kthread_t *PROC_create_kthread(kthread_function_t thread_loc, void *arg);
void PROC_reschedule();
void on_yield();
void yield();
void save_registers(uint64_t *isr_rbp, kthread_t *thread_p);
void load_registers(uint64_t *isr_rbp, kthread_t *thread_p);
void PROC_run();
void on_PROC_run();
void on_kexit();
void on_return();
void kexit();
void PROC_external_queue_init();
void PROC_block_on(external_queue_t *queue, uint8_t enable_ints);
void on_block();
void PROC_unblock_all(external_queue_t *queue);
void PROC_unblock_head(external_queue_t *queue);

external_queue_t PROC_keyboard_queue;
external_queue_t PROC_primary_ata_queue;
external_queue_t PROC_secondary_ata_queue;
kthread_t *PROC_head;

#endif
