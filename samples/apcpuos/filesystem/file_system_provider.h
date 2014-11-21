#ifndef _FILE_SYSTEM_PROVIDER_
#define _FILE_SYSTEM_PROVIDER_

#include "kernel_shared/disk_info.h"
#include "stdcshared_defs.h"
#include <stdlib.h>

#include "extern/fatfs/src/ff.h"
#include "extern/fatfs/src/diskio.h"

bool make_file_system(int driveNum);

bool is_disk_exist(int driveNum);
bool is_dir_exist(const char * path);
bool change_drive(int driveNum);
bool is_file_system_exist();

typedef enum FILE_TYPE{
	T_FILE = 0, 
	T_DIR
} FILE_TYPE;

typedef struct FILE{
	char name[20];
	FILE_TYPE type;
} FILE;

FILE * get_subdirs(const char * path, int * size);
bool make_dir(const char * dir_name);
bool unlink(const char * link);

#endif