#include "testframework/testframework.h"
#include "utils/bitops.h"

// Putting these in a function to force code to be generated and thus we can
// can test the compiler a bit more instead of the preprocessor
static int do_ALIGN(int x, int a)
{
	return ALIGN(x, a);
}
TEST(ALIGN)
{
	//
	// The ALIGN macro is only for powers of two
	// Putting ALIGN in a function so that we can actually test the compiler and
	// not the preprocessor
	//
	CHECK(do_ALIGN(1, 4) == 4);
	CHECK(do_ALIGN(4, 4) == 4);
	CHECK(do_ALIGN(5, 4) == 8);
	CHECK(do_ALIGN(0, 4) == 0);

	// non-power-of-two alignment
	CHECK(align(3, 0) == 3);
	CHECK(align(3, 1) == 3);
	CHECK(align(0, 3) == 0);
	CHECK(align(6, 5) == 10);
}

bool do_ISALIGNED(int x, int a)
{
	return ISALIGNED(x, a);
}
TEST(ISALIGNED)
{
	// The ISALIGNED macro is only for powers of two
	CHECK(do_ISALIGNED(1, 4) == false);
	CHECK(do_ISALIGNED(4, 4) == true);
	CHECK(do_ISALIGNED(5, 4) == false);
	CHECK(do_ISALIGNED(8, 4) == true);
}

static uint32_t do_GETBITS(uint32_t val, int H, int L)
{
	return GETBITS(val, H, L);
}
TEST(GETBITS)
{
	CHECK(do_GETBITS(0xF0000000, 31, 31) == 0b1);
	CHECK(do_GETBITS(0xF0000000, 31, 30) == 0b11);
	CHECK(do_GETBITS(0xF0000000, 31, 28) == 0b1111);
	CHECK(do_GETBITS(0xF0000000, 31, 27) == 0b11110);

	CHECK(do_GETBITS(0xF0000005, 0, 0) == 0b001);
	CHECK(do_GETBITS(0xF0000005, 1, 0) == 0b001);
	CHECK(do_GETBITS(0xF0000005, 2, 0) == 0b101);
}

static uint32_t do_ISBITSET(uint32_t val, int B)
{
	return ISBITSET(val, B);
}
TEST(ISBITSET)
{
	CHECK(ISBITSET( 0x80000000, 31));
	CHECK(!ISBITSET(0x80000000, 30));
	CHECK(ISBITSET( 0x00000001,  0));
	CHECK(!ISBITSET(0x00000002,  0));
}

static uint32_t do_MAKEMASK(int H, int L)
{
	return MAKEMASK(H, L);
}

uint32_t gMAKEMASKTmp;
TEST(MAKEMASK)
{
	// We test both the compiler and the preprocessor:
	// - do_MAKEMASK tests if the compiler generates the right thing, since H
	// and L are not immediates.
	// - By using MAKEMASK directly, we test the preprocessor
	CHECK(do_MAKEMASK(31,0) == 0xFFFFFFFF);
	CHECK(   MAKEMASK(31,0) == 0xFFFFFFFF);
	
	CHECK(do_MAKEMASK(31,28) == 0xF0000000);
	CHECK(   MAKEMASK(31,28) == 0xF0000000);
	
	CHECK(do_MAKEMASK(3,0) == 0xF);
	CHECK(   MAKEMASK(3,0) == 0xF);
	
	CHECK(do_MAKEMASK(0,0) == 0x1);
	CHECK(   MAKEMASK(0,0) == 0x1);
}

static uint32_t do_SETBIT(uint32_t val, int B)
{
	return SETBIT(val, B);
}
static uint32_t do_CLEARBIT(uint32_t val, int B)
{
	return CLEARBIT(val, B);
}
static uint32_t do_SETBIT_TO(uint32_t val, int B, uint32_t state)
{
	return SETBIT_TO(val, B, state);
}
//static void bitops_tests_SET_and_CLEAR(void)
TEST(SET_and_CLEAR)
{
	CHECK(do_SETBIT(0x0F000000,  0) == 0x0F000001);
	CHECK(do_SETBIT(0x0F000000,  1) == 0x0F000002);
	CHECK(do_SETBIT(0x0F000000, 31) == 0x8F000000);

	CHECK(do_CLEARBIT(0x8F000003,  0) == 0x8F000002);
	CHECK(do_CLEARBIT(0x8F000003,  1) == 0x8F000001);
	CHECK(do_CLEARBIT(0x8F000003, 31) == 0x0F000003);

	CHECK(do_SETBIT_TO(0x0F000000,  0, 0) == 0x0F000000);
	CHECK(do_SETBIT_TO(0x0F000000,  0, 1) == 0x0F000001);
	CHECK(do_SETBIT_TO(0x0F000000,  1, 0) == 0x0F000000);
	CHECK(do_SETBIT_TO(0x0F000000,  1, 1) == 0x0F000002);
	CHECK(do_SETBIT_TO(0x0F000000, 31, 0) == 0x0F000000);
	CHECK(do_SETBIT_TO(0x0F000000, 31, 1) == 0x8F000000);
}

