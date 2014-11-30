
#include "common.h"
#include <stdlib.h>
#include <string.h>

//
// We can have up to 128 devices, but because proper support for that in this
// sample would require some kind of screen scrolling, we are keeping things
// simple by only trying to find a limited number of devices
#define MAX_DEVICES 20

int line=1;

#define HWIERR_SUCCESS 0
#define HWIERR_NODEVICE 0x80000001
#define HWIERR_INVALIDFUNCTION 0x80000002
#define HWIERR_INVALIDMEMORYADDRESS 0x80000003

#define HWIFUNC_ID 0x80000000
#define HWIFUNC_DESCRIPTION 0x80000001
#define HWIFUNC_UUID 0x80000002

static int x;
static int y;

/*! This converts a Device/Manufacturer ID to a string
The byte order is different, so this is necessary.
The ID byte order will change in the future, so there won't be a need to reverse
the bytes order
*/
const char* idToString(int id, char* dst)
{	
	memcpy(dst, &id, 4);
	dst[4] = 0;
	return dst;
}

void appMain(void)
{
	initCommon();
	
	x = 0;
	y = 0;
	printfAtXY(x,y,"Hardware Device Enumeration example");
	y++;
	
	for(int bus=0; bus<MAX_DEVICES; bus++) {
		
		x = 0;
		x += printfAtXY(x,++y, "Bus %d: ", bus);
		
		//
		// Get Device ID
		// We also use this to detect if a device exists at that bus
		HwiData data;
		int res = hwiCall(bus, HWIFUNC_ID, &data);
		
		char deviceId[5];
		char manufacturerId[5];
		if (res==HWIERR_SUCCESS) {
			// Print the Device ID information
			x += printfAtXY(x,y,"FOUND: ID=%s, Version=%d, Manuf.=%s",
				idToString(data.regs[0], deviceId),
				data.regs[1],
				idToString(data.regs[2], manufacturerId));				
			
			//
			// Get the device description
			hwiCall(bus, HWIFUNC_DESCRIPTION, &data);
			char desc[4*4+1];
			memset(desc, 0, sizeof(desc));
			memcpy(desc, &data.regs[0], 4*4);
			printfAtXY(x,y,", Desc=%s", desc);		
		} else {
			printfAtXY(x,y,"NO DEVICE");
		}
	}
	
	loopForever();
}
