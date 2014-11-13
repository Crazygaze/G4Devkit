/*
 * Macros to deal with bitsets
 * Based on http://c-faq.com/misc/bitsets.html
 *
 * Here are some usage examples. To declare an ``array'' of 47 bits:
 *
 * char bitarray[BITNSLOTS(47)];
 * To set the 23rd bit:
 *	BITSET(bitarray, 23);
 * To test the 35th bit:
 * 	if(BITTEST(bitarray, 35)) ...
 * To compute the union of two bit arrays and place it in a third array (with all three arrays declared as above):
 * 	for(i = 0; i < BITNSLOTS(47); i++)
 * 		array3[i] = array1[i] | array2[i];
 * To compute the intersection, use & instead of |. 
 */ 
 
#ifndef _APCPUOS_BITSET_H_
#define _APCPUOS_BITSET_H_

#include "utilsharedconfig.h"
#include <limits_shared.h>

#define BITMASK(b) (1 << ((b) % CHAR_BIT))
#define BITSLOT(b) ((b) / CHAR_BIT)
#define BITSET(a, b) ((a)[BITSLOT(b)] |= BITMASK(b))
#define BITCLEAR(a, b) ((a)[BITSLOT(b)] &= ~BITMASK(b))
#define BITTEST(a, b) ((a)[BITSLOT(b)] & BITMASK(b))
#define BITNSLOTS(nb) ((nb + CHAR_BIT - 1) / CHAR_BIT)


#endif

