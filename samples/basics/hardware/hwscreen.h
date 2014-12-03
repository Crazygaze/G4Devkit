#ifndef _hwscreen_h_
#define _hwscreen_h_

#include "hwcommon.h"

#define SCR_XRES 80
#define SCR_YRES 25
#define SCR_BYTESPERCHARACTER 2
#define SCR_BUFFERSIZE (SCR_XRES*SCR_YRES*SCR_BYTESPERCHARACTER)

/*! Initializes the screen, with the default settings
*/
void scr_init(void);

/*
* \param buffer
*	The screen buffer will be mapped to this buffer.
*	Note that this buffer needs to be big enough.
*/
void scr_map(void* buffer);

/*! Prints a character at the specified location
*/
void scr_printCharAtXY(int x, int y, unsigned char ch);

/*! Prints a string at the specified location
*/
void scr_printStringAtXY(int x, int y, const char* str);

int scr_printfAtXY(int x, int y, const char* fmt, ...);

/*! Clears the screen
*/
void scr_clear(void);


#endif
