#include "hwnic.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define HWNICFUNC_GET_ID 1
//#define HWNICFUNC_SND 2
#define HWNICFUNC_RCV 3
#define HWNICFUNC_QUERY_BUFFERS 4
#define HWNICFUNC_QUERY_STATS 5

int hwnic_sendDebug(const char* str)
{
	HwfSmallData hwf;
	hwf.regs[0] = 0; // Destination id (0 is the debug destination)
	hwf.regs[1] = (int)str;
	hwf.regs[2] = strlen(str) + 1;
	return hw_hwfsmall(HWBUS_NIC, HWNICFUNC_SND, &hwf);
}

int hwnic_sendDebugV(const char* fmt, ...)
{
	va_list ap;
	char buf[256];
	va_start(ap, fmt);
	vsnprintf(buf, 256, fmt, ap);
	return hwnic_sendDebug(buf);
}

