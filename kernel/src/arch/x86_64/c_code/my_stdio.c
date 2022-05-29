#include"my_stdio.h"

void printk(char *str, ...) {
	unsigned long i;
	va_list arg;
	unsigned long long value;
	va_start(arg, str);

	for(i = 0; i < strlen(str); i++) {
		if (i < strlen(str) - 1 && str[i] == '%') {
			switch(str[i + 1]) {
				case '%' :
					VGA_display_char('%');
					break;
				case 'd' :
					value = (int) va_arg(arg, int);
					if(value < 0) {
						VGA_display_char('-');
						value = value * -1;
					}
					print_int(value, 10);
					break;
				case 'u' :
					value = (unsigned int) va_arg(arg, unsigned int);
					print_int(value, 10);
					break;
				case 'x' :
					value = (unsigned int) va_arg(arg, unsigned int);
					VGA_display_string("0x");
					print_int(value, 16);
					break;
				case 'c' :
					value = (unsigned long long) va_arg(arg, int);
					VGA_display_char((char) value);
					break;
				case 'p' :
					value = (unsigned long long) va_arg(arg, char *);
					VGA_display_string("0x");
					print_int(value, 16);
                                        break;
				case 's' :
					value = (unsigned long long) va_arg(arg, char *);
                                        VGA_display_string((char *) value);
                                        break;
				case 'h' :
					value = va_arg(arg, unsigned long long);
					i += print_int_submodifiers(str + i + 1, value);
					break;
				case 'l' :
					value = va_arg(arg, unsigned long long);
					i += print_int_submodifiers(str + i + 1, value);
					break;
				case 'q' :
					value = va_arg(arg, unsigned long long);
					i += print_int_submodifiers(str + i + 1, value);
					break;
				default :
					VGA_display_char(str[i]);
					VGA_display_char(str[i + 1]);
					break;
			}
			i += 1;
	
		} else {
			VGA_display_char(str[i]);
		}
	}
}

void print_int(unsigned long long value, int base) {
	static char *rep = "0123456789ABCDEF";
	static char buffer[50];
	char *ptr;
	
	ptr = &buffer[49];
	*ptr = '\0';

	do {
		*--ptr = rep[value % base];
		value /= base;
	} while (value != 0);

	VGA_display_string(ptr);
}

unsigned long print_int_submodifiers(char *str, unsigned long long value) {
	if (strlen(str) == 1) {
		switch(str[0]) {
			case 'h' :
				if(((short int) value) < 0) {
					VGA_display_char('-');
					print_int(((short int) value) * -1, 10);
				} else {
					print_int(value, 10);
				}
				break;
			case 'l' :
				if(((long int) value) < 0) {
					VGA_display_char('-');
					print_int(value * -1, 10);
				} else {
					print_int(value, 10);
				}
				break;	
			case 'q' :
                                if(((double) value) < 0) {
                                        VGA_display_char('-');
                                        print_int(value * -1, 10);
                                } else {
                                        print_int(value, 10);
                                }
				break;
		}
	} else {
                switch(str[0]) {
                        case 'h' :
				switch(str[1]) {
					case 'u' :
                                        	print_int(value, 10);
						return 1;
					case 'x' :
						VGA_display_string("0x");
						print_int(value, 16);
						return 1;
					case 'd' :
						if(((short int) value) < 0) {
                                        		VGA_display_char('-');
                                        		print_int(((short int) value) * -1, 10);
                                		} else {
                                        		print_int(value, 10);
                               			}
						return 1;
					default :
                                                if(((short int) value) < 0) {
                                                        VGA_display_char('-');
                                                        print_int(((short int) value) * -1, 10);
                                                } else {
                                                        print_int(value, 10);
                                                }
				}
				break;

                        case 'l' :
                                switch(str[1]) {
                                        case 'u' :
                                                print_int(value, 10);
                                                return 1;
                                        case 'x' :
						VGA_display_string("0x");
                                                print_int(value, 16);
                                                return 1;
                                        case 'd' :
                                                if(((long) value) < 0) {
                                                        VGA_display_char('-');
                                                        print_int(((long) value) * -1, 10);
                                                } else {
                                                        print_int(value, 10);
                                                }
						return 1;
					default :
                                                if(((long) value) < 0) {
                                                        VGA_display_char('-');
                                                        print_int(((long) value) * -1, 10);
                                                } else {
                                                        print_int(value, 10);
                                                }
                                }
                                break;

                        case 'q' :
                                switch(str[1]) {
                                        case 'u' :
                                                print_int(value, 10);
                                                return 1;
                                        case 'x' :
						VGA_display_string("0x");
                                                print_int(value, 16);
                                                return 1;
                                        case 'd' :
                                                if(((double) value) < 0) {
                                                        VGA_display_char('-');
                                                        print_int(((double) value) * -1, 10);
                                                } else {
                                                        print_int(value, 10);
                                                }
						return 1;
					default :
						if(((double) value) < 0) {
                                                        VGA_display_char('-');
                                                        print_int(((double) value) * -1, 10);
                                                } else {
                                                        print_int(value, 10);
                                                }
                                }
                                break;
                }
	}

	return 0;
}

void __stack_chk_fail(){

}
