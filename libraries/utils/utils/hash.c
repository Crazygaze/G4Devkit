#include "detail/utils_internal.h"
#include "hash.h"

/*
 * 32 bit magic FNV-1a prime
 */
#define FNV_32_PRIME ((uint32_t)0x01000193)

uint32_t fnv_32a_buf(const void* buf, size_t len, uint32_t hval)
{
	const unsigned char *bp = (const unsigned char *)buf; /* start of buffer */
	const unsigned char *be = bp + len;             /* beyond end of buffer */

	/*
	 * FNV-1a hash each octet in the buffer
	 */
	while (bp < be)
	{
		/* xor the bottom with the current octet */
		hval ^= (uint32_t)*bp++;

		/* multiply by the 32 bit FNV magic prime mod 2^32 */
		hval *= FNV_32_PRIME;
	}

	/* return our new hash value */
	return hval;
}

uint32_t fnv_32a_str(const char *str, uint32_t hval)
{
	const unsigned char *s = (const unsigned char *)str;	/* unsigned string */

	/*
	 * FNV-1a hash each octet in the buffer
	 */
	while (*s)
	{
		/* xor the bottom with the current octet */
		hval ^= (uint32_t)*s++;

		/* multiply by the 32 bit FNV magic prime mod 2^32 */
		hval *= FNV_32_PRIME;
	}

	/* return our new hash value */
	return hval;
}
