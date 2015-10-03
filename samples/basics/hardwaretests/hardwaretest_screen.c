#include "hardwaretest_screen.h"
#include <hwscreen.h>
#include <hwclock.h>
#include <hwcpu.h>
#include <assert.h>
#include <string.h>

typedef struct ScreenDriver
{
	Driver base;
} ScreenDriver;

static ScreenDriver screenDriver;

static void hardwareTest_screen(void);
void hardwareTest_screen_init(DeviceTest* data)
{
	screenDriver.base.handlers = NULL;
	screenDriver.base.numHanders = 0;
	data->driver = &screenDriver.base;
	data->testFunc = &hardwareTest_screen;
}

static void hardwareTest_screen(void)
{
	scr_printf("Screen Tests\n");
	ScreenInfo info;
	memset(&info, 0, sizeof(info));
	scr_getInfo(&info);
	scr_printf("	Buffer=%u\n", info.buffer);
	scr_printf("	X resolution = %d\n", info.xres);
	scr_printf("	Y resolution = %d\n", info.yres);
	scr_printf("	Bytes per character = %d\n", info.bytesPerCharacter);

	check(info.xres==SCR_XRES);
	check(info.yres==SCR_YRES);
	check(info.bytesPerCharacter==2);
	
	// Check if the screen buffer is at the expected default address
	const u32 ram = cpu_getRamAmount();
	const u32 screenSize = info.xres*info.yres*info.bytesPerCharacter;
	check((u32)info.buffer==(ram-screenSize));

	// Test mapping the screen buffer to a different address
	const u32 newAddr = (u32)info.buffer - screenSize;
	const u32 originalAddr = (u32)info.buffer;
	scr_map((void*)newAddr);
	memset(&info, 0, sizeof(info));	
	scr_getInfo(&info);
	// Remap back to the original address, so we can see what's going on
	scr_map((void*)originalAddr);
	// Check if we indeed remapped earlier
	check((u32)info.buffer==newAddr);
	check(info.xres==SCR_XRES);
	check(info.yres==SCR_YRES);
}

