#ifndef _app_diskdrive_h_
#define _app_diskdrive_h_

#include "appsdkconfig.h"
#include "app_syscalls.h"
#include "kernel_shared/disk_info.h"

void app_dd_read(u32 diskNum, u32 sectorNum, char * data, int size);
void app_dd_write(u32 diskNum, u32 sectorNum, const char * data, int size);
void app_dd_set_flags(u32 diskNum, u32 flags);
int app_dd_get_flags(u32 diskNum);
int app_dd_get_disk_info(u32 diskNum, DISK_INFO * di);

#endif