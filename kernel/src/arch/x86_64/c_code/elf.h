#ifndef ELF_H__
#define ELF_H__

#include"kthread.h"
#include"fat.h"
#include"my_stdlib.h"
#include"heap.h"
#include<stdint.h>

kthread_function_t *load_elf(uint8_t *file_location, uint8_t *address);

#endif
