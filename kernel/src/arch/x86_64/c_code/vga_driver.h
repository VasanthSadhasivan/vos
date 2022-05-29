#ifndef VGA_DRIVER_H__
#define VGA_DRIVER_H__

#include"my_stdlib.h"
#include"string.h"
#include"interrupt.h"
#include"serial.h"

#define VGA_BASE 0xb8000

void VGA_clear(void);
void VGA_display_char(char);
void VGA_display_string(const char *);
int VGA_row_count();
int VGA_col_count();
void VGA_display_attr_char(int x, int y, char c, int fg, int bg);

#endif