static uint32_t do_ZEROBITS(uint32_t val, int H, int L)
{
	return ZEROBITS(val, H, L);
}
TEST(ZEROBITS)
{
	CHECK(do_ZEROBITS(0xFFFFFFFF, 31,31) == 0x7FFFFFFF);
	CHECK(   ZEROBITS(0xFFFFFFFF, 31,31) == 0x7FFFFFFF);
	
	CHECK(do_ZEROBITS(0xFFFFFFFF, 27,24) == 0xF0FFFFFF);
	CHECK(   ZEROBITS(0xFFFFFFFF, 27,24) == 0xF0FFFFFF);
	
	CHECK(do_ZEROBITS(0xFFFFFFFF, 3,0) == 0xFFFFFFF0);
	CHECK(   ZEROBITS(0xFFFFFFFF, 3,0) == 0xFFFFFFF0);
	
	CHECK(do_ZEROBITS(0xFFFFFFFF, 0,0) == 0xFFFFFFFE);
	CHECK(   ZEROBITS(0xFFFFFFFF, 0,0) == 0xFFFFFFFE);
}

static uint32_t do_SETBITS(uint32_t val, int H, int L, uint32_t bits)
{
	return SETBITS(val, H, L, bits);
}
TEST(SETBITS)
{
	// We set 4 bits to 0x6, because 0x6 in binary is 0b0110 and so we can
	// check for those zeros on the borders to.
	
	CHECK(do_SETBITS(0xFFFFFFFF, 31, 28, 0x6) == 0x6FFFFFFF);
	CHECK(   SETBITS(0xFFFFFFFF, 31, 28, 0x6) == 0x6FFFFFFF);
	
	CHECK(do_SETBITS(0xFFFFFFFF, 27, 24, 0x6) == 0xF6FFFFFF);
	CHECK(   SETBITS(0xFFFFFFFF, 27, 24, 0x6) == 0xF6FFFFFF);
	
	CHECK(do_SETBITS(0xFFFFFFFF, 3, 0, 0x6) == 0xFFFFFFF6);
	CHECK(   SETBITS(0xFFFFFFFF, 3, 0, 0x6) == 0xFFFFFFF6);
}

TEST(log2u)
{
	// log2(0) is INFINITY, but this is an integer function, so lets stick with
	// accepting 0
	CHECK(log2u(0) == 0);
	CHECK(log2u(1) == 0);
	CHECK(log2u(2) == 1);
	CHECK(log2u(256) == 8);
	CHECK(log2u(255) == 7);
}

TEST(isPowerOfTwo)
{
	// 0 is not a power of two
	CHECK(isPowerOfTwo(0) == false);
	CHECK(isPowerOfTwo(1) == true);
	CHECK(isPowerOfTwo(2) == true);
	CHECK(isPowerOfTwo(5) == false);
	CHECK(isPowerOfTwo(8) == true);
}

TEST(roundUpToPowerOfTwo)
{
	// Make sure if it handles the `0` edge case.
	// Some algorithms don't care for this edge case and thus don't handle it
	// correctly
	CHECK(roundUpToPowerOfTwo(0) == 1);

	CHECK(roundUpToPowerOfTwo(1) == 1);
	CHECK(roundUpToPowerOfTwo(2) == 2);
	CHECK(roundUpToPowerOfTwo(5) == 8);
}

void bitops_tests(void)
{
	ALIGN_tests();
	ISALIGNED_tests();
	GETBITS_tests();
	ISBITSET_tests();
	MAKEMASK_tests();
	SET_and_CLEAR_tests();
	ZEROBITS_tests();
	SETBITS_tests();
	log2u_tests();
	isPowerOfTwo_tests();
	roundUpToPowerOfTwo_tests();
}
