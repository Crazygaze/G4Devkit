#include "app_diskdrive.h"

int app_dd_make_fat(char * drive)
{
	return app_syscall1(kSysCall_DiskDriveMakeFAT, (int)drive);
}

int app_dd_open_file(FIL * fs, char * file_name, int mode)
{
	return app_syscall3(kSysCall_DiskDriveOpenFile, (int)fs, (int)file_name, mode);
}

int app_dd_mount_drive(char * drive_id)
{
	return app_syscall1(kSysCall_DiskDriveMountDrive, (int)drive_id);
}

int app_dd_close_file(FIL * fs)
{
	return app_syscall1(kSysCall_DiskDriveCloseFile, (int)fs);
}

int app_dd_write_to_file(FIL * fs, const void * buffer, int size, int * byte_written) 
{
	return app_syscall4(kSysCall_DiskDriveWriteToFile, (int)fs, (int)buffer, (int)size, (int)byte_written)
}

int app_dd_read_from_file(FIL * fs,  void * buffer, int size, int * byte_written) 
{
	return app_syscall4(kSysCall_DiskDriveReadFromFile, (int)fs, (int)buffer, (int)size, (int)byte_written)
}