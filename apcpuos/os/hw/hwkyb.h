#ifndef _hwkyb_h_
#define _hwkyb_h_

#include "hwcommon.h"

/*! Used internally */
hw_Drv* hwkyb_ctor(uint8_t bus);
/*! Used internally */
void hwkyb_dtor(hw_Drv* drv);

typedef enum KeyEvent
{
	kKeyEvent_None,
	kKeyEvent_Press,
	kKeyEvent_Release,
	kKeyEvent_Typed
} KeyEvent;

/*!
 * Clears the keyboard buffer
 */
void hwkyb_clear(void);

/*!
 * Retrieves the next keyboard event.
 *
 * \param key
 *	The key code, if the return value was not `kKeyEvent_None`
 *
 * \return
 *	The event type. If there was no event pending, `kKeyEvent_None` is returned
 */
KeyEvent hwkyb_getNext(uint8_t* key);


/*!
 * Checks if the specified key currently being pressed
 * \param key The key to check. If 0, then it checks if any key is pressed. 
 */
bool hwkyb_isPressed(uint8_t key);

#endif

