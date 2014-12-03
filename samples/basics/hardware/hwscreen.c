#include "hwscreen.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>

char* scr_buffer;
int scr_xres;
int scr_yres;

#define HWSCRFUNC_INFO 0
#define HWSCRFUNC_MAP 1

void scr_init(void)
{
	// Get the screen info
	HwiData data;
	hwiCall(HWBUS_SCR, HWSCRFUNC_INFO, &data);
	scr_buffer = (char*)data.regs[0];
	scr_xres = data.regs[1];
	scr_yres = data.regs[2];
}

void scr_map(void* buffer)
{
	HwiData data;
	data.regs[0] = (u32)buffer;
	u32 res = hwiCall(HWBUS_SCR, HWSCRFUNC_MAP, &data);
	assert(res==HWIERR_SUCCESS);
	scr_buffer = buffer;
}

static unsigned short* scr_getXYPtr(int x, int y)
{
	return (unsigned short*)(scr_buffer + (y*(scr_xres*2) + x*2));
}

void scr_clear(void)
{
	memset(scr_buffer, 0, scr_xres*scr_yres*2);
}

void scr_printCharAtXY(int x, int y, unsigned char ch)
{
	*scr_getXYPtr(x,y) = (0x0F00|ch);
}

void scr_printStringAtXY(int x, int y, const char* str)
{
	unsigned short* ptr = scr_getXYPtr(x,y);
	while(*str) {
		*ptr = 0xF00|*str;
		ptr++; str++;
	}
}

int scr_printfAtXY(int x, int y, const char* fmt, ...)
{
	char buf[256];
	va_list ap;
	va_start(ap, fmt);	
	int res = vsprintf(buf, fmt, ap);
	scr_printStringAtXY(x,y,buf);
	return res;
}
