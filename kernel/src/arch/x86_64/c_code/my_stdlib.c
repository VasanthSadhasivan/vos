#include"my_stdlib.h"
void my_memcpy(unsigned char *destination, unsigned char *source, unsigned long long len){
	unsigned long long i;

	for(i = 0; i < len; i++) {
		destination[i] = source[i];
	}
}



uint8_t my_memcmp(unsigned char *destination, unsigned char *source, unsigned long long len){
	unsigned long long i;

	for(i = 0; i < len; i++) {
		if(destination[i] != source[i]) {
			return 1;
		}
	}

	return 0;
}

void my_memset(unsigned char *destination, unsigned char value,  unsigned long long len){
	unsigned long long i;

	for(i = 0; i < len; i++) {
		destination[i] = value;
	}
}

uint64_t my_strlen(uint8_t *string) {
	uint64_t i;

	for(i = 0; string[i]; i++);

	return i;
}
