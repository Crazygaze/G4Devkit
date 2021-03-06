#include "logging.h"

void printFResult(char * info, FRESULT value)
{
	switch (value){
		case FR_OK:
			LOG ("%s: FR_OK", info);
			break;
		case FR_DISK_ERR:
			LOG ("%s: FR_DISK_ERR", info);
			break;
		case FR_INT_ERR:
			LOG ("%s: FR_INT_ERR", info);
			break;
		case FR_NOT_READY:
			LOG ("%s: FR_NOT_READY", info);
			break;
		case FR_NO_FILE:
			LOG ("%s: FR_NO_FILE", info);
			break;
		case FR_NO_PATH:
			LOG ("%s: FR_NO_PATH", info);
			break;
		case FR_INVALID_NAME:
			LOG ("%s: FR_INVALID_NAME", info);
			break;
		case FR_DENIED:
			LOG ("%s: FR_DENIED", info);
			break;
		case FR_EXIST:
			LOG ("%s: FR_EXIST", info);
			break;
		case FR_INVALID_OBJECT:
			LOG ("%s: FR_INVALID_OBJECT", info);
			break;
		case FR_WRITE_PROTECTED:
			LOG ("%s: FR_WRITE_PROTECTED", info);
			break;
		case FR_INVALID_DRIVE:
			LOG ("%s: FR_INVALID_DRIVE", info);
			break;
		case FR_NOT_ENABLED:
			LOG ("%s: FR_NOT_ENABLED", info);
			break;
		case FR_NO_FILESYSTEM:
			LOG ("%s: FR_NO_FILESYSTEM", info);
			break;
		case FR_MKFS_ABORTED:
			LOG ("%s: FR_MKFS_ABORTED", info);
			break;
		case FR_TIMEOUT:
			LOG ("%s: FR_TIMEOUT", info);
			break;
		case FR_LOCKED:
			LOG ("%s: FR_LOCKED", info);
			break;
		case FR_NOT_ENOUGH_CORE:
			LOG ("%s: FR_NOT_ENOUGH_CORE", info);
			break;
		case FR_TOO_MANY_OPEN_FILES:
			LOG ("%s: FR_TOO_MANY_OPEN_FILES", info);
			break;
		case FR_INVALID_PARAMETER:
			LOG ("%s: FR_INVALID_PARAMETER", info);
			break;
		default:
			LOG ("%s: unexpected return value", info);
			break;
	}
}

void printDStatus(char * info, DSTATUS value){
	switch (value){
		case STA_NOINIT:
			LOG ("%s: STA_NOINIT", info);
			break;
		case STA_NODISK:
			LOG ("%s: STA_NODISK", info);
			break;
		case STA_PROTECT:
			LOG ("%s: STA_PROTECT", info);
			break;
		default:	
			LOG ("%s: unexpected return value", info);	
			break;
	}
}


void printDResult(char * info, DRESULT value){
	switch (value){
		case RES_OK:
			LOG ("%s: RES_OK", info);
			break;
		case RES_ERROR:
			LOG ("%s: RES_ERROR", info);
			break;
		case RES_WRPRT:
			LOG ("%s: RES_WRPRT", info);
			break;
		case RES_NOTRDY:
			LOG ("%s: RES_NOTRDY", info);
			break;
		case RES_PARERR:
			LOG ("%s: RES_PARERR", info);
			break;
		default:	
			LOG ("%s: unexpected return value", info);	
			break;
	}
}
