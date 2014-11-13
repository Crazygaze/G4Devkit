/*******************************************************************************
 * Code for debugging kernel stuff
 ******************************************************************************/

#include "boot/boot.h"
#include "kerneldebug.h"
#include "hw/hwcpu.h"
#include "hw/hwscreen.h"
#include "hw/hwnic.h"
#include <stdarg_shared.h>
#include <stdio_shared.h>
#include <string_shared.h>
#include "oslogo.h"
#include "appsdk/kernel_shared/txtui_shared.h"

static int krn_bootLog_x;
static int krn_bootLog_y = OSLOGO_HEIGHT;

int krn_debugOutput(const char* fmt, ...)
{
	va_list ap;
	char buf[256];
	char* out = &buf[0];
	va_start(ap, fmt);	
	vsprintf(buf, fmt, ap);
	return hw_nic_sendDebugV("kernel: %s", buf);
}

/*! Scrolls up 1 line
*/
static void krn_checkScroll(void)
{
	if (krn_bootLog_y < rootCanvas.height)
		return;		
	memmove(rootCanvas.data, rootCanvas.data+rootCanvas.stride,
		txtui_getBuferSize(&rootCanvas) -
		(rootCanvas.width*2)*(krn_bootLog_y-rootCanvas.height+1));
	krn_bootLog_y = rootCanvas.height-1;
}

void krn_bootLog(const char* fmt, ...)
{
	va_list ap;
	char buf[80];
	char* out = &buf[0];
	va_start(ap, fmt);	
	vsprintf(buf, fmt, ap);
	krn_checkScroll();
	krn_bootLog_x += txtui_printAtXY(&rootCanvas, krn_bootLog_x,
		krn_bootLog_y, buf);
	char* p=buf;
	while(*p) {
		if (*p == '\n') {
			krn_bootLog_x = 0;
			krn_bootLog_y++;
		}
		p++;
	}
}

// Actual panic implementation
void krn_PanicImpl(const char* file, int line, const char* fmt, ...)
{
	krn_bootLog_x = 0;
	krn_bootLog_y = 0;
	
	char tmpbuf[512];

	hw_cpu_disableIRQ();

	// Remap the screen to show the kernel buffer
	hw_scr_mapBuffer(rootCanvas.data);
	txtui_setBackgroundColour(&rootCanvas, kTXTCLR_BLUE);
	txtui_setForegroundColour(&rootCanvas, kTXTCLR_BRIGHT_WHITE);
	txtui_clear(&rootCanvas);
	
	va_list ap;
	va_start(ap, fmt);
	vsprintf(tmpbuf, fmt, ap);
	va_end(ap);
	krn_bootLog("KERNEL PANIC\n");
	krn_bootLog(tmpbuf);

#if DEBUG_NICLOGPANIC
	hw_nic_sendDebugV("KERNEL PANIC: %s", tmpbuf);
#endif

	krn_bootLog("\n");
	krn_bootLog("File: %s\n", file);
	krn_bootLog("Line: %d\n", line);

#if DEBUG_NICLOGPANIC
	hw_nic_sendDebugV("File: %s", file);
	hw_nic_sendDebugV("Line: %d", line);
#endif

	// Infinite loop
	while(TRUE) {
		hw_cpu_hlt();
	}
}

void krn_forceCrash(void)
{
	unsigned char* ptr = (unsigned char*)0x4;
	*ptr = 0;
}

