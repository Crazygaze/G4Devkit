#include "utils_tests.h"
#include "utils/queue32.h"

static void pushNode(Queue32* q, uint32_t id)
{
	int size = queue32_size(q);
	CHECK(queue32_push(q, id));
	CHECK(queue32_size(q) == (size + 1));
}

static void popNode(Queue32* q, uint32_t expectedId)
{
	uint32_t n;
	int size = queue32_size(q);

	// 0 means we expect the queue to be empty, and thus a pop should fail
	if (expectedId == 0) {
		CHECK(queue32_pop(q, &n) == false);
		CHECK(queue32_size(q) == size);
	} else {
		CHECK(queue32_pop(q, &n));
		CHECK(n == expectedId);
		CHECK(queue32_size(q) == size - 1);
	}
}

static void checkQueueInternals(Queue32* q, uint32_t* ids)
{
	// Our queue internally uses the equivalent of an uint32_t array, so we can
	// use that for easier testing.
	uint32_t* a = q->data;
	int size = queue32_size(q);

	int i = 0;
	while (*ids) {
		CHECK(i < size);

		// Putting it in a variable, so we can see it in the debugger
		uint32_t n = a[(q->head + i) % q->capacity];
		CHECK(n == *ids);
		// LOG_LOG("id = %d", *ids);

		i++;
		ids++;
	}

	CHECK(i == size);
}

TEST(queue32_create)
{
	TEST_MEM_START

	Queue32 q;
	{
		CHECK(queue32_create(&q, 0));
		CHECK(q.data == NULL);
		CHECK(q.capacity == 0);
		CHECK(q.tail == 0);
		CHECK(q.head == 0);
		queue32_destroy(&q);
	}

	{
		CHECK(queue32_create(&q, 10));
		CHECK(q.data != NULL);

		// +1, because the implementation uses an extra empty slot
		CHECK(q.capacity == 11);

		CHECK(q.tail == 0);
		CHECK(q.head == 0);
		queue32_destroy(&q);
	}

	TEST_MEM_END
}

TEST(queue32_size)
{
	TEST_MEM_START

	Queue32 q;
	CHECK(queue32_create(&q, 0));

	CHECK(queue32_isEmpty(&q));
	CHECK(queue32_size(&q) == 0);

	pushNode(&q, 1);

	CHECK(queue32_isEmpty(&q) == false);
	CHECK(queue32_size(&q) == 1);

	queue32_destroy(&q);

	TEST_MEM_END
}

TEST(queue32_reserve)
{
	TEST_MEM_START

	Queue32 q;
	CHECK(queue32_create(&q, 0));
	CHECK(q.capacity == 0 && q.data == NULL);

	CHECK(queue32_reserve(&q, 2));

	// +1 because the implementation needs an empty slot
	CHECK(q.capacity == 2 + 1 && q.data);

	queue32_destroy(&q);

	TEST_MEM_END
}

TEST(queue32_push)
{
	TEST_MEM_START

	Queue32 q;
	CHECK(queue32_create(&q, 0));
	CHECK(q.capacity == 0 && q.data == NULL);

	pushNode(&q, 1);
	checkQueueInternals(&q, (int[]){ 1, 0 });

	CHECK((q.capacity == 16 + 1) && (q.data));
	for (int i = 0; i < 14; ++i) {
		pushNode(&q, i + 2);
	}
	CHECK((q.capacity == 16 + 1) && (q.data)); // No change in capacity
	CHECK(queue32_size(&q) == 15);

	queue32_push(&q, 16);
	CHECK((q.capacity == 16 + 1) && (q.data)); // No change in capacity
	CHECK(queue32_size(&q) == 16);

	checkQueueInternals(&q, (int[]){ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
									 14, 15, 16, 0 });

	pushNode(&q, 17);
	CHECK((q.capacity == 32 + 1) && (q.data)); // Should double the capacity
	checkQueueInternals(&q, (int[]){ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
									 14, 15, 16, 17, 0 });

	queue32_destroy(&q);

	TEST_MEM_END
}

