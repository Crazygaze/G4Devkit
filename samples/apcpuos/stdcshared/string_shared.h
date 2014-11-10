#ifndef _string_shared_h_
#define _string_shared_h_

#include <stddef_shared.h>

/*! Returns the length of the specified string 
*/
int strlen(const char* str);

/*! Copies a string to a specified destination
* \param dest
*	Where to copy to
* \param src
*	String to copy
* \param num
*	Maximum number of bytes to copy
* \return
*	Returns "dest"
* \note
*	If the end of the source string is found before "num" characters have been
*	copied, "dest" is padded with zeros until a a total of "num" characters have
*	written to it.
*/
char* strncpy(char *dest, const char *src, size_t num);

/*! Compares two strings
* \return
*	<0 The first character that does not match has a lower value in str1 than in
*	   str2
*
*	 0 The contents of both strings are equal
*
*	>0 The first character that does not match has a greater value in str1 than
*	   in str2
*/
int strcmp( const char * str1, const char * str2 );

/*! Sets a block of memory to the specified byte value
* \param dest
*	Memory to set
* \param c
*	Byte value to set the memory to
* \param count
*	How many bytes to set
* \return
*	Returns "dest"
*/
void* memset(__reg("r0") void* dest, __reg("r1") int c, __reg("r2") int count )
INLINEASM("\t\
memset r0,r1,r2");

/*! Copies a block of memory
* \param dest Destination
* \param src Source
* \param count How many bytes to copy
* \return
*	Returns "dest"
*/
void* memcpy(__reg("r0") void* dest, __reg("r1") const void* src,
__reg("r2")int count )
INLINEASM("\t\
memcpy r0,r1,r2");


#endif
