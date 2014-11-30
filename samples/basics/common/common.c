#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

void printCharacterAtXY(int x, int y, unsigned char ch)
{
	*getScreenPtr(x,y) = (0x0F00|ch);
}

void printStringAtXY(int x, int y, const char* str)
{
	unsigned short* ptr = getScreenPtr(x,y);
	while(*str) {
		*ptr = 0xF00|*str;
		ptr++; str++;
	}
}

int printfAtXY(int x, int y, const char* fmt, ...)
{
	char buf[256];
	va_list ap;
	va_start(ap, fmt);	
	int res = vsprintf(buf, fmt, ap);
	printStringAtXY(x,y,buf);
	return res;
}
