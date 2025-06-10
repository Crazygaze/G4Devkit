#ifndef _stdc_string_h_
#define _stdc_string_h_

#include "detail/config.h"
#include <hwcrt0_stdc.h>


char* strcpy(char *dest, const char *src);
char* strncpy(char *dest, const char *src, size_t num);
int strcmp(const char *str1, const char *str2);

char* strchr(const char* str, int ch);
char* strrchr(const char* str, int ch);

#endif
