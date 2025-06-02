/*******************************************************************************
* This declares symbols that are defined in boot.asm
*******************************************************************************/

#ifndef _APCPUOS_BOOT_H
#define _APCPUOS_BOOT_H

typedef enum TxtColour
{
	/*
	Colours that can be used for background and foreground
	*/
	kTXTCLR_BLACK = 0,
	kTXTCLR_BLUE,
	kTXTCLR_GREEN,
	kTXTCLR_CYAN,
	kTXTCLR_RED,
	kTXTCLR_MAGENTA,
	kTXTCLR_BROWN,
	kTXTCLR_WHITE,
	
	/*
	Colours that can only be used for foreground
	*/
	kTXTCLR_GRAY,
	kTXTCLR_BRIGHT_BLUE,
	kTXTCLR_BRIGHT_GREEN,
	kTXTCLR_BRIGHT_CYAN,
	kTXTCLR_BRIGHT_RED,
	kTXTCLR_BRIGHT_MAGENTA,
	kTXTCLR_BRIGHT_YELLOW,
	kTXTCLR_BRIGHT_WHITE
} TxtColour;

/*!
 * Initializes the necessary data that we need to display things during boot
 */
void boot_ui_init(void);

/*!
 * Sets the background colour
 */
void boot_ui_setBkgColour(TxtColour colour);

/*!
 * Sets the foreground colour
 */
void boot_ui_setForeColour(TxtColour colour);

/*!
 * Clear the entire screen with the currently set background colour
 */
void boot_ui_clear(void);


/*!
 * Prints a string at the current cursor, and moves the cursor
 */
void boot_ui_printString(const char* str);

/*!
 * Similar to printf.
 * Prints a string a the current cursor, using the specified format and
 * parameters.
 */
void boot_ui_printf(const char* fmt, ...);

/*!
 * Display the OS logo
 */
void boot_ui_displayOSLogo(void);

#endif
