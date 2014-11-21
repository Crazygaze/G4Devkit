/*-----------------------------------------------------------------------/
/  Low level disk interface module for the APCPU                         /
/-----------------------------------------------------------------------*/

#ifndef _DISKIO_DEFINED
#define _DISKIO_DEFINED

#ifdef __cplusplus
extern "C" {
#endif

#define _USE_WRITE	1	/* 1: Enable disk_write function */
#define _USE_IOCTL	1	/* 1: Enable disk_ioctl fucntion */

#include "integer.h"


/* Status of Disk Functions */
typedef BYTE	DSTATUS;

/* Results of Disk Functions */
typedef enum DRESULT {
	RES_OK = 0,		/* 0: Successful */
	RES_ERROR,		/* 1: R/W Error */
	RES_WRPRT,		/* 2: Write Protected */
	RES_NOTRDY,		/* 3: Not Ready */
	RES_PARERR		/* 4: Invalid Parameter */
} DRESULT;


/*---------------------------------------*/
/* Prototypes for disk control functions */


DSTATUS disk_initialize (BYTE pdrv);
DSTATUS disk_status (BYTE pdrv);
DRESULT disk_read (BYTE pdrv, BYTE* buff, DWORD sector, UINT count);
DRESULT disk_write (BYTE pdrv, const BYTE* buff, DWORD sector, UINT count);
DRESULT disk_ioctl (BYTE pdrv, BYTE cmd, void* buff);

/* Disk Status Bits (DSTATUS) */
#define STA_NOINIT		0x01	/* Drive not initialized */
#define STA_NODISK		0x02	/* No medium in the drive */
#define STA_PROTECT		0x04	/* Write protected */

/* Command code for disk_ioctrl fucntion */

/* Generic command (used by FatFs) */
#define CTRL_SYNC			0	/* Complete pending write process (used at read/write cfg.) */
#define GET_SECTOR_COUNT	1	/* Get media size (used in f_mkfs()) */
#define GET_SECTOR_SIZE		2	/* Get sector size (used at variable sector size, _MAX_SS > _MIN_SS) */
#define GET_BLOCK_SIZE		3	/* Get erase block size (used in f_mkfs()) */
#define CTRL_ERASE_SECTOR	4	/* Force erased a block of sectors (used at _USE_ERASE) */


#ifdef __cplusplus
}
#endif

#endif
