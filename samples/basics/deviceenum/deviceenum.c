
#include "common.h"
#include "hwcommon.h"
#include "hwscreen.h"
#include <stdlib.h>
#include <string.h>

//
// We can have up to 128 devices, but because proper support for that in this
// sample would require some kind of screen scrolling, we are keeping things
// simple by only trying to find a limited number of devices
#define MAX_DEVICES 20

int line=1;


static int x;
static int y;

/*! This converts a Device/Manufacturer ID to a string.
*/
const char* idToString(int id, char* dst)
{	
	memcpy(dst, &id, 4);
	dst[4] = 0;
	return dst;
}

void appMain(void)
{
	scr_init();
	
	x = 0;
	y = 0;
	scr_printfAtXY(x,y,"Hardware Device Enumeration example");
	y++;
	
	for(int bus=0; bus<MAX_DEVICES; bus++) {
		
		x = 0;
		x += scr_printfAtXY(x,++y, "Bus %d: ", bus);
		
		//
		// Get Device ID
		// We also use this to detect if a device exists at that bus
		HwiData data;
		int res = hwiCall(bus, HWIFUNC_ID, &data);
		
		char deviceId[5];
		char manufacturerId[5];
		if (res==HWIERR_SUCCESS) {
			// Print the Device ID information
			x += scr_printfAtXY(x,y,"FOUND: ID=%s, Version=%d, Manuf.=%s",
				idToString(data.regs[0], deviceId),
				data.regs[1],
				idToString(data.regs[2], manufacturerId));				
			
			//
			// Get the device description
			hwiCall(bus, HWIFUNC_DESCRIPTION, &data);
			char desc[4*4+1];
			memset(desc, 0, sizeof(desc));
			memcpy(desc, &data.regs[0], 4*4);
			scr_printfAtXY(x,y,", Desc=%s", desc);		
		} else {
			scr_printfAtXY(x,y,"NO DEVICE");
		}
	}
	
	loopForever();
}
