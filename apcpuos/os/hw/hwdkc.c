#include "hwdkc.h"
#include <string.h>
#include <stdlib.h>
#include "utils/bitops.h"

#include "../fs/extern/fatfs/source/ff.h"			/* Obtains integer types */
#include "../fs/extern/fatfs/source/diskio.h"		/* Declarations of disk functions */

#define HWDKC_MAXDISKS 4

#define HWDKC_FUNC_QUERYSTATE 1
#define HWDKC_FUNC_QUERYDISKINFO 2
#define HWDKC_FUNC_READSECTOR 3
#define HWDKC_FUNC_WRITESECTOR 4

/*
State code
	7 6 5 4 3 2 1 0   7 6 5 4 3 2 1 0   7 6 5 4 3 2 1 0   7 6 5 4 3 2 1 0 
	M P   i [R] [W]   M P   i [R] [W]   M P   i [R] [W]   M P   i [R] [W]

	M:
		0 : No disk
		1 : Disk mounted (present)
	P:
		0 - Disk is writable
		1 - Disk is write protected
	R: Read operation status
		0 - Ready
		1 - Ongoing
		2 - Operation Error
	W: Write operation status
		0 - Ready
		1 - Ongoing
		2 - Operation Error
		
	i: Tells if the drive has been initialized.
		This is a custom flag used by the driver code. It doesn't come from the
		hardware
*/

// Mask for the bits we use for flags the driver itself has
#define HWDKC_HWMASK                 0b11001111
#define HWDKC_CUSTOMFLAGS_MASK       0b00110000
#define HWDKC_CUSTOMFLAG_INITIALIZED 0b00010000

typedef enum OpState {
	kOpState_Ready,
	kOpState_Ongoing,
	kOpState_Error
} OpState;

typedef struct {
	FATFS fs;
	uint32_t sectorSize;
	uint32_t numSectors;
	
	// 7 6 5 4 3 2 1 0
	// M P     [R] [W]
	uint8_t state;
} hwdkc_Disk;

typedef struct {
	hw_Drv base;
	hwdkc_Disk disks[HWDKC_MAXDISKS];
} hwdkc_Drv;

static hwdkc_Drv dkcDrv;

static inline bool isMounted(uint8_t state)
{
	return (state & (1<<7)) ? true : false;
}

static inline bool isProtected(uint8_t state)
{
	return (state & (1<<6)) ? true : false;
}

static inline OpState getReadState(uint8_t state)
{
	return (state >> 2) & 0x3;
}

static inline OpState getWriteState(uint8_t state)
{
	return state & 0x3;
}

static inline bool isBusy(uint8_t state)
{
	return
		getReadState(state) != kOpState_Ready ||
		getWriteState(state) != kOpState_Ready;
}

static inline bool isInitialized(uint8_t state)
{
	return (state & HWDKC_CUSTOMFLAG_INITIALIZED) ? true : false;
}

static inline void setInitialized(hwdkc_Disk* disk, bool v)
{
	if (v)
		disk->state |= HWDKC_CUSTOMFLAG_INITIALIZED;
	else
		disk->state &= ~HWDKC_CUSTOMFLAG_INITIALIZED;
}


/*!
 * Returns the overal state of the device
 */
static inline uint32_t hwdkc_queryState(void)
{
	defineZeroed(HwfSmallData, data);
	hw_hwf_0_1(HWBUS_DKC, HWDKC_FUNC_QUERYSTATE, &data);
	return data.regs[0];
}

/*!
 * Blocks until any read or write operations complete
 */
static inline void hwdkc_sync(int diskNum)
{
	// #TODO : This is problematic, since if this is called while formatting
	// a drive, we will be clearing the device's IRQ pin, and we'll miss any
	// interrupts for drives that were attached or detached while the formating
	// is going on
	
	hwdkc_Disk* disk = &dkcDrv.disks[diskNum];
	int counter = 0;
	do {
		//HW_VER("Drive %d sync. %d", diskNum, counter++);
		uint32_t state = hwdkc_queryState();
		uint8_t newState = (state >> (8 * diskNum)) & 0xFF;
		// add the custom flags the driver keeps
		newState |= disk->state & HWDKC_CUSTOMFLAGS_MASK;
		disk->state = newState;
	} while (isBusy(disk->state));
}

