#ifndef _hwclk_h_
#define _hwclk_h_

#include "hwcommon.h"

/*! Used internally */
hw_Drv* hwclk_ctor(uint8_t bus);
/*! Used internally */
void hwclk_dtor(hw_Drv* drv);

#define HWCLK_NUMTIMERS 8

/*!
 * Timer callback function.
 * \param cookie Data that was passed to hwclk_setTimer.
 * 
 * \return
 *	The callback should return true if it wishes to stay present and
 * active, or false it it wishes to be removed.
 * Returning false is useful to implement one-off timers.
 */
typedef bool (*hwclk_TimerFunc)(void* cookie);

/*!
 * Gets milliseconds elapsed since boot, as a 64 bits unsigned int number.
 *
 * \param outLow (Required) The lowest 32-bits.
 * \param outHigh (Required) The highest 32-bits.
 */
void hwclk_getMsSinceBoot(u32* outLow, u32 *outHigh);


/*!
 * Date and time as retrieved from the clock device
 */
typedef struct {
	int16_t year; //
	int16_t month; // 1-31
	int16_t dayOfWeek; // 0(Monday)-6
	int16_t day; // 1-31
	int16_t hour; // 0-23
	int16_t minute; // 0-59
	int16_t second; // 0-59
	int16_t millisecond; // 0-999
} DateTime;
/*
 * ! Retrieves the current system date and time
 */
void hwclk_getSystemTime(DateTime* outDT);

/*!
 * Starts the specified timer.
 *
 * \param timerNumber
 *	What timer to set
 * 
 * \param ms
 *	Timer countdown, in milliseconds.
 * 
 * \param autoReset
 *	If true, the timer will auto reset (restart), when it expires.
 * 
 * \param irqMode
 *	If true, an IRQ will be generated if the clock is allowed to generate IRQs.
 *	If false, no IRQ will be generated, but the status of the timer can still be
 *	queried with hwclk_getTimer
 */
void hwclk_startTimer(u32 timerNumber, u32 ms, bool autoReset, bool irqMode);

/*!
 * Adds a callback to a timer.
 *
 * \param timerNumber
 *	Timer to add the callback to.
 *
 * \param func
 *	Function to call when the timer expires. This function should return true
 * to stay registered, or false to be removed (one-shot callback).
 *
 * \param cookie
 *	Value to pass to the func.
 */
void hwclk_addCallback(u32 timerNumber, hwclk_TimerFunc func, void* cookie);

#endif
