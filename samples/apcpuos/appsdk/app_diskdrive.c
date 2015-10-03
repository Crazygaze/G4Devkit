#include "app_diskdrive.h"
	
void app_dd_read(u32 diskNum, u32 sectorNum, char * data, int size)
{
	app_syscall4(kSysCall_diskDriveRead, (u32)diskNum, (u32)sectorNum, (u32)data, (u32)size);
}

void app_dd_write(u32 diskNum, u32 sectorNum, const char * data, int size)
{
	app_syscall4(kSysCall_diskDriveWrite, (u32)diskNum, (u32)sectorNum, (u32)data, (u32)size);
}

void app_dd_set_flags(u32 diskNum, u32 flags)
{
	app_syscall2(kSysCall_diskDriveSetFlags, (u32)diskNum, (u32)flags);
}

int app_dd_get_flags(u32 diskNum)
{
	return app_syscall1(kSysCall_diskDriveGetFlags, (u32)diskNum);
}

int app_dd_get_disk_info(u32 diskNum, DISK_INFO * di)
{
	return app_syscall2(kSysCall_diskDriveGetInfo, (u32)diskNum, (u32)di);
}