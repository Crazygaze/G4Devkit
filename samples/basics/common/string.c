#include "string.h"

int strlen(const char* str)
{
	const char* end = str;
	while (*end) {
		end++;
	}
	
	return end - str;
}
