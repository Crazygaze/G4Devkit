#ifndef _common_h_
#define _common_h_

#include <stddef.h>

//******************************************************************************
//		System functions
//******************************************************************************

/*!
* Initializes some global variables used by the library
*/
void initCommon(void);

/*!
* Loops forever, blocking the application
*/
void loopForever(void);

/*! Gets the number of seconds the system as been running for
*/
double getRunningTimeSeconds(void);

/*! Pauses for the specified duration
* \param ms
*	Pause duration, in milliseconds
*/
void pause(int ms);

/*!
 * Struct used to make hwi calls. It's used for both input and output
 */
typedef struct HwiData {
	unsigned int regs[4];
} HwiData;

int hwiCall(int bus, int funcNum, HwiData* data);

//******************************************************************************
//		Screen functions
//******************************************************************************


/*! Prints a character at the specified location
*/
void printCharacterAtXY(int x, int y, unsigned char ch);

/*! Prints a string at the specified location
*/
void printStringAtXY(int x, int y, const char* str);

int printfAtXY(int x, int y, const char* fmt, ...);

/*! Clears the screen
*/
void clearScreen(void);

//******************************************************************************
//		Keyboard functions
//******************************************************************************

void kybClearBuffer(void);
int kybGetKey(void);



int sprintf (char *buffer, const char *format, ...);


#endif
