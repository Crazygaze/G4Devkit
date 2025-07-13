#include "hwkyb.h"
#include "appsdk/os_shared/process_shared.h"

#define HWKYB_FUNC_CLEAR 1
#define HWKYB_FUNC_GETNEXTEVENT 2
#define HWKYB_FUNC_GETKEYSTATE 3


typedef struct {
	hw_Drv base;
} hwkyb_Drv;

static hwkyb_Drv kybDrv;

void hwkyb_handler(void)
{
	KeyEvent evt;
	u8 key;
	
	while((evt = hwkyb_getNext(&key))) {
		HW_VER("Key event: %u, %u('%c')",
			(u32)evt, (u32)key, key >= KEY_ASCII_FIRST ? (char)key : ' ');
	}
}

hw_Drv* hwkyb_ctor(uint8_t bus)
{
	kybDrv.base.handler = hwkyb_handler;
	return &kybDrv.base;
}

void hwkyb_dtor(hw_Drv* drv)
{
}

void hwkyb_clear(void)
{
	hw_hwf_0_0(HWBUS_KYB, HWKYB_FUNC_CLEAR);
}

KeyEvent hwkyb_getNext(uint8_t* key)
{
	HwfSmallData data;
	hw_hwf_0_2(HWBUS_KYB, HWKYB_FUNC_GETNEXTEVENT, &data);
	*key = data.regs[1];
	return data.regs[0];
}

bool hwkyb_isPressed(uint8_t key)
{
	HwfSmallData data;
	data.regs[0] = key;
	hw_hwf_1_1(HWBUS_KYB, HWKYB_FUNC_GETKEYSTATE, &data);
	return data.regs[0] ? true : false;
}
