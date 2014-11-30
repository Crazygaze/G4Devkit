#include "stdlib.h"

void itoa(int value, char *str, int base)
{
	char tmp[32];// be careful with the length of the buffer
	char *tp = tmp;
	int i;
	unsigned v;
	int sign;

	sign = (base == 10 && value < 0);
	if (sign)   v = -value;
	else    v = (unsigned)value;

	while (v || tp == tmp)
	{
		i = v % base;
		v /= base; // v/=base uses less CPU clocks than v=v/base does
		if (i < 10)
			*tp++ = i + '0';
		else
			*tp++ = i + 'a' - 10;
	}

	if (sign)
		*str++ = '-';
	while (tp > tmp)
		*str++ = *--tp;
	*str = 0;

}