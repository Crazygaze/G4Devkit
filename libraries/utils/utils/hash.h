#ifndef _utils_hash_h_
#define _utils_hash_h_

#include <stddef.h>

/*!
 * This should be passed to the fnv hash functions as the `hval` argument
 */
#define FNV1_32A_INIT 0x811c9dc5

/*!
 * fnv_32a_buf - perform a 32 bit Fowler/Noll/Vo FNV-1a hash on a buffer
 *
 * input:
 *	buf	 - start of buffer to hash
 *	len	 - length of buffer in octets
 *	hval - previous hash value or FNV1_32A_INIT
 *
 * returns:
 *	32 bit hash as a static hash type
 *
 * NOTE: To use the recommended 32 bit FNV-1a hash, use FNV1_32A_INIT as the
 * 	 hval arg on the first call to either fnv_32a_buf() or fnv_32a_str().
 */
uint32_t fnv_32a_buf(const void* buf, size_t len, uint32_t hval);

/*!
 * fnv_32a_str - perform a 32 bit Fowler/Noll/Vo FNV-1a hash on a string
 *
 * input:
 *	str  - string to hash
 *	hval - previous hash value or FNV1_32A_INIT
 *
 * returns:
 *	32 bit hash as a static hash type
 *
 * NOTE: To use the recommended 32 bit FNV-1a hash, use FNV1_32A_INIT as the
 *  	 hval arg on the first call to either fnv_32a_buf() or fnv_32a_str().
 */
uint32_t fnv_32a_str(const char *str, uint32_t hval);
	
#endif
