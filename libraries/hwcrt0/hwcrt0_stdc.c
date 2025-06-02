#include "hwcrt0_stdc.h"

int strlen(const char* str)
{
	assert(str);
	const char* end = str;
	while (*end) {
		end++;
	}

	return end - str;
}
