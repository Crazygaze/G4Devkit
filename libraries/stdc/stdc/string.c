#include "detail/stdc_internal.h"

#include <string.h>
#include <assert.h>

char* strcpy(char* dst, const char* src)
{
	assert(dst != NULL && src != NULL);
	char* temp = dst;
	*dst = *src;
	while (*dst) {
		++dst;
		++src;
		*dst = *src;
	}
	return temp;
}

// copied from http://linux.die.net/man/3/strncpy
char* strncpy(char* dest, const char* src, size_t num)
{
	assert(dest && src);
	size_t i;
	for (i = 0; i < num && src[i] != '\0'; i++)
		dest[i] = src[i];
	for (; i < num; i++)
		dest[i] = '\0';
	return dest;
}

int strcmp(const char* str1, const char* str2)
{
	assert(str1 && str2);
	while (*str1 && (*str1 == *str2)) {
		str1++;
		str2++;
	}
	return *(const unsigned char*)str1 - *(const unsigned char*)str2;
}

char* strchr(const char* str, int ch)
{
	for(; *str != '\0' && *str != ch; ++str)
	{
	}

	// NOTE: This correctly handles the edge case where ch is `\0`
	return *str == ch ? (char*)str : NULL;
}

// See https://en.cppreference.com/w/c/string/byte/strrchr
// The implementation is simple. It just iterates the string and remembers
// the last position the character was found.
char* strrchr(const char* str, int ch)
{
	const char* pos = NULL;
	do {
		if (*str == (char)ch)
			pos = str;
	} while(*str++);

	return (char*)pos;
}
