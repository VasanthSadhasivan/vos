#ifndef USER_H__
#define USER_H__

#include"kthread.h"
#include"paging.h"
#include"my_stdlib.h"
#include"elf.h"
#include"framing.h"
#include"paging.h"

#include<stdint.h>

pt1_entry_t *new_user_page_table();
kthread_function_t *load_user_elf(uint8_t *filename, pt1_entry_t *user_page_table);

#endif
