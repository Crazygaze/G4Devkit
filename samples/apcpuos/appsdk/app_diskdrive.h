#ifndef _app_diskdrive_h_
#define _app_diskdrive_h_

#include "appsdkconfig.h"
#include "app_syscalls.h"
#include "os/extern/fatfs/src/ff.h"

int app_dd_make_fat(const TCHAR * drive);
int app_dd_open_file(FIL * fs, const TCHAR * file_name, int mode);
int app_dd_mount_drive(const TCHAR * drive_id);		
int app_dd_close_file(FIL * fs);
int app_dd_write_to_file(FIL * fs, const void * buffer, int size, int * byte_written);
int app_dd_read_from_file(FIL * fs, void * buffer, int size, int * byte_written);
int app_dd_file_seek(FIL * fs, DWORD offset);
int app_dd_sync(FIL * fs);
int app_dd_open_dir(DIR * dir, const TCHAR * dir_name);
int app_dd_close_dir(DIR * dir);
int app_dd_read_dir(DIR * dir, FILINFO * file_info);
int app_dd_make_dir(const TCHAR * dir_name);
int app_dd_delete(const TCHAR * path);
int app_dd_rename_or_move(const TCHAR * old_name, const TCHAR * new_name);
int app_dd_get_file_info(const TCHAR * file_name, FILINFO * file_info);
int app_dd_get_free_clusters_num(const char * drive_number, DWORD * num_of_free_clusters);


#endif