static bool hwdkc_format(const char* driveName)
{
	HW_LOG("Formatting drive %s...", driveName);
	
	const workBufSize = FF_MAX_SS;
	void* workBuf = calloc(workBufSize);
	if (!workBuf) {
		HW_ERR("Failed to allocate working buffer.");
		return false;
	}
	
	MKFS_PARM opt;
	opt.fmt = FM_FAT | FM_SFD;
	opt.n_fat = 0;
	opt.align = 512;
	opt.n_root = 0;
	opt.au_size = 512;
		
	FRESULT res = f_mkfs(driveName, &opt, workBuf, workBufSize);
	free(workBuf);
	
	if (res == FR_OK) {
		return true;
	} else {
		HW_ERR("Failed to format drive %s. Error=%d", driveName, res);
		return false;
	}
}

static bool hwdkc_mount(int diskNum)
{
	hwdkc_Disk* disk = &dkcDrv.disks[diskNum];
	char driveName[2];
	driveName[0] = '0' + diskNum;
	driveName[1] = 0;
	
	FRESULT res = f_mount(&disk->fs, driveName, 1);
	//FRESULT res = FR_NO_FILESYSTEM;
	
	if (res == FR_NO_FILESYSTEM) {
		if (!hwdkc_format(driveName))
			return false;
			
		// If we formatted, then now we need to attempt mounting again
		res = f_mount(&disk->fs, driveName, 1);
	}
		
	if (res == FR_OK) {
		HW_LOG("Mounted drive %s", driveName);
		
		// #TODO : Remove this
		DWORD freeClusters;
		FATFS* fs;
		if (f_getfree(driveName, &freeClusters, &fs) == FR_OK)
		{
			u32 totalSectors = (fs->n_fatent - 2) * fs->csize;
			u32 freeSectors = freeClusters * fs->csize;
			HW_LOG("Drive %s size: %10u KiB total space. %10u KiB available",
				driveName, totalSectors/2, freeSectors /2);
		}
		
		return true;
	} else {
		HW_ERR("Failed to mount drive %s. Error=%d", driveName, res);
		return false;
	}
}

static void hwdkc_updateState(void)
{
	uint32_t state = hwdkc_queryState();
	for (int i = 0; i < HWDKC_MAXDISKS; i++) {
		hwdkc_Disk* disk = &dkcDrv.disks[i];
		
		uint8_t newState = (state >> (8*i)) & 0xFF;
		// add the custom flags the driver keeps
		newState |= disk->state & HWDKC_CUSTOMFLAGS_MASK;
		
		uint8_t oldState = disk->state;
		
		// If nothing changed for this drive number, then nothing to do
		if (newState == oldState)
			continue;
			
		disk->state = newState;
			
		if (!isMounted(oldState) && isMounted(newState)) {
			defineZeroed(HwfFullData, data);
			// Set what drive we want to retrieve information from
			data.regs[0] = i;
			int res = hw_hwffull(HWBUS_DKC, HWDKC_FUNC_QUERYDISKINFO, &data);
			if (res != HWERR_SUCCESS)
				continue;
				
			disk->numSectors = data.regs[0];
			disk->sectorSize = data.regs[1];
			uint32_t KiB = (disk->numSectors * disk->sectorSize) / 1024;
			HW_LOG("Drive %d: size=%uKiB, numSectors=%d, sectorSize=%d", i, KiB,
				disk->numSectors, disk->sectorSize);
			
			hwdkc_mount(i);
		}
		else
		{
		}
		
	}
}

void hwdkc_handler(void)
{
	hwdkc_updateState();
}

void handles_file_dtr(void* data)
{
	if (!data)
		return;
		
	FIL* fp = (FIL*)data;
	FRESULT res = f_close(fp);
	if (res != FR_OK) {
		OS_LOG("Failed to close file");
	}
	free(fp);
}

hw_Drv* hwdkc_ctor(uint8_t bus)
{
	dkcDrv.base.handler = hwdkc_handler;
	
	hwdkc_handler();
	
	return &dkcDrv.base;
}

void hwdkc_dtor(hw_Drv* drv)
{

}

/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

