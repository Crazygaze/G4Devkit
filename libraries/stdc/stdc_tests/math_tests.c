#include "testframework/testframework.h"
#include <math.h>
#include <float.h>

TEST(miscmath)
{
	// isinf
	CHECK(isinf((double)INFINITY));
	CHECK(isinf((float)INFINITY));
	CHECK(!isinf((double)NAN));
	CHECK(!isinf((float)NAN));
	CHECK(!isinf(0));

	// isnan
	CHECK(isnan((double)NAN));
	CHECK(isnan((float)NAN));
	CHECK(isnan((double)(-NAN)));
	CHECK(isnan((float)(-NAN)));
	CHECK(!isnan((double)INFINITY));
	CHECK(!isnan((float)INFINITY));
	CHECK(!isnan((double)(-INFINITY)));
	CHECK(!isnan((float)(-INFINITY)));
	CHECK(!isnan(1.0));

	CHECK(fabs(1) == 1.0);
	CHECK(fabs(-1) == 1.0);
	CHECK(fabsf(1) == 1.0f);
	CHECK(fabsf(-1) == 1.0f);
}

#define DBL_MARGIN 0.000000000000001
#define FLT_MARGIN 0.000001f

// According to https://en.cppreference.com/w/c/numeric/math/log
TEST(log)
{
	CHECK(log(0) == -INFINITY);
	CHECK(log(1) == 0);
	CHECK(isnan(log2(-10)));
	CHECK(log2(INFINITY) == INFINITY);
	CHECK(isnan(log2(NAN)));

	CHECK(test_nearlyEqual(log(255) , 5.54126354515842614625, DBL_MARGIN));
	CHECK(test_nearlyEqual(logf(255), 5.54126354515842614625f, FLT_MARGIN));
}

// According to https://en.cppreference.com/w/c/numeric/math/log2
TEST(log2)
{
	CHECK(log2(0) == -INFINITY);
	CHECK(log2(1) == 0);
	CHECK(isnan(log2(-10)));
	CHECK(log2(INFINITY) == INFINITY);
	CHECK(isnan(log2(NAN)));

	CHECK(test_nearlyEqual(log2(255) , 7.99435343685885793758, DBL_MARGIN));
	CHECK(test_nearlyEqual(log2f(255), 7.99435343685885793758f, FLT_MARGIN));
}

void math_tests(void)
{
	miscmath_tests();
	log_tests();
	log2_tests();
}
