#ifndef _FILE_SYSTEM_PROVIDER_
#define _FILE_SYSTEM_PROVIDER_

#include "logging/logging.h"
#include "stddef_shared.h"

bool change_drive(int driveNum);

bool open_file(FIL * file, const char * path, int mode, bool append);
bool close_file(FIL * file);

bool read_file(FIL * file, char * buf, int btr, int * br);
bool write_file(FIL * file, const char * buf, int btw, int * bw);

typedef enum FS_ITEM_TYPE{
	T_FILE = 0,
	T_DIR
} FS_ITEM_TYPE;

typedef struct FS_ITEM{
	char 			path[14];
	unsigned long	size;	
	int				date;	
	int				time;	
	unsigned char	attrib;

	FS_ITEM_TYPE type;
} FS_ITEM;

bool open_directory(const char * path, DIR * dir);
bool close_directory(DIR * dir);
bool read_directory(DIR * dir, FS_ITEM * item);

bool fs_is_disk_exist(int driveNum);
bool fs_is_file_system_exist();
bool fs_is_dir_exist(const char * path);
bool fs_make_file_system(int driveNum);
bool fs_make_dir(const char * dir_name);
bool fs_unlink(const char * link);

#endif