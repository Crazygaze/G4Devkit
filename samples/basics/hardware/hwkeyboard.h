#ifndef _hwkeyboard_h_
#define _hwkeyboard_h_

#include "hwcommon.h"

//
// All non-ascii key codes
//
#define KEY_BACKSPACE 0x01
#define KEY_RETURN 0x02
#define KEY_INSERT 0x03
#define KEY_DELETE 0x04
#define KEY_UP 0x05
#define KEY_DOWN 0x06
#define KEY_LEFT 0x07
#define KEY_RIGHT 0x08
#define KEY_SHIFT 0x09
#define KEY_CONTROL 0x0A
#define KEY_TAB 0x0B
#define KEY_ASCII_FIRST 0x20
#define KEY_ASCII_LAST 0x7E


//
// Interrupt reasons for keyboard
//
#define HWKYB_INTERRUPT_EVENT 0
#define HWKYB_INTERRUPT_MAX 1

typedef enum KeyEvent
{
	kKeyEvent_None,
	kKeyEvent_Press,
	kKeyEvent_Release,
	kKeyEvent_Typed
} KeyEvent;


/*! Clears the keyboard event buffer
*/
void kyb_clearBuffer(void);

/*! Gets the next key event from the keyboard queue
* \param keyCode
*	Where you will get the keycode, if there was an event
* \param block
* 	If no event available, it will block waiting for one
* \return
*	0 if no event, otherwise the event type
*/
KeyEvent kyb_getNext(int* keyCode, bool block);

/*! Checks if the specified key is pressed
* \param keyCode
*	KeyCode to check. If 0, it will check if ANY key is pressed
* \note
*	Not all keyCode can be checked for "Pressed". Look at the documentation.
* \return
*	True if pressed, false it not pressed
*/
bool kyb_isPressed(int keyCode);

/*! Enables/disables IRQ mode
*/
void kyb_setIRQMode(bool enabled);

/*! Gets the next "Typed" event from the keyboard event queue.
* \note
*	All Pressed/Released events will be dropped until a Typed event is retrieved
*/
int kyb_getNextTyped(void);


/*! Pauses until a key is typed
* This will clear the keyboard buffer on entry, and on exit, so there are no
* events pending
*/
void kyb_pause(void);


#endif
