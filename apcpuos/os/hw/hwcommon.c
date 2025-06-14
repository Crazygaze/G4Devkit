#include "hwcommon.h"
#include "boot/boot.h"

#include "hwcpu.h"
#include "hwclk.h"
//#include "hwscreen.h"
//#include "hwkeyboard.h"
#include "hwnic.h"
//#include "hwdisk.h"

static hw_Drv* drivers[HWBUS_COUNT];

typedef struct {
	uint32_t id; // device hardware ID
	hw_DrvCtorFunc ctr;
	hw_DrvDtorFunc dtr;
} hw_DrvCreator;

#define HWID(c1,c2,c3,c4) \
	(uint32_t)c1 | (uint32_t)c2<<8 | (uint32_t)c3<<16 | (uint32_t)c4<<24

static hw_DrvCreator createFuncs[] = {
	{ HWID('C','P','U','0'), &hwcpu_ctor, &hwcpu_dtor},
	{ HWID('C','L','K','0'), &hwclk_ctor, &hwclk_dtor},
	//{ HWID('S','C','R','0'), &hw_scr_ctor, &hw_scr_dtor},
	//{ HWID('K','Y','B','0'), &hw_kyb_ctor, &hw_kyb_dtor},
	{ HWID('N','I','C','0'), &hwnic_ctor, &hwnic_dtor},
	//{ HWID('D','K','C','0'), &hw_dkc_ctor, &hw_dkc_dtor},
	{ 0, NULL, NULL }
};

/*!
 * Search for a driver for the specified device type
 */
static hw_DrvCreator* getCreator(u32 hwid)
{
	hw_DrvCreator* p = createFuncs;
	while (p->id && p->id != hwid) {
		p++;
	}

	return p->id ? p : NULL;
}

// Disable warning. Something that we still need to improve in the
// compiler:
//
// "target-warning: Impossible to figure out if calling a variadic
// function. Assuming it is not variadic. You should check for possible
// compiler bugs"
#pragma dontwarn 323
void hw_initAll(void)
{
	for(int bus = 0; bus < 26; bus++) {
		HwfSmallData data = { 0 };
		if (hw_hwf_0_3(bus, HWFUNC_ID, &data) == HWERR_NODEVICE) {
			continue;
		}
		
		//hwclk_spinMs(250);
		u32 id = data.regs[0];
		char idStr[4+1] = { 0 };
		memcpy(idStr, &id, 4);
		u32 version = data.regs[1];
		u32 manuId = data.regs[2];
		char manuIdStr[4+1] = { 0 };
		memcpy(manuIdStr, &manuId, 4);

		char desc[4*4+1] = { 0 };
		memset(&data, 0, sizeof(data));
		hw_hwf_0_4(bus, HWFUNC_DESCRIPTION, &data);
		memcpy(desc, &data.regs[0], 4*4);

		boot_ui_printf("BUS %d: %s-%s, %s, v0x%X\n",
			bus, idStr, desc, manuIdStr, version);
			
		hw_DrvCreator* creator = getCreator(id);
		
		if (!creator) {
			boot_ui_printf("  No compatible driver found\n");
			continue;
		}
		
		boot_ui_printf("  Initializing driver...\n");

		hw_Drv* drv = creator->ctr(bus);
		drv->bus = bus;
		drivers[bus] = drv;
		drv->dtor = creator->dtr;
	}
}

void hw_handleIRQ(unsigned bus)
{
	hw_Drv* drv = drivers[bus];
	if (!drv || drv->handler==NULL) {
		HW_ERR("No driver for device at bus %u to handle irq.", bus);
		hw_touch(bus);
		return;
	}
	
	drv->irqCount++;
	drv->handler();
}

#pragma popwarn
