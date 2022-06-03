#ifndef MY_STDLIB_H__
#define MY_STDLIB_H__

#include<stdint.h>

void my_memcpy(unsigned char *, unsigned char *, unsigned long long);
uint8_t my_memcmp(unsigned char *, unsigned char *, unsigned long long);
void my_memset(unsigned char *, unsigned char, unsigned long long);
uint64_t my_strlen(uint8_t *);

#endif
