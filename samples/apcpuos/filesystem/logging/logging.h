#ifndef _LOGGING_H_
#define _LOGGING_H_

#include "stdcshared_defs.h"
#include "../extern/fatfs/src/ff.h"
#include "../extern/fatfs/src/diskio.h"

void printFResult(char * info, FRESULT value);
void printDStatus(char * info, DSTATUS value);
void printDResult(char * info, DRESULT value);

#endif