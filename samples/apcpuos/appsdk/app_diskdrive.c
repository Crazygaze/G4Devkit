#include "app_diskdrive.h"

int app_dd_make_fat(const TCHAR * drive)
{
	return app_syscall1(kSysCall_DiskDriveMakeFAT, (int)drive);
}

int app_dd_open_file(FIL * fs, const TCHAR * file_name, int mode)
{
	return app_syscall3(kSysCall_DiskDriveOpenFile, (int)fs, (int)file_name, mode);
}

int app_dd_mount_drive(const TCHAR * drive_id)
{
	return app_syscall1(kSysCall_DiskDriveMountDrive, (int)drive_id);
}

int app_dd_close_file(FIL * fs)
{
	return app_syscall1(kSysCall_DiskDriveCloseFile, (int)fs);
}

int app_dd_write_to_file(FIL * fs, const void * buffer, int size, int * byte_written) 
{
	return app_syscall4(kSysCall_DiskDriveWriteToFile, (int)fs, (int)buffer, (int)size, (int)byte_written);
}

int app_dd_read_from_file(FIL * fs,  void * buffer, int size, int * byte_written) 
{
	return app_syscall4(kSysCall_DiskDriveReadFromFile, (int)fs, (int)buffer, (int)size, (int)byte_written);
}

int app_dd_file_seek(FIL * fs, DWORD offset)
{
	return app_syscall2(kSysCall_DiskDriveFileSeek, (int)fs, (int)offset);
}

int app_dd_sync(FIL * fs)
{
	return app_syscall1(kSysCall_DiskDriveSync, (int)fs);
}
 
int app_dd_open_dir(DIR * dir, const TCHAR * dir_name)
{
	return app_syscall2(kSysCall_DiskDriveOpenDir, (int)dir, (int)dir_name);
}

int app_dd_close_dir(DIR * dir)
{
	return app_syscall1(kSysCall_DiskDriveCloseDir, (int)dir);
}
 
int app_dd_read_dir(DIR * dir, FILINFO * file_info)
{
	return app_syscall2(kSysCall_DiskDriveReadDir, (int)dir, (int)file_info);
}
 
int app_dd_make_dir(const TCHAR * dir_name)
{
	return app_syscall1(kSysCall_DiskDriveMakeDir, (int)dir_name);
}
 
int app_dd_delete(const TCHAR * path)
{
	return app_syscall1(kSysCall_DiskDriveDeleteFileOrDir, (int)path);
}
 
int app_dd_rename_or_move(const TCHAR * old_name, const TCHAR * new_name)
{
	return app_syscall2(kSysCall_DiskDriveRenameOrMove, (int)old_name, (int)new_name);
}
 
int app_dd_get_file_info(const TCHAR * file_name, FILINFO * file_info)
{
	return app_syscall2(kSysCall_DiskDriveGetFileInfo, (int)file_name, (int)file_info);
}
 
int app_dd_get_free_clusters_num(const char * drive_number, DWORD * num_of_free_clusters)
{
	return app_syscall2(kSysCall_DiskDriveGetFreeClustersNum, (int)drive_number, (int)num_of_free_clusters);
}
 
