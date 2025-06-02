#include "hwclk.h"
#include "utils/dynamicarray.h"
#include "utils/bitops.h"

#define TIMERMASK 0x7

// #define HWCLK_FUNC_GET_TIMESINCEBOOT 1
#define HWCLK_FUNC_GET_STATE 2
#define HWCLK_FUNC_READ_TIMER 3
#define HWCLK_FUNC_SET_TIMER 4
#define HWCLK_FUNC_GET_SYSTEMTIME 5

typedef struct hwclk_TimerFuncData {
	hwclk_TimerFunc func;
	void* userdata;
} hwclk_TimerFuncData;

ARRAY_TYPEDECLARE_SIMPLE(hwclk_TimerFuncData)
ARRAY_TYPEDEFINE_SIMPLE(hwclk_TimerFuncData)

typedef struct hwclk_Timer {
	Array_hwclk_TimerFuncData funcs;
} hwclk_Timer;

typedef struct {
	hw_Drv base;
	hwclk_Timer timers[HWCLK_NUMTIMERS];
} hwclk_Drv;

static hwclk_Drv clkDrv;

#pragma dontwarn 323
void hwclk_handler(void)
{
	HwfSmallData data;
	hw_hwf_0_2(HWBUS_CLK, HWCLK_FUNC_GET_STATE, &data);
	uint32_t timersState = data.regs[0];
	//uint32_t irqState = data.regs[1];

	for(int t=0; t<HWCLK_NUMTIMERS; t++) {
		if (!ISBITSET(timersState, t))
			continue;

		HW_VER("Timer %d tick", t);

		hwclk_Timer* timer = &clkDrv.timers[t];
		for(int f = 0; f != timer->funcs.size; ) {
			hwclk_TimerFuncData* fdata = &timer->funcs.data[f];
			// if the  callback returns true, we keep it, otherwise we remove it
			if (fdata->func(fdata->userdata))
			{
				f++;
			}
			else
			{
				array_hwclk_TimerFuncData_removeAt(&timer->funcs, f);
			}
		}
	}
}
#pragma popwarn

hw_Drv* hwclk_ctor(uint8_t bus)
{
	// initialize the data for each timer
	for (int i = 0; i < HWCLK_NUMTIMERS; i++) {
		// NOTE: Leaving initial capacity at 0, so we don't allocate any memory
		// until we need the timer.
		array_hwclk_TimerFuncData_create(&clkDrv.timers[i].funcs, 0);
	}
	
	clkDrv.base.handler = hwclk_handler;
	
	return &clkDrv.base;
}

void hwclk_dtor(hw_Drv* drv)
{
	// Free any resources used by the timers
	for (int i = 0; i < HWCLK_NUMTIMERS; i++) {
		array_hwclk_TimerFuncData_destroy(&clkDrv.timers[i].funcs);
	}
	
	memset(drv, 0, sizeof(clkDrv));
}

void hwclk_getMsSinceBoot(u32* outLow, u32* outHigh)
{
	krnassert(outLow && outHigh);
	HwfSmallData data;
	hw_hwf_0_2(HWBUS_CLK, HWCLK_FUNC_GET_TIMESINCEBOOT, &data);
	*outLow = data.regs[0];
	*outHigh= data.regs[1];
}

void hwclk_startTimer(u32 timerNumber, u32 ms, bool autoReset, bool irqMode)
{
	krnassert(timerNumber < HWCLK_NUMTIMERS);

	HwfSmallData data = { 0 };
	data.regs[0] =  (timerNumber & TIMERMASK);
	if (autoReset)
		data.regs[0] |= 1 << 31;
	if (irqMode)
		data.regs[0] |= 1 << 30;
		
	data.regs[1] = ms;
	
	hw_hwf_2_0(HWBUS_CLK, HWCLK_FUNC_SET_TIMER, &data);
}

void hwclk_addCallback(u32 timerNumber, hwclk_TimerFunc func, void* userdata)
{
	krnassert(timerNumber < HWCLK_NUMTIMERS);
	hwclk_TimerFuncData f;
	f.func = func;
	f.userdata = userdata;
	array_hwclk_TimerFuncData_pushPtr(&clkDrv.timers[timerNumber].funcs, &f);
	
	f.func = NULL;
}

void hwclk_getSystemTime(DateTime* outDT)
{
	HwfSmallData data = { 0 };
	hw_hwf_0_4(HWBUS_CLK, HWCLK_FUNC_GET_SYSTEMTIME, &data);
	memcpy(outDT, &data.regs, sizeof(*outDT));
}

