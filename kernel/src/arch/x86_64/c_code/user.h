#ifndef USER_H__
#define USER_H__

#include"kthread.h"
#include"paging.h"
#include"my_stdlib.h"
#include"elf.h"
#include"framing.h"
#include"paging.h"

#include<stdint.h>

void start_user_process(uint8_t *filename);

#endif
