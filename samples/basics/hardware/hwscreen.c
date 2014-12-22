#include "hwscreen.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>


static ScreenInfo scr;
static u16* cursor;

#define HWSCRFUNC_INFO 0
#define HWSCRFUNC_MAP 1


static unsigned short* scr_getXYPtr(int x, int y)
{
	return (unsigned short*)(scr.buffer + (y*(scr.xres*2) + x*2));
}

void scr_getInfo(ScreenInfo* info)
{
	// Get the screen info
	HwiData data;
	int res = hwiCall(HWBUS_SCR, HWSCRFUNC_INFO, &data);
	always_assert(res==HWIERR_SUCCESS);
	info->buffer = (char*)data.regs[0];
	info->xres = data.regs[1];
	info->yres = data.regs[2];
	info->bytesPerCharacter = data.regs[3];
}

void scr_init(void)
{
	scr_getInfo(&scr);
	cursor = scr_getXYPtr(0,0);
}

void scr_map(void* buffer)
{
	HwiData data;
	data.regs[0] = (u32)buffer;
	int res = hwiCall(HWBUS_SCR, HWSCRFUNC_MAP, &data);
	always_assert(res==HWIERR_SUCCESS);
	scr.buffer = buffer;
}

void scr_clear(void)
{
	memset(scr.buffer, 0, scr.xres*scr.yres*2);
	cursor = scr_getXYPtr(0,0);
}

void scr_clearArea(int x1, int y1, int x2, int y2)
{
	u8* start = (u8*)scr_getXYPtr(x1,y1);
	u8* end = (u8*)scr_getXYPtr(x2,y2);
	memset(start, 0, (end-start)+1);
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

void scr_scroll(int lines)
{
	if (!lines)
		return;
		
	memmove(
		scr_getXYPtr(0,0), scr_getXYPtr(0,lines),
		SCR_BUFFERSIZE - (lines*SCR_XRES*SCR_BYTESPERCHARACTER));
		
	scr_clearArea(0,SCR_YRES-lines, SCR_XRES, SCR_YRES-1);
}

static void scr_checkScroll(const u16* end)
{
	if (cursor>=end) {
		scr_scroll(1);
		cursor = scr_getXYPtr(0, scr.yres-1);
	}	
}

static void scr_printStringHelper(unsigned char ch, int count, const u16* end)
{
	while(count--)
	{
		scr_checkScroll(end);
		*cursor++ = 0x0F00 | ch;
	}
}

void scr_printString(const char* str)
{
	u16* end = scr_getXYPtr(scr.xres-1, scr.yres-1);
	while(*str)
	{
		switch(*str)
		{
		case 0x09: // TAB
			scr_printStringHelper(' ', 4, end);
			break;
		case '\n':
			cursor = scr_getXYPtr(
				0, ((cursor - (u16*)scr.buffer)/scr.xres) + 1);
			scr_checkScroll(end);
			break;
		case '\r':
			cursor = scr_getXYPtr(0, (cursor - (u16*)scr.buffer)/scr.xres);
			break;
		default:
				scr_printStringHelper(*str,1,end);
		}

		str++;
	}
}

void scr_printf(const char* fmt, ...)
{
	char buf[256];
	va_list ap;
	va_start(ap, fmt);
	int res = vsprintf(buf, fmt, ap);
	scr_printString(buf);
}

