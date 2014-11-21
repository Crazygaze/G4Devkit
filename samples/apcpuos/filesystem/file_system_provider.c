#include "file_system_provider.h"

#include <stdlib.h>
#include <string.h>

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

static int fs_mounted_drive = -1;

int mount_drive(int driveNum)
{
	FATFS fs;
	
	char drive_name[2];
	drive_name[0] = (char)driveNum;
	drive_name[1] = 0;
	
	FRESULT res = f_mount(&fs, "0", 0);
		
#ifdef DEBUG_FATFS
	printFResult ("f_mount", res);
#endif

	if (res == FR_OK)
		fs_mounted_drive = driveNum;

	return res;
}

bool make_file_system(int driveNum)
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

	if (res != FR_OK)
		return FALSE;

	return TRUE;
}

bool change_drive(int driveNum){
	if (mount_drive(driveNum) != FR_OK){
		return FALSE;
	}
	
	return TRUE;
}

bool is_disk_exist(int driveNum)
{
	DSTATUS res = disk_status( driveNum );
	
#ifdef DEBUG_FATFS
	printDStatus("disk_status", res);
#endif

	if (res == STA_NODISK){
		return FALSE;
	} 
	
	return TRUE;
}

bool is_file_system_exist()
{
	disk_initialize(fs_mounted_drive);
	
	DIR dir;
	FRESULT res = f_opendir(&dir, "none.fil");

#ifdef DEBUG_FATFS
	printFResult("f_opendir", res);
#endif

	if (res == FR_NO_FILESYSTEM)
		return FALSE;
	
	return TRUE;
}

bool make_dir(const char * dir_name)
{
	FRESULT res = f_mkdir(dir_name);
	
#ifdef DEBUG_FATFS
	printFResult("f_mkdir", res);
#endif


	if (res != FR_OK){
		return FALSE;
	}
	
	return TRUE;
}

bool unlink(const char * link)
{
	FRESULT res = f_unlink(link);
	
#ifdef DEBUG_FATFS
	printFResult("f_unlink", res);
#endif


	if (res != FR_OK){
		return FALSE;
	}
	
	return TRUE;
}

bool is_dir_exist(const char * path)
{
	DIR dir;
	FRESULT res = f_opendir(&dir, path);
	
#ifdef DEBUG_FATFS
	printFResult("is_dir_exist", res);
#endif


	if (res != FR_OK){
		return FALSE;
	}
	
	f_closedir(&dir);
	
	return TRUE;
}

FILE * get_subdirs(const char * path, int * size)
{
    FRESULT res;
    FILINFO fno;
    DIR dir;
    int i;
    char *fn;   /* This function assumes non-Unicode configuration */

	disk_initialize(fs_mounted_drive);
	// calculate count of files inside dir at first
	int count = 0;
    res = f_opendir(&dir, path);                       /* Open the directory */
	if (res == FR_OK) {
		for (;;) {
			disk_initialize(fs_mounted_drive);
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
#ifdef DEBUG_FATFS
	printFResult("f_readdir", res);
#endif			
			
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            
			count++;
        }
        f_closedir(&dir);
    }
	LOG("COUNT: %d", count);
	
	if (count == 0){
		*size = 0;
		return NULL;
	}
	// allocate memory to result 
	FILE * files = malloc(count * sizeof(FILE));
	
	// store items to result
	disk_initialize(fs_mounted_drive);
	res = f_opendir(&dir, path);                       /* Open the directory */
    
	if (res == FR_OK) {
			
		int count = 0;
		for (;;) {
			disk_initialize(fs_mounted_drive);
			res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
		
            fn = fno.fname;

            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
                memcpy(files[count].name, fn, strlen(fn));
				files[count].type = T_DIR;
            } else {                                       /* It is a file. */
                memcpy(files[count].name, fn, strlen(fn));
				files[count].type = T_FILE;
            }
			
			count++;
		}
	}
	*size = count;
    return files;
}