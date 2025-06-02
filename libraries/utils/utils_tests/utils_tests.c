#include "testframework/testframework.h"
#include "utils_tests.h"

void bitops_tests(void);
void bitset_tests(void);
void dynamicarray_tests(void);
void hash_tests(void);
void linkedlist_tests(void);
void priorityqueue_tests(void);
void queue_tests(void);
void queue32_tests(void);

TestStats gTestStats;

int main(void)
{
	bitops_tests();
	bitset_tests();
	dynamicarray_tests();
	hash_tests();
	linkedlist_tests();
	priorityqueue_tests();
	queue_tests();
	queue32_tests();

	test_printXY(4, 0, "**** TESTS FINISHED ****");
	return 0;
}

