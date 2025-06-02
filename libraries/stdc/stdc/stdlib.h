#ifndef _stdc_stdlib_h_
#define _stdc_stdlib_h_

#include <stddef.h>
#include "detail/memdetails.h"

char* itoa(int value, char *str, int base);
long strtol(const char* str, char** str_end, int base);
unsigned long strtoul(const char* str, char** str_end, int base);

// NOTE: This is not really part of the standard
// Copied from https://github.com/antongus/stm32tpl
char* ftoa(double f, char* buf, int precision);

void qsort(
	void *array, size_t count, size_t elementSize,
	int (*cmp)(const void *, const void *));

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define min(a,b) ((a)<=(b) ? (a) : (b))
#define max(a,b) ((a) >(b) ? (a) : (b))

#endif
