#include "common.h"

extern char* screenBuffer;
extern int screenXRes;
extern int screenYRes;

//******************************************************************************
//		System functions
//******************************************************************************

void pause(int ms)
{
	double now = getRunningTimeSeconds();
	double finalTime = now + (double)ms/1000;
	
	do {
		now = getRunningTimeSeconds();
	} while(now<finalTime);
}

//******************************************************************************
//		Screen functions
//******************************************************************************

unsigned short* getScreenPtr(int x, int y)
{
	return (unsigned short*)(screenBuffer + (y*(screenXRes*2) + x*2));
}

void clearScreen(void)
{
	memset(screenBuffer, 0, screenXRes*screenYRes*2);
}

void printNumber(int x, int y, int number, int base)
{
	char buf[20];
	itoa(number, buf, base);
	printString(x,y,buf);
}

void printCharacter(int x, int y, unsigned char ch)
{
	*getScreenPtr(x,y) = (0x0F00|ch);
}

void printString(int x, int y, const char* str)
{
	unsigned short* ptr = getScreenPtr(x,y);
	while(*str) {
		*ptr = 0xF00|*str;
		ptr++; str++;
	}
}

//******************************************************************************
//		Standard C library functions
//******************************************************************************

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

int strlen(const char* str)
{
	const char* end = str;
	while (*end) {
		end++;
	}
	
	return end - str;
}


