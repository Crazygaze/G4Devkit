#include "stdlib_shared.h"

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

typedef union {
	long	L;
	float	F;
}	 LF_t;

char *ftoa(float f, int *status)
{
	long mantissa, int_part, frac_part;
	short exp2;
	LF_t x;
	char *p;
	static char outbuf[15];

	*status = 0;
	if (f == 0.0)
	{
		outbuf[0] = '0';
		outbuf[1] = '.';
		outbuf[2] = '0';
		outbuf[3] = 0;
		return outbuf;
	}
	x.F = f;

	exp2 = (unsigned char)(x.L >> 23) - 127;
	mantissa = (x.L & 0xFFFFFF) | 0x800000;
	frac_part = 0;
	int_part = 0;

	if (exp2 >= 31)
	{
		*status = _FTOA_TOO_LARGE;
		return 0;
	}
	else if (exp2 < -23)
	{
		*status = _FTOA_TOO_SMALL;
		return 0;
	}
	else if (exp2 >= 23)
		int_part = mantissa << (exp2 - 23);
	else if (exp2 >= 0)
	{
		int_part = mantissa >> (23 - exp2);
		frac_part = (mantissa << (exp2 + 1)) & 0xFFFFFF;
	}
	else /* if (exp2 < 0) */
		frac_part = (mantissa & 0xFFFFFF) >> -(exp2 + 1);

	p = outbuf;

	if (x.L < 0)
		*p++ = '-';

	if (int_part == 0)
		*p++ = '0';
	else
	{
		itoa(int_part, p, 10);
		while (*p)
			p++;
	}
	*p++ = '.';

	if (frac_part == 0)
		*p++ = '0';
	else
	{
		char m, max;

		max = sizeof (outbuf)-(p - outbuf) - 1;
		if (max > 7)
			max = 7;
		/* print BCD */
		for (m = 0; m < max; m++)
		{
			/* frac_part *= 10;	*/
			frac_part = (frac_part << 3) + (frac_part << 1);

			*p++ = (frac_part >> 24) + '0';
			frac_part &= 0xFFFFFF;
		}
		/* delete ending zeroes */
		for (--p; p[0] == '0' && p[-1] != '.'; --p)
			;
		++p;
	}
	*p = 0;

	return outbuf;
}
