#include "string.h"

unsigned long strlen(char *s) {
	unsigned long i = 0;

	for(i = 0; s[i] != 0x0; i++);

	return i;
}


char* sp = 0x00; /* the start position of the string */

char* strtok(char* str, const char* delimiters) {

    int i = 0;
    int len = strlen((char *) delimiters);

    /* check in the delimiters */
   // if(len == 0)
   //     printf("delimiters are empty\n");

    /* if the original string has nothing left */
    if(!str && !sp)
        return 0x00;

    /* initialize the sp during the first call */
    if(str && !sp)
        sp = str;

    /* find the start of the substring, skip delimiters */
    char* p_start = sp;
    while(1) {
        for(i = 0; i < len; i ++) {
            if(*p_start == delimiters[i]) {
                p_start ++;
                break;
            }
        }

        if(i == len) {
               sp = p_start;
               break;
        }
    }

    /* return NULL if nothing left */
    if(*sp == '\0') {
        sp = 0x00;
        return sp;
    }

    /* find the end of the substring, and
        replace the delimiter with null */
    while(*sp != '\0') {
        for(i = 0; i < len; i ++) {
            if(*sp == delimiters[i]) {
                *sp = '\0';
                break;
            }
        }

        sp ++;
        if (i < len)
            break;
    }

    return p_start;
}
