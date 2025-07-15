#ifndef _appsdk_app_stdio_h
#define _appsdk_app_stdio_h

#include "os_shared/process_shared.h"

//
// Putting these here for now, because the standard library doesn't have access
// to the system calls.
// I'll have to reshuffle the projects to have the right dependencies
//
typedef struct FILE
{
	int dummy;
} FILE;

/*!
 * Opens a file
 */
FILE* fopen(const char* filename, const char* mode);

/*!
 * Closes a file
 */
int fclose(FILE* stream);


/*!
 * Write data to the specified file
 */
size_t fwrite(const void* buffer, size_t size, size_t count, FILE* stream);

/*!
 * Reads data from a file into the specified buffer
 */
size_t fread(void *buffer, size_t size, size_t count,  FILE* stream);


#endif
