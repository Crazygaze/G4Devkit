#include "testframework/testframework.h"
#include "utils/queue.h"
#include <stdlib.h>
#include <stdc_init.h>
#include <string.h>

typedef struct QStats {
	uint32_t memBefore;
	uint32_t memAfter;
} QStats;
static QStats gQStats;
#define QMEM_START                        \
	memset(&gQStats, 0, sizeof(gQStats)); \
	_mem_getstats(&gQStats.memBefore, NULL, NULL);

#define QMEM_END                                  \
	_mem_getstats(&gQStats.memAfter, NULL, NULL); \
	CHECK(gQStats.memBefore == gQStats.memAfter);

typedef struct QNode {
	int id;
} QNode;

QUEUE_TYPEDECLARE(QNode)

static bool qremove(const QNode* a, uint32_t cookie)
{
	return a->id == (int)cookie;
}

static void pushNode(Queue_QNode* q, int id)
{
	QNode n = { id };
	int size = queue_QNode_size(q);
	CHECK(queue_QNode_push(q, &n));
	CHECK(queue_QNode_size(q) == (size + 1));
}

static void popNode(Queue_QNode* q, int expectedId)
{
	QNode n = { 0 };
	int size = queue_QNode_size(q);

	// 0 means we expect the queue to be empty, and thus a pop should fail
	if (expectedId == 0) {
		CHECK(queue_QNode_pop(q, &n) == false);
		CHECK(queue_QNode_size(q) == size);
	} else {
		CHECK(queue_QNode_pop(q, &n));
		CHECK(n.id == expectedId);
		CHECK(queue_QNode_size(q) == size - 1);
	}
}

static void checkQueueInternals(Queue_QNode* q, int* ids)
{
	// Our queue internally uses the equivalent of an int array, so we can use
	// that for easier testing
	int* a = (int*)(q->data);
	int size = queue_QNode_size(q);

	int i = 0;
	while (*ids) {
		CHECK(i < size);

		// Putting it in a variable, so we can see it in the debugger
		QNode* n = (QNode*)(&a[(q->head + i) % q->capacity]);
		CHECK(n->id == *ids);
		//LOG_LOG("id = %d", *ids);

		i++;
		ids++;
	}

	CHECK(i == size);
}
TEST(queue_create)
{
	QMEM_START

	Queue_QNode q;
	{
		CHECK(queue_QNode_create(&q, 0));
		CHECK(q.data == NULL);
		CHECK(q.capacity == 0);
		CHECK(q.tail == 0);
		CHECK(q.head == 0);
		CHECK(q.elementSize == 4);
		queue_QNode_destroy(&q);
	}

	{
		CHECK(queue_QNode_create(&q, 10));
		CHECK(q.data != NULL);
		
		// +1, because the implementation uses an extra empty slot
		CHECK(q.capacity == 11);
		
		CHECK(q.tail == 0);
		CHECK(q.head == 0);
		CHECK(q.elementSize == 4);
		queue_QNode_destroy(&q);
	}

	QMEM_END
}

TEST(queue_size)
{
	QMEM_START

	Queue_QNode q;
	CHECK(queue_QNode_create(&q, 0));

	CHECK(queue_QNode_isEmpty(&q));
	CHECK(queue_QNode_size(&q) == 0);

	pushNode(&q, 1);

	CHECK(queue_QNode_isEmpty(&q) == false);
	CHECK(queue_QNode_size(&q) == 1);

	queue_QNode_destroy(&q);
	QMEM_END
}

TEST(queue_reserve)
{
	QMEM_START

	Queue_QNode q;
	CHECK(queue_QNode_create(&q, 0));
	CHECK(q.capacity == 0 && q.data == NULL);

	CHECK(queue_QNode_reserve(&q, 2));
	
	// +1 because the implementation needs an empty slot
	CHECK(q.capacity == 2 + 1 && q.data);

	queue_QNode_destroy(&q);
	QMEM_END
}

TEST(queue_push)
{
	QMEM_START

	Queue_QNode q;
	CHECK(queue_QNode_create(&q, 0));
	CHECK(q.capacity == 0 && q.data == NULL);

	pushNode(&q, 1);
	checkQueueInternals(&q, (int[]){ 1, 0 });

	CHECK((q.capacity == 16 + 1) && (q.data));
	for (int i = 0; i < 14; ++i) {
		pushNode(&q, i + 2);
	}
	CHECK((q.capacity == 16 + 1) && (q.data)); // No change in capacity
	CHECK(queue_QNode_size(&q) == 15);

	// Test pushEmpty
	queue_QNode_pushEmpty(&q)->id = 16;
	CHECK((q.capacity == 16 + 1) && (q.data)); // No change in capacity
	CHECK(queue_QNode_size(&q) == 16);

	checkQueueInternals(&q, (int[]){ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
									 14, 15, 16, 0 });

	pushNode(&q, 17);
	CHECK((q.capacity == 32 + 1) && (q.data)); // Should double the capacity
	checkQueueInternals(&q, (int[]){ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
									 14, 15, 16, 17, 0 });

	queue_QNode_destroy(&q);
	QMEM_END
}

