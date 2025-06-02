#ifndef _utils_bitops_h_
#define _utils_bitops_h_

#include <stddef.h>

/*!
 * Aligns `val` on `alignment`, where `alignment` is a power of two
 *
 * E.g: `ALIGN(5,4)` returns 8
 */
#define ALIGN(val, alignment) (((val) + ((alignment) - 1)) & ~((alignment) - 1))

/*!
 * Aligns `val` on `alignment`, where alignment DOESN'T need to be a power of 2
 */
uint32_t align(uint32_t val, uint32_t alignment);

/*!
 * Checks if `val` is aligned on `alignment` bytes, where `alignment is a power
 * of two
 */
#define ISALIGNED(val, alignment) (((val) & ((alignment) - 1)) == 0)

/*!
 * Gets the bits between H-L (inclusive).
 * For example, to get the top 4 bits of some byte variable:
 * GETBITS(somevar, 7, 4)
 *
 * Note that the value gets shifted, so that if e.g you want to get the 3 MSB
 * bits of a word, the result is a value from 0 to 7
 */
#define GETBITS(val, H, L) \
	((val & (uint32_t)((((uint32_t)(1) << ((H)-(L)+1)) -1) << (L))) >> (L))
	
/*!
 * Checks if a specified bit in a word is set	
 * E.g: if (ISBITSIT(someVar, 12)) { ... }
 */
#define ISBITSET(val, B) \
	((val) & ((uint32_t)1 << (uint32_t)(B)))

/*!
 * Makes a bit mask for a range of bits (H-L, inclusive)
 */
#define MAKEMASK(H,L) (((uint32_t) -1 >> ((sizeof(uint32_t)*8-1) - (H))) & ~((1U << (L)) - 1))

/*!
 * Returns `val` with bit B set to 1
 */
#define SETBIT(val, B) \
	((val) | ((uint32_t)1 << (uint32_t)(B)))
	
/*!
 * returns val with the specified range of bits zeroed (H-L, inclusive)
 */
#define ZEROBITS(val, H, L) \
	((val) & (~MAKEMASK((H),(L))))

/*!
 *  Returns "val" with a range of bits (H-L, inclusive) set to "bits".
 */
#define SETBITS(val, H, L, bits) \
	( ZEROBITS((val),(H),(L)) | ((bits) << (L)) )
	
/*!
 * Returns `val` with bit B cleared
 */
#define CLEARBIT(val, B) \
	((val) & ~((uint32_t)1<<(uint32_t)(B)))
	
	
/*!
 * SETBIT and CLEAR bit in one. As-in, it returns `val` with bit B set 0 or 1,
 * as specified by `state`
 */
#define SETBIT_TO(val, B, state) \
	(((val) & ~((uint32_t)(1) << (B))) | ((uint32_t)(state)<<(B)))

/*!
 * Calculates base-2 logarithm of `v`
 */
unsigned int log2u(unsigned int v);

/*!
 * Checks if the specified number is a power of 2
 */
bool isPowerOfTwo(unsigned int v);

/*!
 * Rounds up v to a power of 2
 */
uint32_t roundUpToPowerOfTwo(uint32_t v);

#endif
