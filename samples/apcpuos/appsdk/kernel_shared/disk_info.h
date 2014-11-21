#ifndef _DISK_INFO_H_
#define _DISK_INFO_H_

#include <stdint_shared.h>

/*
	TODO: find a better way
	
	If you change this struct, change same struct in os/hw/hwdisk.h
*/

typedef struct DISK_INFO {
	u32 sector_size;
	u32 sector_count;
	u32 block_size;
} DISK_INFO;

#endif