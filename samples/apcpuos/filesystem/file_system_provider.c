#include "file_system_provider.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "stdlib_shared.h"
#include "stdcshared_defs.h"

#define DEBUG_FATFS 1

static int fs_mounted_drive = -1;

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

	disk_initialize(fs_mounted_drive);
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
	disk_initialize(fs_mounted_drive);
	FRESULT res = f_close(file);
	
#ifdef DEBUG_FATFS
	printFResult ("f_close", res);
#endif

	return (res == FR_OK);
}

bool read_file(FIL * file, char * buf, int btr, int * br)
{
	disk_initialize(fs_mounted_drive);
	FRESULT res = f_read(file, (void *)buf, btr, br);
	
#ifdef DEBUG_FATFS
	printFResult ("f_read", res);
#endif

	return (res == FR_OK);
}

bool write_file(FIL * file, const char * buf, int btw, int * bw)
{
	disk_initialize(fs_mounted_drive);
	FRESULT res = f_write(file, (const void *)buf, btw, bw);
	
#ifdef DEBUG_FATFS
	printFResult ("f_write", res);
#endif

	return (res == FR_OK);
}


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

FILEDEPRECATED * get_subdirs(const char * path, int * size)
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
    FILEDEPRECATED * files = malloc(count * sizeof(FILEDEPRECATED));
    
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

