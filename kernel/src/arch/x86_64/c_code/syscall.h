#ifndef SYSCALL_H__
#define SYSCALL_H__

#include"keyboard.h"
#include"interrupt.h"
#include<stdint.h>

void syscall_init();
void on_int80();

#endif
