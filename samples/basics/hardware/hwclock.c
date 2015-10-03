#include "hwclock.h"
#include <assert.h>

#define HWCLOCKFUNC_GET_TIMESINCEBOOT 0
#define HWCLOCKFUNC_GET_TIMER 1
#define HWCLOCKFUNC_SET_TIMER 2

double clk_getRunningTimeSeconds(void)
{
	HwiData data;
	int res = hwiCall(HWBUS_CLK, HWCLOCKFUNC_GET_TIMESINCEBOOT, &data);
	always_assert(res==HWIERR_SUCCESS);
	return data.fregs[0];
}

u32 clk_getRunningTimeMilliseconds(void)
{
	HwiData data;
	int res = hwiCall(HWBUS_CLK, HWCLOCKFUNC_GET_TIMESINCEBOOT, &data);
	always_assert(res==HWIERR_SUCCESS);
	return data.regs[0];
}

u32 clk_readCountdownTimer(int timer)
{
	HwiData data;
	data.regs[0] = timer;
	int res = hwiCall(HWBUS_CLK, HWCLOCKFUNC_GET_TIMER, &data);
	always_assert(res==HWIERR_SUCCESS);
	return data.regs[0];
}

void clk_setCountdownTimer(int timer, u32 ms, bool autoReset, bool generateIRQ)
{
	HwiData data;
	u32 r0 = timer;
	if (autoReset) r0 |= 1<<31;
	if (generateIRQ) r0 |= 1<<30;
	data.regs[0] = r0;
	data.regs[1] = ms;
	int res = hwiCall(HWBUS_CLK, HWCLOCKFUNC_SET_TIMER, &data);
	always_assert(res==HWIERR_SUCCESS);
}

void clk_disableTimer(int timer)
{
	clk_setCountdownTimer(timer, 0, false, false);
}

void clk_pauseMS(int ms)
{
	double end = clk_getRunningTimeSeconds() + (double)ms/1000.0f;
	// Loop until enough time as passed
	while(end > clk_getRunningTimeSeconds()){
	}
}
