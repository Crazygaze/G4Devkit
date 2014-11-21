/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
#include "app_diskdrive.h"

#include "stdlib.h"

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

/*
	TODO: find a better way to share consts 
*/

#define HW_DKC_FLAG_FIRSTCUSTOMBIT 3
#define HW_DKC_MAXDISKS 4
#define HW_DKC_FLAG_PRESENT (1<<2)
#define HW_DKC_FLAG_WRITEPROTECTED (1<<31)

#define DISKIO_FLAG_INITIALIZED (1 << (HW_DKC_FLAG_FIRSTCUSTOMBIT))

DISK_INFO get_disk_info(int diskNum){
	DISK_INFO  di;// = malloc(sizeof(DISK_INFO));
	app_dd_get_disk_info(diskNum, &di);
	return di;
}

static int get_status(int diskNum)
{
	if (diskNum >= HW_DKC_MAXDISKS)
		return STA_NOINIT;		

	int flags = app_dd_get_flags(diskNum);
	DSTATUS res=0;
	if (flags & HW_DKC_FLAG_PRESENT)
	{
		if ( !(flags & DISKIO_FLAG_INITIALIZED) )
			res |= STA_NOINIT;
		
		if (flags & HW_DKC_FLAG_WRITEPROTECTED)
			res |= STA_PROTECT;
	} else {
		res = STA_NODISK;
	}
	return res;
}

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	return get_status(pdrv);
}


/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv		/* Physical drive nmuber */
)
{
	app_dd_set_flags(pdrv, DISKIO_FLAG_INITIALIZED);
	return get_status(pdrv);
}	

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read */
)
{
	app_dd_read( pdrv, sector, buff, count*get_disk_info(pdrv).sector_size );
		
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
)
{
	app_dd_write( pdrv, sector, buff, count*get_disk_info(pdrv).sector_size );
	
	return RES_OK;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DISK_INFO di = get_disk_info(pdrv);

	switch (cmd){
		case GET_SECTOR_SIZE:
			*((int*)buff)=di.sector_size;
			return RES_OK;
		case GET_SECTOR_COUNT:
			*((int*)buff)=di.sector_count;
			return RES_OK;
		case GET_BLOCK_SIZE:
			*((int*)buff)=di.block_size;
			return RES_OK;
		case CTRL_SYNC:
			return RES_OK;
		case CTRL_ERASE_SECTOR:
			/*
				TODO: Find information about CTRL_ERASE_SECTOR.
					  There's nothing here: http://elm-chan.org/fsw/ff/en/dioctl.html
			*/
			LOG("FatFS CTRL_ERASE_SECTOR not supported yet");
			return RES_PARERR;
		default:
			return RES_PARERR;
	}

	return RES_PARERR;
}
#endif


DWORD get_fattime(void)
{
	/*
		TODO: Returns the current date/time
	*/
	return 11111;//
}