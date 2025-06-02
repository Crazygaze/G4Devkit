#ifndef _hwscreen_h_
#define _hwscreen_h_

#include "hwcommon.h"

#define HWSCR_XRES 80
#define HWSCR_YRES 25
#define HWSCR_CHARSTRIDE 2
#define HWSCR_BUFFERSIZE (HWSCR_XRES * HWSCR_YRES * HWSCR_CHARSTRIDE)

/*!
 * Buffer size used internally by calls that format a string.
 */
#define HWSCR_PRINTF_BUFSIZE (HWSCR_XRES + 1)

typedef struct {
	char* buffer;
	int xres;
	int yres;
	int charStride;
} HWScrInfo;

/*! Used internally */
hw_Drv* hw_scr_ctor(uint8_t bus);
/*! Used internally */
void hw_scr_dtor(hw_Drv* drv);

/*!
 * Initializes the screen, with the default settings.
 */
void hwscr_init(void);

/*!
 * Returns screen information.
 */
void hwscr_getInfo(HWScrInfo* info);

/*
 * Changes the screen memory mapping.
 *
 * \param buffer
 * The screen buffer will be mapped to this buffer.
 * Note that this buffer needs to have the right size, according to the device
 * specs.
 */
void hwscr_map(void* buffer);

/*!
 * Prints a character at the specified location.
 */
void hwscr_printCharAtXY(int x, int y, unsigned char ch);

/*!
 * Prints a string at the specified location.
 */
void hwscr_printStringAtXY(int x, int y, const char* str);

/*!
 * Prints a printf style string at the specified position.
 *
 * The resulting formatted string's length should be smaller than
 * HWSCR_PRINTF_BUFSIZE.
 * If it is >= than HWSCR_PRINTF_BUFSIZE, corruption will occur.
 */
int hwscr_printfAtXY(int x, int y, const char* fmt, ...);

/*!
 * Clears the screen.
 */
void hwscr_clear(void);

/*!
 * Clears a screen area.
 * Coordinates are inclusive, and x2/y2 needs to be after x1/y1 in memory
 */
void hwscr_clearArea(int x1, int y1, int x2, int y2);

/*!
 * Scrolls the screen up.
 */
void hwscr_scroll(int lines);


/*!
 * Prints a string at the current cursor position.
 */
void hwscr_printString(const char* str);

/*!
 * Prints a printf style string at the current cursor position.
 */
void hwscr_printf(const char* fmt, ...);

#endif
