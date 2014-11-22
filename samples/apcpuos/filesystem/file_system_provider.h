#ifndef _FILE_SYSTEM_PROVIDER_
#define _FILE_SYSTEM_PROVIDER_

#include "logging/logging.h"
#include "stddef_shared.h"

bool change_drive(int driveNum);

bool open_file(FIL * file, const char * path, int mode, bool append);
bool close_file(FIL * file);

bool read_file(FIL * file, char * buf, int btr, int * br);
bool write_file(FIL * file, const char * buf, int btw, int * bw);

typedef enum FILE_TYPE{
    T_FILE = 0, 
    T_DIR
} FILE_TYPE;

typedef struct FILEDEPRECATED{
    char name[20];
    FILE_TYPE type;
} FILEDEPRECATED;


bool is_disk_exist(int driveNum);
bool is_file_system_exist();
bool is_dir_exist(const char * path);
bool make_file_system(int driveNum);
bool make_dir(const char * dir_name);
bool unlink(const char * link);
FILEDEPRECATED * get_subdirs(const char * path, int * size);

#endif