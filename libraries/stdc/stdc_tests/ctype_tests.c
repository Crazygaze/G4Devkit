#include "testframework/testframework.h"
#include "stdc_init.h"
#include <ctype.h>

TEST(isupper)
{
	CHECK(isupper('A') == 1);
	CHECK(isupper('Z') == 1);
	CHECK(isupper('a') == 0);
	CHECK(isupper('z') == 0);
	CHECK(isupper('1') == 0);
}

TEST(isalpha)
{
	CHECK(isalpha('A') == 1);
	CHECK(isalpha('Z') == 1);
	CHECK(isalpha('a') == 1);
	CHECK(isalpha('z') == 1);
	CHECK(isalpha('1') == 0);
}

TEST(isspace)
{
	// https://en.cppreference.com/w/c/string/byte/isspace
	CHECK(isspace(' ') == 1);
	CHECK(isspace('\f') == 1);
	CHECK(isspace('\n') == 1);
	CHECK(isspace('\r') == 1);
	CHECK(isspace('\t') == 1);
	CHECK(isspace('\v') == 1);
}

TEST(isdigit)
{
	CHECK(isdigit('0' - 1) == 0);
	CHECK(isdigit('0') == 1);
	CHECK(isdigit('9') == 1);
	CHECK(isdigit('9' + 1) == 0);
}

void ctype_tests(void)
{
	isupper_tests();
	isalpha_tests();
	isspace_tests();
	isdigit_tests();
}
