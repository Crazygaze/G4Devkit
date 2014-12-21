#ifndef _hwkeyboard_h_
#define _hwkeyboard_h_

/*! Clears the keyboard event buffer
*/
void kyb_clearBuffer(void);

/*! Gets the next keyboard "Typed" key.
* If a "Typed" event is not available, it will block until one is available.
*/
int kyb_getKey(void);

/*! Pauses until a key is typed
* This will clear the keyboard buffer on entry, and on exit, so there are no
* events pending
*/
void kyb_pause(void);

#endif
