#include "string.h"
#include "assert.h"

int strlen(const char* str)
{
	const char* end = str;
	while (*end) {
		end++;
	}
	
	return end - str;
}


// copied from http://linux.die.net/man/3/strncpy
char* strncpy(char *dest, const char *src, size_t num)
{
	assert(dest && src);
	size_t i;
	for (i = 0; i < num && src[i] != '\0'; i++)
		dest[i] = src[i];
	for ( ; i < num; i++)
		dest[i] = '\0';
	return dest;
}

int strcmp(const char *str1, const char *str2)
{
	assert(str1 && str2);
    while(*str1 && (*str1==*str2)) {
        str1++; str2++;
	}
    return *(const unsigned char*)str1-*(const unsigned char*)str2;
}
