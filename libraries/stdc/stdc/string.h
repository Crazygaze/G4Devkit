#ifndef _stdc_string_h_
#define _stdc_string_h_

#include "detail/config.h"
#include <hwcrt0_stdc.h>

char* strcpy(char *dest, const char *src);
char* strncpy(char *dest, const char *src, size_t num);
int strcmp(const char *str1, const char *str2);
char* strchr(const char* str, int ch);
char* strrchr(const char* str, int ch);
char* strcat(char* dest, const char* src);
int memcmp(const void* lhs, const void* rhs, size_t count);

/*!
 * Utility macro to create a zeroed struct variable.
 * This is because of a compiler limitation when initializing structs with {0}.
 * The {0} initialization works, but the compiler emits an actual zero array,
 * and memcpy that, instead of just doing a memset on the struct itself.
 * See https://github.com/Crazygaze/G4Devkit/issues/4
 */
#define defineZeroed(type, varname) \
	type varname; \
	memset(&varname, 0, sizeof(type));

/*!
 * Similar to defineZeroed, but defines an array.
 * E.g, to do something similar to this:
 * ```
 * int foo[4] = { 0 }
 * ```
 * , you can use:
 * ```
 * defineZeroedArray(int, foo, 4);
 */
#define defineZeroedArray(type, varname, count) \
	type varname[count]; \
	memset(varname, 0, sizeof(varname));
	

#endif
