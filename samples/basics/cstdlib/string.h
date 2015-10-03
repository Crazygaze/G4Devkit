#ifndef _string_h_
#define _string_h_

#include <stddef.h>

int strlen(const char* str);
void* memmove(void* dest, const void* src, int count);
void* memset(void* dest, int c, int count);
void* memcpy(void* dest, const void* src, int count);

char* strncpy(char *dest, const char *src, size_t num);
int strcmp(const char *str1, const char *str2);

#endif
