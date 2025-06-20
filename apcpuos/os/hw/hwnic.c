#include "hwnic.h"
#include <string.h>

typedef struct {
	hw_Drv base;
} hwnic_Drv;

static hwnic_Drv nicDrv;

#define HWNICFUNC_GET_ID 1
//#define HWNICFUNC_SND 2
#define HWNICFUNC_RCV 3
#define HWNICFUNC_QUERY_BUFFERS 4
#define HWNICFUNC_QUERY_OUTSTATS 5
#define HWNICFUNC_QUERY_INSTATS 6
#define HWNICFUNC_SETMODE 7
#define HWNICFUNC_GETMODE 8

void hwnic_handler(void)
{
	static int count;
	count++;
	hw_touch(nicDrv.base.bus);
	// #TODO Implement what's needed for buffering, etc.
}

hw_Drv* hwnic_ctor(uint8_t bus)
{
	nicDrv.base.handler = hwnic_handler;
	return &nicDrv.base;
}

void hwnic_dtor(hw_Drv* drv)
{
	memset(drv, 0, sizeof(nicDrv));
}

void hwnic_setIRQMode(bool irqOnSend, bool irqOnReceive)
{
	defineZeroed(HwfSmallData, data);
	if (irqOnSend)
		data.regs[0]  = 2;
	if (irqOnReceive)
		data.regs[0] |= 1;
		
	hw_hwf_0_1(HWBUS_NIC, HWNICFUNC_SETMODE, &data);
}