/* Definitions of physical drive number for each drive */
#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive number to identify the drive */
)
{
	if (pdrv >= HWDKC_MAXDISKS)
		return STA_NODISK;
		
	hwdkc_Disk* disk = &dkcDrv.disks[pdrv];
	
	if (!isMounted(disk->state))
		return STA_NODISK;
		
	DSTATUS res = 0;
	if (!isInitialized(disk->state))
		res = STA_NOINIT;
		
	if (isProtected(disk->state))
		res |= STA_PROTECT;
		
	return res;
}


/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS s = disk_status(pdrv);
	if (s & STA_NOINIT) {
		setInitialized(&dkcDrv.disks[pdrv], true);
		s &= ~STA_NOINIT;
	} 
	return s;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

// See https://elm-chan.org/fsw/ff/doc/dread.html
DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	HW_LOG("disk_read: Drive %d, sector=%u, numSectors=%u",
		(int)pdrv, (u32)sector, (u32)count);
		
	if (pdrv >= HWDKC_MAXDISKS)
		return RES_PARERR;
		
	hwdkc_Disk* disk = &dkcDrv.disks[pdrv];
	
	if (!isMounted(disk->state))
		return RES_ERROR;
		
	if (!isInitialized(disk->state))
		return RES_NOTRDY;
		
	// If there is an ongoing read or write, then we can't do anything
	if (isBusy(disk->state))
		return RES_ERROR;
	
	defineZeroed(HwfFullData, data);
	data.regs[0] = pdrv;
	data.regs[1] = sector;
	data.regs[2] = (uint32_t)buff;
	data.regs[3] = count;
	int res = hw_hwffull(HWBUS_DKC, HWDKC_FUNC_READSECTOR, &data);
	
	if (res == HWERR_SUCCESS) {
		hwdkc_sync(pdrv);
		return RES_OK;
	} else {
		return RES_PARERR;
	}
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

// See https://elm-chan.org/fsw/ff/doc/dwrite.html
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	HW_VER("disk_write: Drive %d, sector=%u, numSectors=%u",
		(int)pdrv, (u32)sector, (u32)count);
		
	if (pdrv >= HWDKC_MAXDISKS)
		return RES_PARERR;
		
	hwdkc_Disk* disk = &dkcDrv.disks[pdrv];
	
	if (!isMounted(disk->state))
		return RES_ERROR;
		
	if (!isInitialized(disk->state))
		return RES_NOTRDY;
		
	// If there is an ongoing read or write, then we can't do anything
	if (isBusy(disk->state))
		return RES_ERROR;
		
	if (isProtected(disk->state))
		return RES_WRPRT;
		
	defineZeroed(HwfFullData, data);
	data.regs[0] = pdrv;
	data.regs[1] = sector;
	data.regs[2] = (uint32_t)buff;
	data.regs[3] = count;
	int res = hw_hwffull(HWBUS_DKC, HWDKC_FUNC_WRITESECTOR, &data);
	
	if (res == HWERR_SUCCESS) {
		hwdkc_sync(pdrv);
		return RES_OK;
	} else {
		return RES_PARERR;
	}
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

// See https://elm-chan.org/fsw/ff/doc/dioctl.html
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	if (pdrv >= HWDKC_MAXDISKS)
		return RES_ERROR;
		
	hwdkc_Disk* disk = &dkcDrv.disks[pdrv];
	
	if (!isMounted(disk->state))
		return RES_ERROR;;
		
	if (!isInitialized(disk->state))
		return RES_NOTRDY;
		
	DRESULT res = RES_OK;
	
	if (cmd == CTRL_SYNC) {
		res = res;
	} else if (cmd == GET_SECTOR_COUNT) {
		*((LBA_t*)buff) = disk->numSectors;
	} else if (cmd == GET_SECTOR_SIZE) {
		*((DWORD*)buff) = disk->sectorSize;
	} else if (cmd == GET_BLOCK_SIZE) {
		*((DWORD*)buff) = disk->sectorSize;
	} else {
		res = RES_PARERR;
	}
		
	return res;;
}

/*-----------------------------------------------------------------------*/
/* RTC                                                                   */
/*-----------------------------------------------------------------------*/
DWORD get_fattime(void)
{
	return 0;
}

