#include "hwscreen.h"
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

static HWScrInfo gScr;
static u16* gCursor;

#define HWSCR_FUNC_INFO 1
#define HWSCR_FUNC_MAP 2

static unsigned short* hwscr_getXYPtr(int x, int y)
{
	return (unsigned short*)(gScr.buffer + (y * (gScr.xres * 2) + x * 2));
}

void hwscr_getInfo(HWScrInfo* info)
{
	// Get the screen info
	HwfSmallData data;
	int res = hw_hwfsmall(HWBUS_SCR, HWSCR_FUNC_INFO, &data);
	always_assert(res == HWERR_SUCCESS);
	info->buffer = (char*)data.regs[0];
	info->xres = data.regs[1];
	info->yres = data.regs[2];
	info->charStride= data.regs[3];

	HWLOG_LOG("SCR:xres=%d,yres=%d,charstride=%d", info->xres, info->yres,
			  info->charStride);
}

void hwscr_init(void)
{
	hwscr_getInfo(&gScr);
	gCursor = hwscr_getXYPtr(0,0);
}

void hwscr_map(void* buffer)
{
	HwfSmallData data;
	data.regs[0] = (u32)buffer;
	int res = hw_hwfsmall(HWBUS_SCR, HWSCR_FUNC_MAP, &data);
	always_assert(res == HWERR_SUCCESS);
	gScr.buffer = buffer;
}

void hwscr_clear(void)
{
	memset(gScr.buffer, 0, gScr.xres * gScr.yres * 2);
	gCursor = hwscr_getXYPtr(0, 0);
}

void hwscr_clearArea(int x1, int y1, int x2, int y2)
{
	u8* start = (u8*)hwscr_getXYPtr(x1, y1);
	u8* end = (u8*)hwscr_getXYPtr(x2, y2);
	memset(start, 0, (end - start) + 1);
}

void hwscr_printCharAtXY(int x, int y, unsigned char ch)
{
	*hwscr_getXYPtr(x, y) = 0x0F00 | ch;
}

void hwscr_printStringAtXY(int x, int y, const char* str)
{
	u16* ptr = hwscr_getXYPtr(x, y);
	while (*str) {
		*ptr = 0xF00 | *str;
		ptr++;
		str++;
	}
}

int hwscr_printfAtXY(int x, int y, const char* fmt, ...)
{
	char buf[HWSCR_PRINTF_BUFSIZE];
	va_list ap;
	va_start(ap, fmt);
	int res = vsnprintf(buf, HWSCR_PRINTF_BUFSIZE, fmt, ap);
	hwscr_printStringAtXY(x, y, buf);
	return res;
}

void hwscr_scroll(int lines)
{
	if (!lines)
		return;

	memmove(hwscr_getXYPtr(0, 0), hwscr_getXYPtr(0, lines),
			HWSCR_BUFFERSIZE - (lines * HWSCR_XRES * HWSCR_CHARSTRIDE));

	hwscr_clearArea(0, HWSCR_YRES - lines, HWSCR_XRES, HWSCR_YRES - 1);
}

static void hwscr_checkScroll(const u16* end)
{
	if (gCursor >= end) {
		hwscr_scroll(1);
		gCursor = hwscr_getXYPtr(0, gScr.yres - 1);
	}
}

static void hwscr_printStringHelper(unsigned char ch, int count, const u16* end)
{
	while (count--) {
		hwscr_checkScroll(end);
		*gCursor++ = 0x0F00 | ch;
	}
}

void hwscr_printString(const char* str)
{
	u16* end = hwscr_getXYPtr(gScr.xres - 1, gScr.yres - 1);
	
	while (*str) {
	
		switch (*str) {
		case 0x09: // TAB
			hwscr_printStringHelper(' ', 4, end);
			break;
		case '\n':
			gCursor = hwscr_getXYPtr(
				0, ((gCursor - (u16*)gScr.buffer) / gScr.xres) + 1);
			hwscr_checkScroll(end);
			break;
		case '\r':
			gCursor = hwscr_getXYPtr(0, (gCursor - (u16*)gScr.buffer) / gScr.xres);
			break;
		case '\b':
			gCursor--;
			if ((u32)gCursor < (u32)gScr.buffer)
				gCursor = (u16*)gScr.buffer;
			hwscr_printStringHelper(' ', 1, end);
			gCursor--;
			break;
		default:
			hwscr_printStringHelper(*str, 1, end);
		}

		str++;
	}
}

void hwscr_printf(const char* fmt, ...)
{
	char buf[256];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, 256, fmt, ap);
	hwscr_printString(buf);
}

