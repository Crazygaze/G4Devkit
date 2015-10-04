#ifndef _stdlib_h_
#define _stdlib_h_

#include <stddef.h>

void itoa(int value, char *str, int base);
long strtol(const char* str, char** str_end, int base);
unsigned long strtoul(const char* str, char** str_end, int base);

#endif
