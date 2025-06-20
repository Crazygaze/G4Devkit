#include "boot.h"

#include "hwcrt0.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "../kernel/mmu.h"

/*!
 * Holds temporary information required during the boot process
 * NOTE: We are assuming that a character is always 2 bytes (char and colour) 
 */
typedef struct BootUIInfo
{
	// Screen buffer
	u16* buf;
	u16* end;
	int width;
	int height;
	int stride;
	u16 currColour;
	// Current write position
	u16* cursor;
} BootUIInfo;

static BootUIInfo uiInfo;

static u16* boot_ui_getXYPtr(int x, int y)
{
	return uiInfo.buf + (y * uiInfo.width) + x;
}

void boot_ui_init(void)
{
	defineZeroed(HwfSmallData, data);
	
	hw_hwf_0_4(HWBUS_SCR, 1, &data);	
	uiInfo.width= data.regs[1];
	uiInfo.height = data.regs[2];
	uiInfo.stride = uiInfo.width * data.regs[3];

	//
	// Remap the screen buffer to match do what the mmu expects
	//
	void* buf = mmu_allocIO(uiInfo.stride * uiInfo.height);
	krnassert(buf);
	data.regs[0] = (u32)buf;
	krnverify(hw_hwf_1_0(HWBUS_SCR, 2, &data) == HWERR_SUCCESS);
	uiInfo.buf = buf;
	
	uiInfo.cursor = boot_ui_getXYPtr(0,0);
	uiInfo.end = boot_ui_getXYPtr(0, uiInfo.height);
	
	boot_ui_setBkgColour(kTXTCLR_BLACK);
	boot_ui_setForeColour(kTXTCLR_WHITE);
}

void boot_ui_setBkgColour(TxtColour colour)
{
	uiInfo.currColour = (uiInfo.currColour & 0x8F00) | (colour << (8 + 4));
}

void boot_ui_setForeColour(TxtColour colour)
{
	uiInfo.currColour = (uiInfo.currColour & 0xF000) | (colour << 8);
}

void boot_ui_clearArea(int x1, int y1, int x2, int y2)
{
	u8* start = (u8*)boot_ui_getXYPtr(x1, y1);
	u8* end = (u8*)boot_ui_getXYPtr(x2, y2);
	memset(start, 0, (end - start) + 1);
}

void boot_ui_clear(void)
{
	// We print a space character (32) to clear the screen
	u16 ch = uiInfo.currColour | 32;
	
	u16* ptr = uiInfo.buf;
	int todo = uiInfo.width * uiInfo.height;
	while (todo--) {
		*ptr++ = ch;
	}
}

// Scrolls 1 line
static void boot_ui_scroll(int lines)
{
	if (lines == 0)
		return;

	memmove(boot_ui_getXYPtr(0, 0), boot_ui_getXYPtr(0, lines),
			(uiInfo.width * uiInfo.height * 2) - (lines * uiInfo.stride));

	boot_ui_clearArea(0, uiInfo.height - lines, uiInfo.width - 1,
					  uiInfo.height - 1);
}

static void boot_ui_checkScroll(const u16* end)
{
	if (uiInfo.cursor >= end) {
		boot_ui_scroll(1);
		uiInfo.cursor = boot_ui_getXYPtr(0, uiInfo.height - 1);
	}
}

static void boot_ui_printChar(uint8_t ch, int count, const u16* end)
{
	while(count--) {
		boot_ui_checkScroll(end);
		*uiInfo.cursor++ = 0x0F00 | ch;
	}
}

void boot_ui_printString(const char* str)
{
	u16* end = boot_ui_getXYPtr(uiInfo.width-1, uiInfo.height-1);
	while(*str)
	{
		switch(*str)
		{
		case 0x09: // TAB
			boot_ui_printChar(' ', 4, end);
			break;
		case '\n':
			uiInfo.cursor = boot_ui_getXYPtr(
				0, ((uiInfo.cursor - (u16*)uiInfo.buf) / uiInfo.width) + 1);
			boot_ui_checkScroll(end);
			break;
		case '\r':
			uiInfo.cursor = boot_ui_getXYPtr(
				0, (uiInfo.cursor - (u16*)uiInfo.buf) / uiInfo.width);
			break;
		case '\b':
			uiInfo.cursor--;
			if ((u32)uiInfo.cursor < (u32)uiInfo.buf)
				uiInfo.cursor = (u16*)uiInfo.buf;
			boot_ui_printChar(' ', 1, end);
			uiInfo.cursor--;
			break;
		default:
			boot_ui_printChar(*str, 1, end);
		}

		str++;
	}

}

void boot_ui_printf(const char* fmt, ...)
{
	char buf[256];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, 256, fmt, ap);
	boot_ui_printString(buf);
}

