#include "testframework/testframework.h"
#include "stdc_init.h"
#include <stdlib.h>
#include <string.h>

static void single_itoa_test(int val, int base, const char* expected)
{
	char buf[33];
	memset(buf, 'x', sizeof(buf));
	const char* res = itoa(val, buf, base);
	CHECK(res == buf);
	CHECK(strcmp(res, expected) == 0);
}

TEST(itoa)
{
	single_itoa_test(-1, 16, "ffffffff");

	single_itoa_test(5, 2, "101");
	single_itoa_test(-5, 2, "11111111111111111111111111111011");

	single_itoa_test(1234, 10, "1234");
	single_itoa_test(-1234, 10, "-1234");
	single_itoa_test((int)0xFABCDEFA, 16, "fabcdefa");
	single_itoa_test(0x7FBCDEFA, 16, "7fbcdefa");
}

static void single_ftoa_test(double val, int precision, const char* expected)
{
	char buf[33];
	memset(buf, 'x', sizeof(buf));
	const char* res = ftoa(val, buf, precision);
	CHECK(res == buf);
	CHECK(strcmp(res, expected) == 0);
}

TEST(ftoa)
{
	single_ftoa_test(1.521f,  0, "1");
	single_ftoa_test(1.521f,  1, "1.5");
	single_ftoa_test(1.521f,  2, "1.52");
	single_ftoa_test(1.521f,  3, "1.521");
	single_ftoa_test(1.521f,  4, "1.5210");
	single_ftoa_test(1.521f, -1, "1.52100"); // Automatic precision

	single_ftoa_test(-1.521f,  0, "-1");
	single_ftoa_test(-1.521f,  1, "-1.5");
	single_ftoa_test(-1.521f,  2, "-1.52");
	single_ftoa_test(-1.521f,  3, "-1.521");
	single_ftoa_test(-1.521f,  4, "-1.5210");
	single_ftoa_test(-1.521f, -1, "-1.52100"); // Automatic precision

	single_ftoa_test(0.521f,  0, "0");
	single_ftoa_test(0.521f,  1, "0.5");
	single_ftoa_test(0.521f,  2, "0.52");
	single_ftoa_test(0.521f,  3, "0.521");
	single_ftoa_test(0.521f,  4, "0.5210");
	single_ftoa_test(0.521f, -1, "0.521000"); // Automatic precision

	single_ftoa_test(-0.521f,  0, "-0");
	single_ftoa_test(-0.521f,  1, "-0.5");
	single_ftoa_test(-0.521f,  2, "-0.52");
	single_ftoa_test(-0.521f,  3, "-0.521");
	single_ftoa_test(-0.521f,  4, "-0.5210");
	single_ftoa_test(-0.521f, -1, "-0.521000"); // Automatic precision
}

void single_strtol_test(const char* str, int base, long expected)
{
	char* str_end;
	long r = strtol(str, &str_end, base);
	CHECK(r == expected);
	CHECK(*str_end == ' ');
}

TEST(strtol)
{
	single_strtol_test("0 "  , 10,   0);
	single_strtol_test("101 ",  2,   5);
	single_strtol_test("101 ", 10, 101);
	single_strtol_test("ff " , 16, 255);

	single_strtol_test("-101 ",  2,   -5);
	single_strtol_test("-101 ", 10, -101);
	single_strtol_test("-ff " , 16, -255);

	// Prefix 0 will use base 8
	single_strtol_test("012 " , 0,  10);
	single_strtol_test("012 " , 8,  10);
	single_strtol_test("-012 ", 0, -10);
	single_strtol_test("-012 ", 8, -10);

	// Prefix 0x or 0X will use base 16
	single_strtol_test("0x12 " ,  0,  0x12);
	single_strtol_test("0x12 " , 16,  0x12);
	single_strtol_test("-0X12 ",  0, -0x12);
	single_strtol_test("-0X12 ", 16, -0x12);

	// Test all possible white-space characters
	single_strtol_test(" \f\n\r\t\v123 ", 10, 123);
}

void single_strtoul_test(const char* str, int base, unsigned long expected)
{
	char* str_end;
	unsigned long r = strtoul(str, &str_end, base);
	CHECK(r == expected);
	CHECK(*str_end == ' ');
}

TEST(strtoul)
{
	single_strtoul_test("0 "  , 10,  0);
	single_strtoul_test("101 ",  2,  5);
	single_strtoul_test("101 ", 10, 101);
	single_strtoul_test("ff " , 16, 255);

	single_strtoul_test("-101 ",  2, (unsigned long)  -5);
	single_strtoul_test("-101 ", 10, (unsigned long)-101);
	single_strtoul_test("-ff " , 16, (unsigned long)-255);

	// Prefix 0 will use base 8
	single_strtoul_test("012 " , 0,                 10);
	single_strtoul_test("012 " , 8,                 10);
	single_strtoul_test("-012 ", 0, (unsigned long)-10);
	single_strtoul_test("-012 ", 8, (unsigned long)-10);

	// Prefix 0x or 0X will use base 16
	single_strtoul_test("0x12 " ,  0,                 0x12);
	single_strtoul_test("0x12 " , 16,                 0x12);
	single_strtoul_test("-0X12 ",  0, (unsigned long)-0x12);
	single_strtoul_test("-0X12 ", 16, (unsigned long)-0x12);
}

int single_qsort_test_cmp(const void* _a, const void* _b)
{
	const char* a = _a;
	const char* b = _b;

	if (*a < *b)
		return -1;
	else if (*a > *b)
		return 1;
	else
		return 0;
}

void single_qsort_test(const char* initial, const char* expected)
{
	char buf[64];
	memcpy(buf, initial, strlen(initial) + 1);
	qsort(buf, strlen(buf) / 2, 2, single_qsort_test_cmp);

	CHECK(strcmp(buf, expected) == 0);
}

TEST(qsort)
{
	// NOTE: To make the test easier to code, we use strings, but with an
	// element size of 2, therefore the extra space after each digit.
	single_qsort_test("", "");
	single_qsort_test("0 ", "0 ");
	single_qsort_test("0 1 ", "0 1 ");
	single_qsort_test("1 0 ", "0 1 ");
	single_qsort_test("1 0 2 3 1 ", "0 1 1 2 3 ");
}

void stdlib_tests(void)
{
	itoa_tests();
	ftoa_tests();
	strtol_tests();
	strtoul_tests();
	qsort_tests();
}