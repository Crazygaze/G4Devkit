#ifndef _stdio_h_
#define _stdio_h_

#include <stdarg.h>
#include "stdio_shared.h"

#define MAX_SYMBOLS_FILENAME 10

typedef struct FILE {
	char path[MAX_SYMBOLS_FILENAME];
	char mode[3];
	void * offset;
	void * file_struct;
} FILE;

void ch_drive(int driveNum);

FILE * fopen ( const char * filename, const char * mode );
int fclose ( FILE * stream );

size_t fread ( void * ptr, size_t size, size_t count, FILE * stream );
size_t fwrite ( const void * ptr, size_t size, size_t count, FILE * stream );

int fgetc ( FILE * stream );
char * fgets ( char * str, int num, FILE * stream );

int fputc ( int character, FILE * stream );
int fputs ( const char * str, FILE * stream );

bool is_disk_exist(int driveNum);
bool is_file_system_exist();
bool is_dir_exist(const char * path);
bool make_file_system(int driveNum);
bool make_dir(const char * dir_name);
bool unlink(const char * link);

#endif