TEST(queue32_pop)
{
	TEST_MEM_START

	Queue32 q;

	//
	// Test with queue with capacity 0
	//
	CHECK(queue32_create(&q, 0));
	popNode(&q, 0);
	queue32_destroy(&q);

	//
	// Test starting with capacity set
	CHECK(queue32_create(&q, 4));
	CHECK(q.capacity == 4 + 1);
	pushNode(&q, 1);
	pushNode(&q, 2);
	pushNode(&q, 3);
	pushNode(&q, 4);
	checkQueueInternals(&q, (int[]){ 1, 2, 3, 4, 0 });

	popNode(&q, 1);
	checkQueueInternals(&q, (int[]){ 2, 3, 4, 0 });

	pushNode(&q, 5);
	checkQueueInternals(&q, (int[]){ 2, 3, 4, 5, 0 });

	popNode(&q, 2);
	checkQueueInternals(&q, (int[]){ 3, 4, 5, 0 });

	pushNode(&q, 6);
	checkQueueInternals(&q, (int[]){ 3, 4, 5, 6, 0 });

	// Test if reserve copies things properly, since tail and head have now
	// moved.
	// This CHECK is only to make sure we are in the right scenario to test the
	// reserve call.
	CHECK(q.tail == 1 && q.head == 2);
	queue32_reserve(&q, 16);

	// Check if reserve reset head/tail, since it copies things around so head
	// is again index 0.
	CHECK(q.tail == 4 && q.head == 0);
	// Contents should have not changed
	checkQueueInternals(&q, (int[]){ 3, 4, 5, 6, 0 });

	popNode(&q, 3);
	checkQueueInternals(&q, (int[]){ 4, 5, 6, 0 });
	popNode(&q, 4);
	checkQueueInternals(&q, (int[]){ 5, 6, 0 });
	popNode(&q, 5);
	checkQueueInternals(&q, (int[]){ 6, 0 });

	popNode(&q, 6);
	checkQueueInternals(&q, (int[]){ 0 });

	// Test pop&drop
	pushNode(&q, 7);
	checkQueueInternals(&q, (int[]){ 7, 0 });
	CHECK(queue32_pop(&q, NULL));
	checkQueueInternals(&q, (int[]){ 0 });

	popNode(&q, 0);

	queue32_destroy(&q);

	TEST_MEM_END
}

TEST(queue32_peek)
{
	TEST_MEM_START

	Queue32 q;
	CHECK(queue32_create(&q, 0));

	uint32_t val = 0xFFFFFFFF;
	CHECK(!queue32_peek(&q, &val));

	CHECK(queue32_reserve(&q, 1));
	pushNode(&q, 1);
	pushNode(&q, 2);
	
	// Peek and pop the next element
	CHECK(queue32_peek(&q, &val));
	CHECK(val == 1);
	CHECK(queue32_pop(&q, NULL)); // drop
	
	// Peek and pop the next element
	CHECK(queue32_peek(&q, &val));
	CHECK(val == 2);
	CHECK(queue32_pop(&q, NULL)); // drop

	CHECK(!queue32_peek(&q, &val));

	queue32_destroy(&q);
	
	TEST_MEM_END
}

TEST(queue32_clear)
{
	TEST_MEM_START

	Queue32 q;
	CHECK(queue32_create(&q, 0));
	pushNode(&q, 1);
	pushNode(&q, 2);

	queue32_clear(&q);
	CHECK(queue32_isEmpty(&q));

	queue32_destroy(&q);
	
	TEST_MEM_END
}

TEST(queue32_remove)
{
	TEST_MEM_START

	Queue32 q;
	CHECK(queue32_create(&q, 0));

	pushNode(&q, 1);
	pushNode(&q, 1);
	pushNode(&q, 2);
	pushNode(&q, 3);
	pushNode(&q, 3);
	pushNode(&q, 4);
	pushNode(&q, 4);
	pushNode(&q, 1);
	checkQueueInternals(&q, (int[]){ 1, 1, 2, 3, 3, 4, 4, 1, 0 });

	CHECK(queue32_remove(&q, 1) == 3);
	checkQueueInternals(&q, (int[]){ 2, 3, 3, 4, 4, 0 });

	CHECK(queue32_remove(&q, 5) == 0);

	// Nothing should change
	checkQueueInternals(&q, (int[]){ 2, 3, 3, 4, 4, 0 });

	CHECK(queue32_remove(&q, 3) == 2);
	checkQueueInternals(&q, (int[]){ 2, 4, 4, 0 });

	CHECK(queue32_remove(&q, 2) == 1);
	checkQueueInternals(&q, (int[]){ 4, 4, 0 });

	CHECK(queue32_remove(&q, 4) == 2);
	checkQueueInternals(&q, (int[]){ 0 });
	CHECK(queue32_remove(&q, 4) == 0);
	CHECK(queue32_isEmpty(&q));

	queue32_destroy(&q);

	TEST_MEM_END
}

TEST(queue32_getAt)
{
	TEST_MEM_START

	Queue32 q;
	CHECK(queue32_create(&q, 0));

	for (int i = 1; i <= 4; i++) {
		pushNode(&q, i);
	}

	checkQueueInternals(&q, (int[]){ 1, 2, 3, 4, 0 });
	for (int i = 1; i <= 4; i++) {
		CHECK(queue32_getAt(&q, i - 1) == i);
	}

	queue32_destroy(&q);
	
	TEST_MEM_END
}

void queue32_tests()
{
	queue32_create_tests();
	queue32_size_tests();
	queue32_reserve_tests();
	queue32_push_tests();
	queue32_pop_tests();
	queue32_peek_tests();
	queue32_clear_tests();
	queue32_remove_tests();
	queue32_getAt_tests();
}
