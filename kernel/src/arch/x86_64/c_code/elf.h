#ifndef ELF_H__
#define ELF_H__

#include"kthread.h"
#include"fat.h"
#include"my_stdlib.h"
#include"heap.h"
#include<stdint.h>

kthread_function_t *load_elf(uint8_t *file_location);
kthread_function_t *load_elf_separate_pt(uint8_t *file_location, pt1_entry_t *page_table_base);

#endif
