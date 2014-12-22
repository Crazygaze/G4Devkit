#ifndef _hwclock_h_
#define _hwclock_h_

#include "hwcommon.h"

/*! Gets the number of seconds the system as been running since boot
*/
double clk_getRunningTimeSeconds(void);

/*! Gets the number of milliseconds the system has been running since boot
* Only the lower 32 bits are available
*/
u32 clk_getRunningTimeMilliseconds(void);

/*! Pauses the program for the specified milliseconds
*/
void clk_pauseMS(int ms);

/*! Checks how many milliseconds left on the specified timer
*/
u32 clk_readCountdownTimer(int timer);

/*! Sets a countdown timer
\param timer
	Timer to set
\param ms
	Milliseconds to set the countdown timer to
\param autoReset
	If true, the willl restart whenever the countdown reaches 0
\param generateIRQ
	If true, an interrupt will occur when the countdown reaches 0
*/
void clk_setCountdownTimer(int timer, u32 ms, bool autoReset, bool generateIRQ);

/*! Sets a timer to not do anything
*/
void clk_disableTimer(int timer);

//
// Interrupt reasons for the Clock hardware device
//
#define HWCLOCK_INTERRUPT_TIMER 0
#define HWCLOCK_INTERRUPT_MAX 1

// How many timers the hardware supports
#define HWCLOCK_NUMTIMERS 8

#endif
