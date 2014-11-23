#ifndef _common_h_
#define _common_h_


#define TRUE 1
#define FALSE 0

typedef unsigned char u8;
typedef signed char s8;

typedef unsigned short u16;
typedef signed short s16;

typedef unsigned int u32;
typedef signed int s32;

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


//******************************************************************************
//		Screen functions
//******************************************************************************


/*! Prints a character at the specified location
*/
void printCharacter(int x, int y, unsigned char ch);

/*! Prints a string at the specified location
*/
void printString(int x, int y, const char* str);

/*! Prints a number at the specified location, with base
* \param x,y
*	Location to print at
* \param number
*	Number to print
* \param base
*	Base used to convert to string. E.g: 10 decimal or 16 for hexadecimal
*/
void printNumber(int x, int y, int number, int base);

/*! Clears the screen
*/
void clearScreen(void);

//******************************************************************************
//		Keyboard functions
//******************************************************************************

void kybClearBuffer(void);
int kybGetKey(void);


//******************************************************************************
//		Standard C library functions
//******************************************************************************

void itoa(int value, char *str, int base);
int strlen(const char* str);
void* memcpy(void* dest, const void* src, int count);
void* memset(void* dest, int c, int count);
void* memmove(void* dest, const void* src, int count);

#endif
