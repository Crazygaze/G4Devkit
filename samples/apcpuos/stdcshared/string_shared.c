#include "string_shared.h"
#include "assert_shared.h"

int strlen(const char* str)
{
	assert(str);
	const char* end = str;
	while (*end)
	{
		end++;
	}
	
	return end - str;
}

// added by brick_btv (copied from my mind)
char* strcpy(char* dest, const char *src)
{
	assert(dest && src);
	memcpy(dest, src, strlen(src)*sizeof(char));
	
	return dest;
}

// added by brick_btv 
int find (const char* str, const char ch, int start_pos)
{
	assert(str);
	int str_len = strlen(str);	
	int i = start_pos;
	char ch_i = str[i];
	
	if (start_pos > str_len){
		return -1;
	}
	
	for (int i = start_pos; i < str_len; i++){
		if (str[i] == ch){
			return i;
		}
	}
	
	return -1;
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