TEST(queue_pop)
{
	QMEM_START

	Queue_QNode q;

	//
	// Test with queue with capacity 0
	//
	CHECK(queue_QNode_create(&q, 0));
	popNode(&q, 0);
	queue_QNode_destroy(&q);

	//
	// Test starting with capacity set
	CHECK(queue_QNode_create(&q, 4));
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
	queue_QNode_reserve(&q, 16);

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
	CHECK(queue_QNode_pop(&q, NULL));
	checkQueueInternals(&q, (int[]){ 0 });

	popNode(&q, 0);

	queue_QNode_destroy(&q);
	QMEM_END
}

TEST(queue_peek)
{
	QMEM_START

	Queue_QNode q;
	CHECK(queue_QNode_create(&q, 0));

	CHECK(queue_QNode_peek(&q) == NULL);

	CHECK(queue_QNode_reserve(&q, 1));
	pushNode(&q, 1);
	pushNode(&q, 2);
	CHECK(queue_QNode_peek(&q)->id == 1);
	CHECK(queue_QNode_pop(&q, NULL));

	CHECK(queue_QNode_peek(&q)->id == 2);
	CHECK(queue_QNode_pop(&q, NULL));

	CHECK(queue_QNode_peek(&q) == NULL);

	queue_QNode_destroy(&q);
	QMEM_END
}

TEST(queue_clear)
{
	QMEM_START

	Queue_QNode q;
	CHECK(queue_QNode_create(&q, 0));
	pushNode(&q, 1);
	pushNode(&q, 2);

	queue_QNode_clear(&q);
	CHECK(queue_QNode_isEmpty(&q));

	queue_QNode_destroy(&q);
	QMEM_END
}

TEST(queue_remove)
{
	QMEM_START

	Queue_QNode q;
	CHECK(queue_QNode_create(&q, 0));

	pushNode(&q, 1);
	pushNode(&q, 1);
	pushNode(&q, 2);
	pushNode(&q, 3);
	pushNode(&q, 3);
	pushNode(&q, 4);
	pushNode(&q, 4);
	pushNode(&q, 1);
	checkQueueInternals(&q, (int[]){ 1, 1, 2, 3, 3, 4, 4, 1, 0 });

	CHECK(queue_QNode_remove(&q, qremove, 1) == 3);
	checkQueueInternals(&q, (int[]){ 2, 3, 3, 4, 4, 0 });

	CHECK(queue_QNode_remove(&q, qremove, 5) == 0);

	// Nothing should change
	checkQueueInternals(&q, (int[]){ 2, 3, 3, 4, 4, 0 });

	CHECK(queue_QNode_remove(&q, qremove, 3) == 2);
	checkQueueInternals(&q, (int[]){ 2, 4, 4, 0 });

	CHECK(queue_QNode_remove(&q, qremove, 2) == 1);
	checkQueueInternals(&q, (int[]){ 4, 4, 0 });

	CHECK(queue_QNode_remove(&q, qremove, 4) == 2);
	checkQueueInternals(&q, (int[]){ 0 });
	CHECK(queue_QNode_remove(&q, qremove, 4) == 0);
	CHECK(queue_QNode_isEmpty(&q));

	queue_QNode_destroy(&q);
	QMEM_END
}

TEST(queue_getAt)
{
	QMEM_START

	Queue_QNode q;
	CHECK(queue_QNode_create(&q, 0));

	for (int i = 1; i <= 4; i++) {
		pushNode(&q, i);
	}

	checkQueueInternals(&q, (int[]){ 1, 2, 3, 4, 0 });
	for (int i = 1; i <= 4; i++) {
		CHECK(queue_QNode_getAt(&q, i - 1)->id == i);
	}

	queue_QNode_destroy(&q);
	QMEM_END
}

void queue_tests()
{
	queue_create_tests();
	queue_size_tests();
	queue_reserve_tests();
	queue_push_tests();
	queue_pop_tests();
	queue_peek_tests();
	queue_clear_tests();
	queue_remove_tests();
	queue_getAt_tests();
}
