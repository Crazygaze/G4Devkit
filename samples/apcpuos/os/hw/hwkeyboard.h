/*******************************************************************************
* KEYBOARD driver
*******************************************************************************/
#ifndef _APCPU_HWKEYBOARD_H_
#define _APCPU_HWKEYBOARD_H_

#include "hwcommon.h"

//
// Device functions
//
#define HW_KYB_FUNC_CLEARBUFFER 0
#define HW_KYB_FUNC_GETEVENT 1
#define HW_KYB_FUNC_ISKEYPRESSED 2
#define HW_KYB_FUNC_IRQMODE 3

#define KW_KYB_EVENT_NONE 0
#define HW_KYB_EVENT_PRESSED 1
#define HW_KYB_EVENT_RELEASED 2
#define HW_KYB_EVENT_TYPED 3



/*
Key event callback functions.
\param userdata Data passed when setting the callback
\param event Event type
\param key Key that caused the event
\param consumed
	The callback should set this to TRUE if it handled the event, in
	which case th
\return
	Should return TRUE if the callback should stay active.
	Should return FALSE if the callback should be removed
*/
//typedef bool (*hw_kyb_timerfunc_st)(void* userdata);

hw_Drv* hw_kyb_ctor(hw_BusId);
void hw_kyb_dtor(hw_Drv* drv);


/*
 * Clears the event buffer
 */
void hw_kyb_clearBuffer(void);

/*
 * Retrieves the next keyboard event.
 * Returns KYBEVENT_NONE if no event
 * \param key Where the relevant key is saved
 */
uint8_t hw_kyb_getNextEvent(uint8_t* key);

/*
 * Checks if the specified key is pressed.
 * NOTE: Not all keys can be checked for this
 * \param key Key to check.
 * \return TRUE or FALSE
 */
#define hw_kyb_isKeyPressed(key) \
	((bool)hw_hwiSimple1(HWBUS_KYB, HW_KYB_FUNC_ISKEYPRESSED, key))

/* 
 * Enables/Disables IRQ mode
 * \state TRUE or FALSE
 * \return nothing
 */
#define hw_kyb_setIRQMode(state) \
	hw_hwiSimple1(HWBUS_KYB, HW_KYB_FUNC_IRQMODE, state)

#define HW_KYB_FLAG_CTRL  (1<<0)
#define HW_KYB_FLAG_SHIFT (1<<1)

typedef bool (*hw_kyb_EventFunc)(uint8_t evttype, uint8_t key, int flags
	, void* cookie);

/*
 * Handlers should return true if they consumed the event, or false if they
 * don't
 */
void hw_kyb_addEventCallback(hw_kyb_EventFunc func, void* cookie);

#endif
