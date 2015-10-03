#ifndef _DIRENT_H_
#define _DIRENT_H_

#include "file_system_provider.h"

typedef struct DIRECTORY{
	char path[128];
	void * dir_struct;
} DIRECTORY;

DIRECTORY * opendir(const char * path);
bool closedir(DIRECTORY * dir);
bool readdir(DIRECTORY * dir, FS_ITEM * item);

#endif