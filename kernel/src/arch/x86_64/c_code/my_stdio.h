#ifndef MY_STDIO_H__
#define MY_STDIO_H__

#include"vga_driver.h"
#include"string.h"
#include<stdarg.h>
//typedef __builtin_va_list va_list;
//#define va_start __builtin_va_start
//#define va_end __builtin_va_end
//#define va_arg __builtin_va_arg

void printk(char *str, ...);
void print_int(unsigned long long value, int base);
unsigned long print_int_submodifiers(char *str, unsigned long long value);
void __stack_chk_fail(void);
#endif
