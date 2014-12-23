#include <hwscreen.h>
#include "hwclock.h"
#include "hwcpu.h"
#include "hardwaretest_clock.h"

typedef struct ClockDriver
{
	Driver base;
} ClockDriver;

static ClockDriver clockDriver;
static int clock_timersIRQs[HWCLOCK_NUMTIMERS];

// Interrupt handler
void clock_handleTimer(u32 data0, u32 data1, u32 data2, u32 data3)
{
	for(int timer=0; timer<HWCLOCK_NUMTIMERS; timer++) {
		if (data0 & (1<<timer))
			clock_timersIRQs[timer]++;
	}
}

InterruptHandler clockHandlers[HWCLOCK_INTERRUPT_MAX] =
{
	&clock_handleTimer
};

static void hardwareTest_clock(void);
void hardwareTest_clock_init(DeviceTest* data)
{
	clockDriver.base.handlers = clockHandlers;
	clockDriver.base.numHanders =
		sizeof(clockHandlers)/sizeof(InterruptHandler);
	data->driver = &clockDriver.base;
	data->testFunc = &hardwareTest_clock;
}

static void hardwareTest_clock(void)
{
	return;
	
	scr_printf("CLOCK Tests\n");
	const int seconds = 2;
	scr_printf("	Pausing for %d seconds\n", seconds);
	clk_pauseMS(seconds*1000);
	
	//
	// Test timers initial state
	//
	scr_printf("	Check timers initial state\n");
	for(int timer=0; timer<HWCLOCK_NUMTIMERS; timer++) {
		u32 ms = clk_readCountdownTimer(timer);
		scr_printf("	Timer %d : %u ms. ", timer, ms);
		check_nl(ms==0);
	}
	doPause();	

	//
	// Test all timers with a oneshot countdown
	//
	scr_printf("	Test a oneshot countdown on all timers\n");
	const int countdownMs = 1000;
	for(int timer=0; timer<HWCLOCK_NUMTIMERS; timer++) {
		clk_setCountdownTimer(timer, countdownMs, false, false);
		u32 ms = clk_readCountdownTimer(timer);
		scr_printf("	Timer %d : %u ms. ", timer, ms);		
		check_nl(ms>0 && ms<=countdownMs);
	}
	
	// Make a pause to let all the timers finish
	scr_printf("	Pausing to give time for all timers to expire...\n");
	clk_pauseMS(countdownMs);
	for(int timer=0; timer<HWCLOCK_NUMTIMERS; timer++) {
		u32 ms = clk_readCountdownTimer(timer);
		scr_printf("	Timer %d : %u ms. ", timer, ms);
		check_nl(ms==0);
	}
	doPause();

	//
	// Test auto reset
	//
	scr_printf("	Testing auto timer auto resets\n");
	for(int timer=0; timer<HWCLOCK_NUMTIMERS; timer++) {
		clk_setCountdownTimer(timer, countdownMs, true, false);
	}
	// make a pause slightly bigger than the countdown time, to give enough
	// time for the timers to wrap around
	scr_printf("	Pausing to give time for all timers to wrap around...\n");
	clk_pauseMS(countdownMs+(countdownMs/10));
	for(int timer=0; timer<HWCLOCK_NUMTIMERS; timer++) {
		u32 ms = clk_readCountdownTimer(timer);
		scr_printf("	Timer %d : %u ms. ", timer, ms);
		check_nl(ms>0 && ms<countdownMs && clock_timersIRQs[timer]==0);
	}
	doPause();

	//
	// Test timer IRQs (without auto reset)
	//
	cpu_disableIRQ(); // Disable IRQs, so we can test queueing of IRQs
	scr_printf("	Testing timers IRQ (without auto reset and queued IRQs)\n");
	for(int timer=0; timer<HWCLOCK_NUMTIMERS; timer++) {
		clk_setCountdownTimer(timer, countdownMs, false, true);
	}
	scr_printf("	Pausing to give time for all timers to expire...\n");
	// Pausing a little longer to make sure we let all timers expire
	clk_pauseMS(countdownMs + countdownMs/2);
	cpu_enableIRQ(); // Allow interrupts now, since all timers should be done
	// Programmatically get the IRQ data from the interrupt queue, to test
	// handling IRQs programatically without interrupts
	IRQData irqData;
	while (cpu_getNextIRQ(&irqData, 255)) {
		checkf(irqData.busid==HWBUS_CLK, 
			"Expected to get IRQ for CLOCK, but got IRQ for %d\n", irqData.busid);
		clock_handleTimer( irqData.regs[0], irqData.regs[1],irqData.regs[2],
			irqData.regs[3]);	
	}
	// And check if all timers expired exactly once
	for(int timer=0; timer<HWCLOCK_NUMTIMERS; timer++) {
		u32 ms = clk_readCountdownTimer(timer);
		scr_printf("	Timer %d : %u ms, numIRQs=%d. ", timer, ms,
			clock_timersIRQs[timer]);
		check_nl(ms==0 && clock_timersIRQs[timer]==1);
	}
	doPause();
	
	//
	// Test timer IRQs with interrupts (instead of programatically), 
	// and with auto reset
	//
	scr_printf("	Testing timers IRQ (with auto reset)\n");
	for(int timer=0; timer<HWCLOCK_NUMTIMERS; timer++) {
		clk_setCountdownTimer(timer, countdownMs, true, true);
	}
	scr_printf("	Pausing to give time for all timers to wrap around...\n");
	// Pausing a bit longer, to let the timer wrap around a few times
	clk_pauseMS(countdownMs*2);
	for(int timer=0; timer<HWCLOCK_NUMTIMERS; timer++) {
		u32 ms = clk_readCountdownTimer(timer);
		scr_printf("	Timer %d : %u ms, numIRQs=%d. ", timer, ms,
			clock_timersIRQs[timer]);
		check_nl(ms<=countdownMs && clock_timersIRQs[timer]>1);
	}

	// Disable timers
	for(int timer=0; timer<HWCLOCK_NUMTIMERS; timer++) {
		clk_disableTimer(timer);
	}

}
