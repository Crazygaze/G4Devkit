/*******************************************************************************
 *
 * Macros to deal with bitsets
 * Based on http://c-faq.com/misc/bitsets.html
 *
 * Here are some usage examples, declaring an array of 47 bits:
 *
 * char bitarray[BS_NUMSLOTS(47)];
 * To set the 23rd bit:
 *		BS_SETBIT(bitarray, 23);
 *
 * To test the 35th bit:
 * 		if(BS_ISBITSET(bitarray, 35)) ...
 *
 * To compute the union of two bit arrays and place it in a third array
 * (with all three arrays declared as above):
 * 		for(i = 0; i < BS_NUMSLOTS(47); i++)
 * 			array3[i] = array1[i] | array2[i];
 *
 * To compute the intersection, use & instead of |. 
*******************************************************************************/
 
#ifndef _utils_bitset_h_
#define _utils_bitset_h_

#include "detail/utils_common.h"
#include <limits.h>

// NOTE: In all of the macros, bitN is the bit number within the array

/*!
 * Returns how many bytes are required for an array of numBits bits.
 */
#define BS_NUMSLOTS(numBits) ((numBits + CHAR_BIT - 1) / CHAR_BIT)


/*!
 * Given a bitN, it returns the bit mask for that bit.
 * This is mostly an implementation detail an not meant to be used externally
 *
 * NOTE: The mask is for the byte holding that bit.
 */
#define _BS_BITMASK(bitN) (1 << ((bitN) % CHAR_BIT))

/*!
 * Given a bitN, it returns the byte index for that bit.
 * This is mostly an implementation detail an not meant to be used externally
 */
#define _BS_BITSLOT(bitN) ((bitN) / CHAR_BIT)

/*!
 * Given an array `a` and a bitN, it sets that bit
 */
#define BS_SETBIT(a, bitN) ((a)[_BS_BITSLOT(bitN)] |= _BS_BITMASK(bitN))

/*!
 * Given an array `a` and a bitN, it clears that bit
 */
#define BS_CLEARBIT(a, bitN) ((a)[_BS_BITSLOT(bitN)] &= ~_BS_BITMASK(bitN))

/*!
 * Checks if bitN in array `a` is set
 */
#define BS_ISBITSET(a, bitB) ((a)[_BS_BITSLOT(bitB)] & _BS_BITMASK(bitB))

#endif
