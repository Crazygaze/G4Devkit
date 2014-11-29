#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <file_system_provider.h>

void ch_drive(int driveNum)
{
	change_drive(driveNum);
}

FILE * fopen ( const char * filename, const char * mode )
{
	// check input
	if (strlen(mode) > 3 /*|| strlen(filename) > 8*/)
		return NULL;

	// parse mode
	int bytemode = 0;
	bool append = FALSE;
	
	int i = 0;
	while (mode[i]){
		switch (mode[i]){
			case 'r':
				bytemode |= FA_READ;
				break;
			case 'w':
				bytemode |= FA_WRITE;
				break;
			case 'a':
				append = TRUE;
				break;
		}
		i++;
	}
	
	FIL * file_struct = malloc(sizeof(FIL));
	if (open_file(file_struct, filename, bytemode, append) == FALSE)
		return NULL;
	
	FILE * file = malloc(sizeof(FILE));
	file->file_struct = file_struct;
	file->offset = 0;
	memcpy(file->path, filename, strlen(filename));
	memcpy(file->mode, mode, strlen(mode));
	
	return file;
}

int fclose ( FILE * stream )
{
	int res = close_file(stream->file_struct);
	
	free(stream->file_struct);
	free(stream);
	
	return res;
}

size_t fread ( void * ptr, size_t size, size_t count, FILE * stream )
{
	int readed_size;
	read_file(stream->file_struct, ptr, size*count, &readed_size);
	
	return readed_size;
}

size_t fwrite ( const void * ptr, size_t size, size_t count, FILE * stream )
{
	int writed_size;
	write_file(stream->file_struct, ptr, size*count, &writed_size);
	
	return writed_size;
}

int fgetc ( FILE * stream )
{
	char buff;
	if (fread(&buff, 1, 1, stream))
		return buff;
	
	return EOF;
}

char * fgets ( char * str, int num, FILE * stream )
{
	// check input
	if (num > 1024)
		return NULL;
		
	char buff[1024];
	memset(buff, 0, 1024);
	memset(str, 0, num);
	int readed = fread(buff, 1, num, stream);
	
	if (readed > 0 && readed <= num){
		memcpy(&str[0], &buff[0], readed);
		return str;
	}
	
	return NULL;
}

int fputc ( int character, FILE * stream )
{
	char buff;
	if (fwrite(&character, 1, 1, stream))
		return character;
	
	return EOF;
}

int fputs ( const char * str, FILE * stream )
{
	size_t ret_val = 0;
	if (ret_val = fwrite(str, 1, strlen(str), stream))
		return ret_val;
	
	return EOF;
}
