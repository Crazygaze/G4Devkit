/*******************************************************************************
* Common code for hardware device drivers
*******************************************************************************/

#include "hwcommon.h"
#include "kernel/kerneldebug.h"
#include "hwcpu.h"
#include "hwclock.h"
#include "hwscreen.h"
#include "hwnic.h"
#include "hwkeyboard.h"
#include "hwdisk.h"


hw_Drv* hw_drivers[HWBUS_DEFAULTDEVICES_MAX];

#define HWID(c1,c2,c3,c4) \
	(uint32_t)c1<<24 | (uint32_t)c2<<16 | (uint32_t)c3<<8 | (uint32_t)c4

typedef struct hw_DrvCreator {
	uint32_t id; // device hardware ID
	hw_CreateDriver ctor;
	hw_Destroydriver dtor;
} hw_DrvCreator;
static hw_DrvCreator createfuncs[] = {

 { HWID('C','P','U','0'), &hw_cpu_ctor, &hw_cpu_dtor},
 { HWID('C','L','K','0'), &hw_clk_ctor, &hw_clk_dtor},
 { HWID('S','C','R','0'), &hw_scr_ctor, &hw_scr_dtor},
 { HWID('N','I','C','0'), &hw_nic_ctor, &hw_nic_dtor},
 { HWID('K','Y','B','0'), &hw_kyb_ctor, &hw_kyb_dtor},
 { HWID('D','K','C','0'), &hw_dkc_ctor, &hw_dkc_dtor},
 { 0, NULL, NULL }
};

static hw_DrvCreator* getCreator(uint32_t hwid)
{
	hw_DrvCreator* p = createfuncs;
	while(p->id && p->id!=hwid) {
		p++;
	}
	
	return p->id ? p : NULL;
}

void hw_initAll(void)
{
	// Initialize screen first, so we can display boot messages
	{
		hw_DrvCreator* creator = getCreator(HWID('S','C','R','0'));
		hw_Drv* drv = creator->ctor(HWBUS_SCR);
		hw_drivers[HWBUS_SCR] = drv;
		// Initialize our fields
		drv->bus = HWBUS_SCR;
		drv->dtor = creator->dtor;
	}
	
	// Initialize the rest of the default devices
	for(int bus=0; bus<HWBUS_DEFAULTDEVICES_MAX; bus++) {
		hw_HwiData h;
		h.regs[0] = bus;
		h.regs[1] = HWDEFAULT_FUNC_QUERYINFO;
		HWERROR err = hw_hwiFull(&h);
		
		if (err==HWERR_DEFAULT_DEVICENOTFOUND) {
			continue;
		} else if (err!=HWERR_SUCCESS) {
			krn_bootLog("BUS %d: Can't initialize (error 0x%X)\n", bus, err);
			continue;
		}
		
		uint32_t id = h.regs[1];
		uint32_t version = h.regs[2];
		
		krn_bootLog("BUS %d: %c%c%c%c v0x%X...",
			bus, id>>24, (id>>16)&0xFF, (id>>8)&0xFF, id&0xFF,
			version);

		if (hw_drivers[bus]) { // skip screen device
			krn_bootLog("Done\n");
			continue;
		}
		
		hw_DrvCreator* creator = getCreator(id);
		if (!creator) {
			krn_bootLog("No driver available\n");
			continue;
		}
		
		hw_Drv* drv = creator->ctor(bus);
		hw_drivers[bus] = drv;
		// Initialize our fields
		drv->bus = bus;
		drv->dtor = creator->dtor;
		krn_bootLog("Done\n");		
	}
	
}
