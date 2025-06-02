#include "detail/utils_internal.h"
#include "bitops.h"

uint32_t align(uint32_t val, uint32_t alignment)
{
	if (alignment && (val % alignment)) {
		val += (alignment - val % alignment);
	}
	return val;
}

/*
 * Copied from http://graphics.stanford.edu/~seander/bithacks.html .
 * Not very efficient but quite small
 */
unsigned int log2u(unsigned int v)
{
	unsigned int r = 0;
	while (v >>= 1) {
		r++;
	}
	return r;
}

/*!
 * Copied from
 * http://graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2 .
 */
bool isPowerOfTwo(unsigned int v)
{
	return v && !(v & (v - 1));
}

// Copied from
// http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
uint32_t roundUpToPowerOfTwo(uint32_t v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	v += (v == 0);
	return v;
}
