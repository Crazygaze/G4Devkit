/*******************************************************************************
* This declares symbols that are defined in boot.asm
*******************************************************************************/

#ifndef _APCPU_BOOT_H_
#define _APCPU_BOOT_H_

// Markers to check the size of the interrupt context.
extern const int intrCtxStart;
extern const int intrCtxEnd;

/*!
 * Ram amount, in bytes
 */
extern const int ramAmount;

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

#define NO_INTERRUPT -1
extern const int krn_currIntrBusAndReason;
extern const int krn_prevIntrBusAndReason;

#endif



