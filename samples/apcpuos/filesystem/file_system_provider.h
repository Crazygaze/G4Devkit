#ifndef _FILE_SYSTEM_PROVIDER_
#define _FILE_SYSTEM_PROVIDER_

#include "kernel_shared/disk_info.h"
#include "stdcshared_defs.h"
#include <stdlib.h>

int make_file_system(int driveNum);

bool is_disk_exist(int driveNum);
bool change_drive(int driveNum);
bool is_file_system_exist();

#endif