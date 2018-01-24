#ifndef _common_h_
#define _common_h_

#include <stddef.h>

void loopForever(void);

/*! Pauses for the specified duration
* \param ms
*	Pause duration, in milliseconds
*/
void pause(int ms);

/*!
 */
typedef struct ROMProcessInfo
{
	unsigned int readOnlyAddr; // address
	unsigned int readOnlySize; // size in bytes
	unsigned int readWriteAddr; // address
	unsigned int readWriteSize; // size in bytes
	unsigned int sharedReadWriteAddr;
	unsigned int sharedReadWriteSize;
} ROMProcessInfo;
extern ROMProcessInfo processInfo;

#endif
