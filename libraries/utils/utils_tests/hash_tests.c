#include "testframework/testframework.h"
#include "utils/hash.h"

// Values for these tested were copied from https://md5calc.com/hash

TEST(fnv)
{
	const char* str = "Hello World!";

	// str
	uint32_t hval = fnv_32a_str(str, FNV1_32A_INIT);
	CHECK(hval == 0xb1ea4872);
	// Test appending
	CHECK(fnv_32a_str(str, hval) == 0x6b5b4eb9);

	// buffer
	hval = fnv_32a_buf(str, 12, FNV1_32A_INIT);
	CHECK(hval == 0xb1ea4872);
	// Test appending
	CHECK(fnv_32a_buf(str, 12, hval) == 0x6b5b4eb9);
}

void hash_tests(void)
{
	fnv_tests();
}

