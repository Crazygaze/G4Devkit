/*******************************************************************************
* KEYBOARD driver
*******************************************************************************/

#include "hwkeyboard.h"
#include "kernel/kerneldebug.h"
#include "utilshared/bitset.h"
#include "appsdk/kernel_shared/process_shared.h"

#define KYB_DEBUG_EVENTS 0
#define KYB_MAX_CALLBACKS 4

typedef struct hw_kyb_EventFuncData
{
	hw_kyb_EventFunc func;
	void* cookie;
} hw_kyb_EventFuncData;

typedef struct hw_kyb_Drv {
	hw_Drv base;
	// Extra fields required by the device goes below...
	uint32_t numCallbacks;
	hw_kyb_EventFuncData callbacks[KYB_MAX_CALLBACKS];
	
	// Tells if modifier keys are pressed (e.g: Ctrl, Shift)
	union {
		uint32_t all;
		char bits[BITNSLOTS(32)];
	} flags;
} hw_kyb_Drv;

static hw_kyb_Drv drv;
static void hw_kyb_irqHandler(uint16_t reason, u32 data1, u32 data2);

hw_Drv* hw_kyb_ctor(hw_BusId busid)
{
	drv.base.irqHandler = hw_kyb_irqHandler;	
	hw_kyb_setIRQMode(TRUE);
	return &drv.base;
}

void hw_kyb_dtor(hw_Drv* drv)
{
}


void hw_kyb_clearBuffer(void)
{
	hw_hwiSimple0(HWBUS_KYB, HW_KYB_FUNC_CLEARBUFFER);
}

uint8_t hw_kyb_getNextEvent(uint8_t* key)
{
	hw_HwiData hwi;
	hwi.regs[0] = HWBUS_KYB;
	hwi.regs[1] = HW_KYB_FUNC_GETEVENT;
	hw_hwiFull(&hwi);
	*key = hwi.regs[2];
	return hwi.regs[1];
}

static void hw_kyb_updateModifier(int flag, uint8_t event)
{
	if (event==HW_KYB_EVENT_PRESSED) {
		BITSET(drv.flags.bits, flag);
	} else if (event==HW_KYB_EVENT_RELEASED) {
		BITCLEAR(drv.flags.bits, flag);
	}
}

static void hw_kyb_irqHandler(uint16_t reason, u32 data1, u32 data2)
{
	uint8_t key;
	uint8_t event;
	while((event = hw_kyb_getNextEvent(&key))) {
		
		if (key==KEY_CONTROL) {
			hw_kyb_updateModifier(0, event);
		} else if (key==KEY_SHIFT) {
			hw_kyb_updateModifier(1, event);
		}
#if KYB_DEBUG_EVENTS
		else if (key>=32 && key<=127) {
			KERNEL_DEBUG("Key '%c' : %d", key, event);
		} else {
			KERNEL_DEBUG("Key %d : %d", key, event);
		}
#endif

		for(int i=0; i<drv.numCallbacks; i++) {
			if (drv.callbacks[i].func(event, key, drv.flags.all,
				drv.callbacks[i].cookie))
				break;
		}

	}
}

void hw_kyb_addEventCallback(hw_kyb_EventFunc func, void* cookie)
{
	kernel_assert(drv.numCallbacks<KYB_MAX_CALLBACKS);
	drv.callbacks[drv.numCallbacks].func = func;
	drv.callbacks[drv.numCallbacks].cookie = cookie;
	drv.numCallbacks++;
}
