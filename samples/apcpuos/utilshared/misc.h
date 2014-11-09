/********************************************************************
	CrazyGaze (http://www.crazygaze.com)
	Author : Rui Figueira
	Email  : rui@crazygaze.com
	
	purpose:
	Miscellaneous definitions
*********************************************************************/

#ifndef _utilshared_misc_h_
#define _utilshared_misc_h_

#include "utilsharedconfig.h"

#ifdef __cplusplus //if we are compiling as C++, tell
extern "C" { //the compiler that this stuff is plain old C
#endif


uint32_t align(uint32_t val, uint32_t alignment);
#define isAligned(val,alignment) ((val)%(alignment)==0)

// Hashing, based on  http://www.isthe.com/chongo/tech/comp/fnv/
unsigned int fnv32hash_compute( const void* data, unsigned int dataSize );


/*
	Gets the bits between H-L (inclusive)
	For example, to get the top 4 bits of some byte variable:
		GET_BITS(somevar, 7, 4)
*/
#define GET_BITS(val, H, L) \
	(val & (unsigned int)((((unsigned int)(1) << ((H)-(L)+1)) -1) << (L))) >> (L)

#define SET_BIT(var, pos, value) \
	((var & ~((uint32_t)(1) << pos)) | ((uint32_t)(value)<<pos))
	
#define ISBIT_SET(var, bit) \
	(var & (1<<bit))

unsigned int log2(unsigned int v);
bool isPowerOfTwo(unsigned int v);
uint32_t roundUpToPowerOfTwo(uint32_t v);


#ifdef __cplusplus //
} // closing curly bracket
#endif

#endif // common_h__

