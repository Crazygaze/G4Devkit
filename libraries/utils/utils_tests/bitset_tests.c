#include "testframework/testframework.h"
#include "utils/bitset.h"
#include "utils/staticassert.h"
#include <stdint.h>
#include <string.h>

// Putting this in a global variable, otherwise a Release build will optimize
// a bunch of stuff away since the test is pretty much all constants
static uint8_t a[BS_NUMSLOTS(16)];
static int bit1 = 0;
static int bit2 = 7;
static int bit3 = 8;
static int bit4 = 15;

TEST(BS_SETBIT)
{
	uint16_t* all = (uint16_t*)&a;
	STATIC_ASSERT(sizeof(a) == 2);

	BS_SETBIT(a, bit1);
	CHECK(a[0] == 1);

	BS_SETBIT(a, bit2);
	CHECK(a[0] == 1 | 0x80);

	BS_SETBIT(a, bit3);
	CHECK(a[0] == 1 | 0x80);
	CHECK(a[1] == 1);

	BS_SETBIT(a, bit4);
	CHECK(a[0] == 1 | 0x80);
	CHECK(a[1] == 1 | 0x80);

	CHECK(*all == (((1 | 0x80) << 8) | (1 | 0x80)));
}

TEST(BS_CLEARBIT)
{
	memset(&a, 0xFF, sizeof(a));
	uint16_t* all = (uint16_t*)&a;
	CHECK(*all == 0xFFFF);

	BS_CLEARBIT(a, bit1);
	CHECK(*all == 0xFFFE);

	BS_CLEARBIT(a, bit2);
	CHECK(*all == 0xFF7E);

	BS_CLEARBIT(a, bit3);
	CHECK(*all == 0xFE7E);

	BS_CLEARBIT(a, bit4);
	CHECK(*all == 0x7E7E);
}

TEST(BS_ISBITSET)
{
	uint16_t* all = (uint16_t*)&a;
	*all = 0x7E7E;

	CHECK(!BS_ISBITSET(a, bit1));
	CHECK(BS_ISBITSET(a, bit1 + 1));

	CHECK(!BS_ISBITSET(a, bit3));
	CHECK(BS_ISBITSET(a, bit3 + 1));
}

TEST(BS_NUMSLOTS)
{
	CHECK(BS_NUMSLOTS(8) == 1);
	CHECK(BS_NUMSLOTS(9) == 2);
	CHECK(BS_NUMSLOTS(16) == 2);
	CHECK(BS_NUMSLOTS(17) == 3);
}

void bitset_tests(void)
{
	BS_SETBIT_tests();
	BS_CLEARBIT_tests();
	BS_ISBITSET_tests();
	BS_NUMSLOTS_tests();
}
