#include "file_system_provider.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "stdlib_shared.h"
#include "stdcshared_defs.h"

#define DEBUG_FATFS 1

static int fs_mounted_drive = -1;

/*
 *	Working with files 
 */

bool open_file(FIL * file, const char * path, int mode, bool append)
{
	if (append == TRUE){
		mode |= FA_OPEN_ALWAYS;
	} else {
		if (mode & FA_WRITE){
			mode |= FA_CREATE_ALWAYS;
		} else {
			mode |= FA_OPEN_ALWAYS;
		}
	}

	FRESULT res = f_open(file, path, mode);
	
#ifdef DEBUG_FATFS
	printFResult ("f_open", res);
#endif

	// if append is needed, just seek to end of file
	if (append){
		FRESULT res_seek = f_lseek(file, f_size(file));

#ifdef DEBUG_FATFS
	printFResult ("f_lseek", res_seek);
#endif		
		if (res != FR_OK)
			return FALSE;
	}

	return (res == FR_OK);
}

bool close_file(FIL * file)
{
	FRESULT res = f_close(file);
	
#ifdef DEBUG_FATFS
	printFResult ("f_close", res);
#endif

	return (res == FR_OK);
}

bool read_file(FIL * file, char * buf, int btr, int * br)
{
	FRESULT res = f_read(file, (void *)buf, btr, br);
	
#ifdef DEBUG_FATFS
	printFResult ("f_read", res);
#endif

	return (res == FR_OK);
}

bool write_file(FIL * file, const char * buf, int btw, int * bw)
{
	FRESULT res = f_write(file, (const void *)buf, btw, bw);
	
#ifdef DEBUG_FATFS
	printFResult ("f_write", res);
#endif

	return (res == FR_OK);
}

/*
 *	Working with drives 
 */

int mount_drive(int driveNum)
{
	FATFS fs;
	
	char drive_name[2];
	drive_name[0] = (char)driveNum;
	drive_name[1] = 0;
	
	FRESULT res = f_mount(&fs, drive_name, 0);
		
#ifdef DEBUG_FATFS
	printFResult ("f_mount", res);
#endif

	if (res == FR_OK){
		fs_mounted_drive = driveNum;
		disk_initialize(fs_mounted_drive);
	}

	return res;
}

bool fs_make_file_system(int driveNum)
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

	return (res == FR_OK);
}

bool change_drive(int driveNum){
	if (mount_drive(driveNum) != FR_OK){
		return FALSE;
	}
	
	return TRUE;
}

bool fs_is_disk_exist(int driveNum)
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

bool fs_is_file_system_exist()
{	
	DIR dir;
	FRESULT res = f_opendir(&dir, "none.fil");

#ifdef DEBUG_FATFS
	printFResult("f_opendir", res);
#endif

	if (res == FR_NO_FILESYSTEM)
		return FALSE;
	
	return TRUE;
}

/*
 *	Working with directories 
 */


bool fs_make_dir(const char * dir_name)
{
	FRESULT res = f_mkdir(dir_name);
	
#ifdef DEBUG_FATFS
	printFResult("f_mkdir", res);
#endif

	return (res == FR_OK);
}

bool fs_unlink(const char * link)
{
	FRESULT res = f_unlink(link);
	
#ifdef DEBUG_FATFS
	printFResult("f_unlink", res);
#endif

	return (res == FR_OK);
}

bool fs_is_dir_exist(const char * path)
{
	DIR dir;
	FRESULT res = f_opendir(&dir, path);
	
#ifdef DEBUG_FATFS
	printFResult("is_dir_exist", res);
#endif


	if (res != FR_OK){
		return FALSE;
	}
	
	res = f_closedir(&dir);

#ifdef DEBUG_FATFS
	printFResult("f_opendir", res);
#endif

	return TRUE;
}

bool open_directory(const char * path, DIR * dir)
{
	FRESULT res = f_opendir(dir, path); 
	
#ifdef DEBUG_FATFS
	printFResult("f_opendir", res);
#endif

	return (res == FR_OK);
}

bool close_directory(DIR * dir)
{
	FRESULT res = f_closedir(dir); 
	
#ifdef DEBUG_FATFS
	printFResult("f_closedir", res);
#endif

	return (res == FR_OK);
}

bool read_directory(DIR * dir, FS_ITEM * item)
{
	FILINFO finfo;
	FRESULT res = f_readdir(dir, &finfo);
	
#ifdef DEBUG_FATFS
	printFResult("f_readdir", res);
#endif
	
	if (finfo.fname[0] == 0)
		return FALSE;

	if (res == FR_OK){
		item->date = finfo.fdate;
		item->attrib = finfo.fattrib;
		item->size = finfo.fsize;
		item->time = finfo.ftime;
		item->type = (finfo.fattrib & AM_DIR) ? T_DIR : T_FILE;
		
		memset(item->path, 0, 14);
		memcpy(item->path, finfo.fname, strlen(finfo.fname));
	}		

	return (res == FR_OK);
}
