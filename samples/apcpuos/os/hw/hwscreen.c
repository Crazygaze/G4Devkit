/*******************************************************************************
 * Screen hardware device driver
*******************************************************************************/

#include "hwscreen.h"
#include "boot/boot.h"
#include "kernel/kerneldebug.h"
#include "hw/hwcpu.h"
#include <stdlib_shared.h>
#include <stdio_shared.h>
#include "kernel/kernel.h"
#include "appsdk/kernel_shared/txtui_shared.h"
#include "kernel/oslogo.h"

typedef struct hw_scr_Drv {
	hw_Drv base;
	TxtCanvas canvas;
	uint16_t* currentBuffer;
	uint32_t buffersize;
	uint8_t bytesperchar;
} hw_scr_Drv;

static unsigned short hw_scr_colour = kTXTCLR_WHITE<<8;

static hw_scr_Drv driver;

static void hw_scr_initInfo(void)
{
	hw_HwiData h;
	h.regs[0] = HWBUS_SCR;
	h.regs[1] = HW_SCR_FUNC_GETINFO;
	hw_hwiFull(&h);
	driver.canvas.width = h.regs[2];
	driver.canvas.height = h.regs[3];
	driver.canvas.stride = driver.canvas.width;
	driver.bytesperchar = h.regs[4];
	driver.buffersize =
		driver.canvas.width*driver.canvas.height*driver.bytesperchar;
	
	driver.canvas.data = (void*)((u32)ramAmount - (u32)driver.buffersize);	
}

uint32_t hw_scr_getBufferSize(void)
{
	if (!driver.buffersize) {
		hw_scr_initInfo();
	}
	return driver.buffersize;
}

hw_Drv* hw_scr_ctor(hw_BusId bus)
{
	// Map buffer
	hw_scr_initInfo();

	txtui_init(&driver.canvas);
	hw_scr_mapBuffer(rootCanvas.data);

	// Clear screen	
	txtui_clear(&rootCanvas);
	displayOSLogo();
	krn_bootLog("Screen initialized at 0x%X\n", driver.currentBuffer);
	
	return (hw_Drv*)&driver;
}

void hw_scr_dtor(hw_Drv* drv)
{
}

void hw_scr_mapBuffer(void* buffer)
{
	if (driver.currentBuffer==buffer)
		return;

	hw_hwiSimple1(HWBUS_SCR, HW_SCR_FUNC_MAP, (uint32_t)buffer);
	driver.currentBuffer = buffer;
}

// TODO : Remove this, since each application will have its own screen buffer
void* hw_scr_getScreenBuffer(void)
{
	return driver.currentBuffer;
}

int hw_scr_getScreenXRes(void)
{
	return driver.canvas.width;
}

int hw_scr_getScreenYRes(void)
{
	return driver.canvas.height;
}
