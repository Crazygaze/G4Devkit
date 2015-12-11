#include "misc.h"

uint32_t align(uint32_t val, uint32_t alignment)
{
	if (val % alignment) {
		val += (alignment - val % alignment);
	}
	return val;
}

// Hashing, based on  http://www.isthe.com/chongo/tech/comp/fnv/
unsigned int fnv32hash_compute( const void* data, unsigned int dataSize )
{
	const unsigned char* pData = (const unsigned char*)(data);
	const unsigned char* pEnd = pData + dataSize;
	unsigned int hashval = 2166136261U;
	unsigned int prime = 16777619U;

	while( pData < pEnd ) 
	{
		hashval *= prime;
		hashval ^= *pData++;
	}

	return hashval;
}

/*
 * Copied from http://graphics.stanford.edu/~seander/bithacks.html
 * Not very efficient but quite small
 */
unsigned int log2(unsigned int v)
{
	unsigned int r = 0;
	while (v >>= 1) {
		r++;
	}
	return r;
}

/*!
 * Copied from http://graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2
 */
 bool isPowerOfTwo(unsigned int v)
 {
	return v && !(v & (v - 1));
 }
 
 // Copied from http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
uint32_t roundUpToPowerOfTwo(uint32_t v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	v += (v==0);
	return v;
}