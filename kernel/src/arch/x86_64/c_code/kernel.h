#ifndef KERNEL_H__
#define KERNEL_H__
#include"my_stdio.h"
#include"keyboard.h"
#include"interrupt.h"
#include"serial.h"
#include"framing.h"
#include"paging.h"
#include"heap.h"
#include"kthread.h"
#include"snakes.h"
#include"block.h"
#include"ata.h"
#include"fat.h"
#include"extras/md5.h"
#include"elf.h"
#include"syscall.h"
#include"user.h"
#include<stdint.h>

void kernel_main();
void kernel_virtualized_main();

void thread_1(void *arg);
void thread_2(void *arg);
#endif
