#ifndef _utils_tests_h_
#define _utils_tests_h_

#include "testframework/testframework.h"
#include <stdlib.h>

typedef struct TestStats {
	uint32_t memBefore;
	uint32_t memAfter;
} TestStats;
extern TestStats gTestStats;

#define TEST_MEM_START        \
	gTestStats.memBefore = 0; \
	gTestStats.memAfter = 0;  \
	_mem_getstats(&gTestStats.memBefore, NULL, NULL);

#define TEST_MEM_END                                 \
	_mem_getstats(&gTestStats.memAfter, NULL, NULL); \
	CHECK(gTestStats.memBefore == gTestStats.memAfter);

#endif
