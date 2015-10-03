/*******************************************************************************
* NIC (Network Interface Card) driver
*******************************************************************************/

#include "hwnic.h"
#include <string_shared.h>
#include <stdio_shared.h>

typedef struct hw_nic_Drv {
	hw_Drv base;
	// Extra fields required by the device goes below...
} hw_nic_Drv;

static hw_nic_Drv driver;
static void hw_nic_irqHandler(u32 reason, u32 data1, u32 data2);

hw_Drv* hw_nic_ctor(hw_BusId busid)
{
	driver.base.irqHandler = hw_nic_irqHandler;
	return &driver.base;
}

void hw_nic_dtor(hw_Drv* drv)
{
}

HWERROR hw_nic_sendDebug(const char* str)
{
	hw_HwiData hwi;
	hwi.regs[0] = 0; // Destination id (0 is a debug destination)
	hwi.regs[1] = (unsigned int)str;
	hwi.regs[2] = strlen(str)+1;
	return hw_hwiFull(HWBUS_NIC,HW_NIC_FUNC_SEND, &hwi);
}

HWERROR hw_nic_sendDebugV(const char* fmt, ...)
{
	char buffer[1024];
	va_list ap;
	va_start(ap, fmt);
	vsprintf(buffer, fmt, ap);
	return hw_nic_sendDebug(buffer);
}

HWERROR hw_nic_sendDebugVA(const char* fmt,  int* valist)
{
	char buffer[1024];
	char *b = buffer;
	va_list ap;
	ap.argptr = (char*)valist;
	vsprintf(buffer, fmt, ap);
	return hw_nic_sendDebug(buffer);
}

static void hw_nic_irqHandler(u32 reason, u32 data1, u32 data2)
{
}
