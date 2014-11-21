#include "file_system_provider.h"

#include <stdlib.h>
#include "stdcshared_defs.h"
#include "extern/fatfs/src/ff.h"

#define DEBUG_FATFS 1

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



int mount_drive(int driveNum)
{
	FATFS fs;// = malloc(sizeof(FATFS));
	
	char drive_name[2];
	drive_name[0] = (char)driveNum;
	drive_name[1] = 0;
	
	FRESULT res = f_mount(&fs, "0", 0);
	
	//free (fs);
#ifdef DEBUG_FATFS
	printFResult ("f_mount", res);
#endif

	return res;
}

int make_file_system(int driveNum)
{
	if (mount_drive(driveNum) != FR_OK){
		return 1;
	}
	
	char drive_name[2];
	drive_name[0] = (char)driveNum;
	drive_name[1] = 0;
	
	FRESULT res = f_mkfs(drive_name, 0, 0);

#ifdef DEBUG_FATFS
	printFResult ("f_mkfs", res);
#endif

	return 0;
}
