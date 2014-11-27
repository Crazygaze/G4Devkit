/*******************************************************************************
* Clock driver
*******************************************************************************/

#include "hwclock.h"
#include "utilshared/dynamicarray.h"
#include "utilshared/misc.h"
#include "kernel/kerneldebug.h"


typedef struct TimerFuncData {
	hw_clk_TimerFunc func;
	void* userdata;
} TimerFuncData;

ARRAY_TYPEDECLARE_SIMPLE(TimerFuncData)
ARRAY_TYPEDEFINE_SIMPLE(TimerFuncData)

typedef struct Timer {
	array_TimerFuncData funcs;
} Timer;

typedef struct hw_clk_Drv {
	hw_Drv base;
	// Extra fields required by the device goes below...
	Timer timers[HW_CLK_NUMTIMERS];
} hw_clk_Drv;

double hw_clk_currSecs;
static hw_clk_Drv driver;
static void hw_clk_irqHandler(uint16_t reason, u32 data1, u32 data2);

hw_Drv* hw_clk_ctor(hw_BusId busid)
{
	driver.base.irqHandler = hw_clk_irqHandler;
	
	// Create timer arrays
	for(int i=0; i<HW_CLK_NUMTIMERS; i++) {
		array_TimerFuncData_create(&driver.timers[i].funcs,0);
	}
	
	return &driver.base;
}

void hw_clk_dtor(hw_Drv* drv)
{
	for(int i=0; i<HW_CLK_NUMTIMERS; i++) {
		array_TimerFuncData_destroy(&driver.timers[i].funcs);
	}
}

void hw_clk_startTimer(uint32_t timerNumber, uint32_t ms, bool autoReset
	, bool irqMode)
{
	hw_hwiSimple2(
		HWBUS_CLK, HW_CLK_FUNC_SETTIMER,
		((uint32_t)autoReset<<31) | ((uint32_t)irqMode<<30) | timerNumber,
		ms);
}

void hw_clk_addCallback(uint32_t timerNumber, hw_clk_TimerFunc func
	, void* userdata)
{
	kernel_assert(timerNumber<HW_CLK_NUMTIMERS);
	TimerFuncData f;
	f.func = func;
	f.userdata = userdata;
	array_TimerFuncData_pushVal(&driver.timers[timerNumber].funcs, f);
}

/*
reason - 1 bit per timer, that tells if that timer finished
*/
static void hw_clk_irqHandler(uint16_t reason, u32 data1, u32 data2)
{
	for (int t=0; t<HW_CLK_NUMTIMERS; t++) {
		if (ISBIT_SET(data1, t)) {
			Timer* timer = &driver.timers[t];
			for(int f=0; f!=timer->funcs.size;) {
				// If the callback returns TRUE, we keep it, otherwise, we
				// remove it
				if (timer->funcs.data[f].func(timer->funcs.data[f].userdata)) {
					f++;
				} else {
					array_removeAtGeneric((Array_generic*)(&timer->funcs), f);
				}
			}
		}
	}
}