////////////////////////////////////////////////////////////////////////////////
// OS LOGO
////////////////////////////////////////////////////////////////////////////////

/*
Gray background
		txtui_setColour(&rootCanvas, kTXTCLR_BLACK, kTXTCLR_GRAY);
		char 176
		
Yellow shadow
		txtui_setColour(&rootCanvas, kTXTCLR_BLACK, kTXTCLR_BRIGHT_YELLOW);
		char 176

Full yellow
		txtui_setColour(&rootCanvas, kTXTCLR_BLACK, kTXTCLR_BRIGHT_YELLOW);
		char 219

*/

#define G 0x0800 // Gray
#define Y 0x0E00 // Yellow

#define ii G|176
#define aa Y|176
#define C0 Y|219
#define C3 Y|220
#define C4 Y|223
#define C1 Y|178
#define C2 Y|177
#define U0 Y|'_'

#define OSLOGO_WIDTH 65
#define OSLOGO_HEIGHT 8

const unsigned short oslogo[OSLOGO_WIDTH*OSLOGO_HEIGHT] =
{
//     A                   |   P                   |   C                   |  P                    |  U                                   O                       S
ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,
ii,ii,C0,C0,C0,C0,C0,C0,C0,ii,C0,C0,C0,C0,C0,C0,C0,ii,C0,C0,C0,C0,C0,C0,C0,ii,C0,C0,C0,C0,C0,C0,C0,aa,C0,C0,C0,ii,C0,C0,C0,ii,ii,ii,ii,ii,C0,C0,C0,C0,C0,C0,C0,ii,C0,C0,C0,C0,C0,C0,C0,ii,ii,ii,ii,
ii,ii,C1,C0,C0,ii,C0,C0,C0,aa,C1,C0,C0,ii,C0,C0,C0,aa,C1,C0,C0,aa,C0,C0,C0,aa,C1,C0,C0,aa,C0,C0,C0,aa,C1,C0,C0,aa,C0,C0,C0,aa,ii,ii,ii,ii,C1,C0,C0,aa,C0,C0,C0,aa,C1,C0,C0,aa,aa,aa,aa,aa,ii,ii,ii,
ii,ii,C1,C0,C0,C0,C0,C0,C0,aa,C1,C0,C0,C0,C0,C0,C0,aa,C1,C0,C0,aa,aa,aa,aa,aa,C1,C0,C0,C0,C0,C0,C0,aa,C1,C0,C0,aa,C0,C0,C0,aa,ii,ii,ii,ii,C1,C0,C0,aa,C0,C0,C0,aa,C2,C1,C0,C0,C0,C0,C0,aa,ii,ii,ii,
ii,ii,C2,C0,C0,aa,C0,C0,C0,aa,C2,C0,C0,aa,aa,aa,aa,aa,C2,C0,C0,aa,C0,C0,C0,aa,C2,C0,C0,aa,aa,aa,aa,aa,C2,C0,C0,aa,C0,C0,C0,aa,ii,ii,ii,ii,C2,C0,C0,aa,C0,C0,C0,aa,ii,aa,aa,aa,C0,C0,C0,aa,ii,ii,ii,
ii,ii,C2,C2,C1,aa,C2,C1,C0,aa,C2,C2,C1,aa,ii,ii,ii,ii,C2,C2,C1,C1,C0,C1,C0,aa,C2,C2,C1,aa,ii,ii,ii,ii,C2,C2,C1,C1,C0,C1,C0,aa,ii,ii,ii,ii,C2,C2,C1,C1,C0,C1,C0,aa,C2,C1,C0,C0,C0,C0,C0,aa,ii,ii,ii,
ii,ii,ii,aa,aa,aa,aa,aa,aa,aa,ii,aa,aa,aa,ii,ii,ii,ii,ii,aa,aa,aa,aa,aa,aa,aa,aa,aa,aa,aa,ii,ii,ii,ii,ii,aa,aa,aa,aa,aa,aa,aa,ii,ii,ii,ii,ii,aa,aa,aa,aa,aa,aa,aa,ii,aa,aa,aa,aa,aa,aa,aa,ii,ii,ii,
ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii
};

void boot_ui_displayOSLogo(void)
{
	u16* ptr = uiInfo.buf;
	const u16* src = &oslogo[0];
	int offset = (uiInfo.width - OSLOGO_WIDTH) / 2;
	for (int i = 0; i < OSLOGO_WIDTH * OSLOGO_HEIGHT; i++) {
		ptr[(i / OSLOGO_WIDTH) * uiInfo.width + (i % OSLOGO_WIDTH) +
			offset] = *src++;
	}
	
	uiInfo.cursor = boot_ui_getXYPtr(0, OSLOGO_HEIGHT);
}
