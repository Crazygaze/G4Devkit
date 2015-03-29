/*******************************************************************************
* Disk Controller driver
*******************************************************************************/
#ifndef _APCPU_HWDISK_H
#define _APCPU_HWDISK_H

#include "hwcommon.h"

//
// Misc
//
#define HW_DKC_MAXDISKS 4

//
// Drive flags
//
// These 3 flags match what the device function gives us
#define HW_DKC_FLAG_WRITEPROTECTED (1<<31)
#define HW_DKC_FLAG_READING (1<<1)
#define HW_DKC_FLAG_WRITING (1<<0)
// Using unused bits for our driver flags
#define HW_DKC_FLAG_PRESENT (1<<2)
// Mask for all flags used internally
#define HW_DKC_INTERNAL_FLAGS \
	(HW_DKC_FLAG_WRITEPROTECTED | \
	 HW_DKC_FLAG_READING | \
	 HW_DKC_FLAG_WRITING | \
	 HW_DKC_FLAG_PRESENT)
	 
// First flag that can be used by other code as a custom flag
#define HW_DKC_FLAG_FIRSTCUSTOMBIT 3


//
// Available device functions
//
#define HW_DKC_FUNC_SETIRQMODE 0
#define HW_DKC_FUNC_QUERY 1
#define HW_DKC_FUNC_READSECTOR 2
#define HW_DKC_FUNC_WRITESECTOR 3

hw_Drv* hw_dkc_ctor(hw_BusId);
void hw_dkc_dtor(hw_Drv* drv);

#define hw_dkc_setIRQMode(state) \
	hw_hwiSimple1(HWBUS_DKC, HW_DKC_FUNC_SETIRQMODE, state)

/*
* Get information aboud disk drive
*/

typedef struct DISK_INFO {
	u32 sector_size;
	u32 sector_count;
	u32 block_size;
} DISK_INFO;

DISK_INFO hw_dck_getDiskInfo(u32 diskNum);

/*!
 * Reads data from a sector
 */
void hw_dkc_read(u32 diskNum, u32 sectorNum, char* data, int size);
void hw_dkc_read_sync(u32 diskNum, u32 sectorNum, char* data, int size);

/*!
 * Writes data to a sector
 */
void hw_dkc_write(u32 diskNum, u32 sectorNum, const char* data, int size);
void hw_dkc_write_sync(u32 diskNum, u32 sectorNum, const char* data, int size);

/*!
* Reads data from a disk sector, and blocks until the operation finishes.
* Not recommended to use this.
*/
void hw_dkc_read_sync(u32 diskNum, u32 sectorNum, char* data, int size);

/*!
* Writes data to a disk sector, and blocks until the operation finishes.
* Not recommended to use this.
*/
void hw_dkc_write_sync(u32 diskNum, u32 sectorNum, const char* data, int size);


u32 hw_dkc_getFlags(u32 diskNum);
void hw_dkc_setCustomFlags(u32 diskNum, u32 flags); 
void hw_dkc_clearCustomFlags(u32 diskNum, u32 flags);

#endif
