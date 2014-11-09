/*******************************************************************************
* Clock driver
*******************************************************************************/

#ifndef _APCPU_HWCLOCK_H_
#define _APCPU_HWCLOCK_H_

#include "hwcommon.h"

#define HW_CLK_FUNC_READBOOTMS 0
#define HW_CLK_FUNC_READTIMER 1
#define HW_CLK_FUNC_SETTIMER 2

#define HW_CLK_NUMTIMERS 8

/*
Timer callback functions.
\param userdata Data passed when setting the timer function
\return
	Should return TRUE if the callback should stay active.
	Should return FALSE if the callback should be removed
*/
typedef bool (*hw_clk_TimerFunc)(void* userdata);

hw_Drv* hw_clk_ctor(hw_BusId);
void hw_clk_dtor(hw_Drv* drv);

/*!
 * Returns how many milliseconds the system as been running since the last
 * reset.
 * Note: Only returns the lowest 32 bits, since the internal counter is 64 bits.
 */
#define hw_clk_getRunningTimeMs32() \
	hw_hwiSimple0(HWBUS_CLK, HW_CLK_FUNC_READBOOTMS)

/*!
 * Returns how many seconds the system as been running since the last reset.
 */
#define hw_clk_getRunningTimeSeconds() \
	hw_hwiSimple0_Double(HWBUS_CLK, HW_CLK_FUNC_READBOOTMS)

/*! Starts the specified timer
 * \param timerNumber
 *		Timer to set
 * \param ms
 *		Interval in milliseconds
 * \param autoReset
 *		If true, the timer will automatically restart every time it is
 *		triggered.
 * \param irqMode
 *		If true, an IRQ will be generated. If false then an explicit HWI call
 *		Must be made to check the countdown.
 */
void hw_clk_startTimer(uint32_t timerNumber, uint32_t ms, bool autoReset
	, bool irqMode);

/*! Adds a new callback function to the specified timer
 *
 */
void hw_clk_addCallback(uint32_t timerNumber, hw_clk_TimerFunc func
	, void* userdata);

/*! Current system
This is updated every time an interrupt occurs
*/
extern double hw_clk_currSecs;

#endif
