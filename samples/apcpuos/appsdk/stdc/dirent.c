#include "dirent.h"

#include <stdlib.h>

DIRECTORY * opendir(const char * path)
{
	DIR * dir = malloc(sizeof(DIR));
	if (! open_directory(path, dir)){
		return NULL;
	}
	
	DIRECTORY * dir_ret = malloc(sizeof(DIRECTORY));
	dir_ret->dir_struct = (void*)dir;
	
	return dir_ret;
}

bool closedir(DIRECTORY * dir)
{
	if (!close_directory((DIR *)dir->dir_struct))
		return FALSE;

	free(dir->dir_struct);
	free(dir);
	
	return TRUE;
}

bool readdir(DIRECTORY * dir, FS_ITEM * item)
{
	FS_ITEM item_tmp;
	if (! read_directory((DIR *)dir->dir_struct, &item_tmp))
		return FALSE;
	
	*item = item_tmp;
	return TRUE;
}