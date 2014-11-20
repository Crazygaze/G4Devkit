#ifndef _app_diskdrive_h_
#define _app_diskdrive_h_

#include "appsdkconfig.h"
#include "app_syscalls.h"
#include "os/extern/fatfs/src/ff.h"

#define app_dd_file_seek() 				app_syscall0(kSysCall_DiskDriveFileSeek)
#define app_dd_sync() 					app_syscall0(kSysCall_DiskDriveSync)
#define app_dd_open_dir() 				app_syscall0(kSysCall_DiskDriveOpenDir)
#define app_dd_close_dir() 				app_syscall0(kSysCall_DiskDriveCloseDir)
#define app_dd_read_dir() 				app_syscall0(kSysCall_DiskDriveReadDir)
#define app_dd_make_dir() 				app_syscall0(kSysCall_DiskDriveMakeDir)
#define app_dd_delete() 				app_syscall0(kSysCall_DiskDriveDeleteFileOrDir)
#define app_dd_rename_or_move()			app_syscall0(kSysCall_DiskDriveRenameOrMove)
#define app_dd_get_file_info() 			app_syscall0(kSysCall_DiskDriveGetFileInfo)
#define app_dd_change_dir() 			app_syscall0(kSysCall_DiskDriveChangeDir)
#define app_dd_change_drive() 			app_syscall0(kSysCall_DiskDriveChangeDrive) 
#define app_dd_get_current_dir()		app_syscall0(kSysCall_DiskDriveGetCurrentDir)
#define app_dd_get_free_clusters_num() 	app_syscall0(kSysCall_DiskDriveGetFreeClustersNum)

int app_dd_make_fat(char * drive);
int app_dd_open_file(FIL * fs, char * file_name, int mode);
int app_dd_mount_drive(char * drive_id);		
int app_dd_close_file(FIL * fs);
int app_dd_write_to_file(FIL * fs, const void * buffer, int size, int * byte_written);
int app_dd_read_from_file(FIL * fs, void * buffer, int size, int * byte_written);


#endif