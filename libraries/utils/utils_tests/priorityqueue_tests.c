#include "testframework/testframework.h"
#include "utils/priorityqueue.h"
#include <stdlib.h>
#include <stdc_init.h>

typedef struct PQStats {
	uint32_t memBefore;
	uint32_t memAfter;
} PQStats;
static PQStats gPQStats;
#define PQMEM_START                         \
	memset(&gPQStats, 0, sizeof(gPQStats)); \
	_mem_getstats(&gPQStats.memBefore, NULL, NULL);

#define PQMEM_END                                  \
	_mem_getstats(&gPQStats.memAfter, NULL, NULL); \
	CHECK(gPQStats.memBefore == gPQStats.memAfter);

typedef struct PQNode {
	int id;
} PQNode;

PQUEUE_TYPEDECLARE(PQNode)

// Order so that higher IDs are at the front of the queue
static int qcmp(const PQNode* a, const PQNode* b)
{
	if (a->id > b->id)
		return 1;
	else if (a->id < b->id)
		return -1;
	return 0;
}

static bool qremove(const PQNode* a, uint32_t cookie)
{
	return a->id == (int)cookie;
}

static void pushNode(PQueue_PQNode* q, int id)
{
	PQNode n = { id };
	CHECK(pqueue_PQNode_push(q, &n));
}

static void checkQueueInternals(PQueue_PQNode* q, int* ids)
{
	// Our queue internally uses the equivalent of an int array, so we can use
	// that for easier testing
	Array_int* a = (Array_int*)(&q->a);

	int i = 0;
	while (*ids) {
		CHECK(i < a->size);

		// Putting it in a variable, so we can see it in the debugger
		PQNode* n = (PQNode*)(&a->data[i]);
		CHECK(n->id == *ids);
		//LOG_LOG("id = %d", *ids);

		i++;
		ids++;
	}

	CHECK(i == a->size);
}

static void peekCheck(PQueue_PQNode* q, int expectedId)
{
	int size = q->a.size;
	const PQNode* n = pqueue_PQNode_peek(q);
	CHECK(n);
	CHECK(n->id == expectedId);
	CHECK(q->a.size == size);
}

static void popAndCheck(PQueue_PQNode* q, int expectedId)
{
	peekCheck(q, expectedId);

	PQNode n = { 0 };
	int size = q->a.size;
	CHECK(pqueue_PQNode_pop(q, &n));
	CHECK(n.id == expectedId);
	CHECK(q->a.size == (size - 1));
}

TEST(pqueue_create)
{
	PQMEM_START

	PQueue_PQNode q;
	PQNode n = { 0 };
	CHECK(pqueue_PQNode_create(&q, 0, qcmp));
	CHECK(pqueue_PQNode_peek(&q) == NULL);

	// The queue is empty, so pop should not do anything
	CHECK(pqueue_PQNode_pop(&q, NULL) == false);
	CHECK(pqueue_PQNode_pop(&q, &n) == false);

	pushNode(&q, 3);
	CHECK(pqueue_PQNode_peek(&q)->id == 3);

	pushNode(&q, 1);
	CHECK(pqueue_PQNode_peek(&q)->id == 3);

	pushNode(&q, 1);
	CHECK(pqueue_PQNode_peek(&q)->id == 3);

	pushNode(&q, 2);
	CHECK(pqueue_PQNode_peek(&q)->id == 3);

	pushNode(&q, 3);
	CHECK(pqueue_PQNode_peek(&q)->id == 3);

	pushNode(&q, 4);
	CHECK(pqueue_PQNode_peek(&q)->id == 4);

	checkQueueInternals(&q, (int[]){ 1, 1, 2, 3, 3, 4, 0 });

	//
	// Pop one at a time
	//
	popAndCheck(&q, 4);
	checkQueueInternals(&q, (int[]){ 1, 1, 2, 3, 3, 0 });
	popAndCheck(&q, 3);
	checkQueueInternals(&q, (int[]){ 1, 1, 2, 3, 0 });
	popAndCheck(&q, 3);
	checkQueueInternals(&q, (int[]){ 1, 1, 2, 0 });
	popAndCheck(&q, 2);
	checkQueueInternals(&q, (int[]){ 1, 1, 0 });
	popAndCheck(&q, 1);
	checkQueueInternals(&q, (int[]){ 1, 0 });
	popAndCheck(&q, 1);
	checkQueueInternals(&q, (int[]){ 0 });
	// The queue is empty, so pop should not do anything
	CHECK(pqueue_PQNode_pop(&q, NULL) == false);
	CHECK(pqueue_PQNode_pop(&q, &n) == false);

	pqueue_PQNode_destroy(&q);
	PQMEM_END
}

TEST(pqueue_remove)
{
	PQMEM_START

	PQueue_PQNode q;
	CHECK(pqueue_PQNode_create(&q, 0, qcmp));

	pushNode(&q, 3);
	pushNode(&q, 1);
	pushNode(&q, 1);
	pushNode(&q, 2);
	pushNode(&q, 3);
	pushNode(&q, 4);
	checkQueueInternals(&q, (int[]){ 1, 1, 2, 3, 3, 4, 0 });

	// If non-existent, then the queue should not be changed
	CHECK(pqueue_PQNode_remove(&q, qremove, 5) == 0);
	checkQueueInternals(&q, (int[]){ 1, 1, 2, 3, 3, 4, 0 });

	CHECK(pqueue_PQNode_remove(&q, qremove, 3) == 2);
	checkQueueInternals(&q, (int[]){ 1, 1, 2, 4, 0 });

	CHECK(pqueue_PQNode_remove(&q, qremove, 1) == 2);
	checkQueueInternals(&q, (int[]){ 2, 4, 0 });

	CHECK(pqueue_PQNode_remove(&q, qremove, 4) == 1);
	checkQueueInternals(&q, (int[]){ 2, 0 });

	CHECK(pqueue_PQNode_remove(&q, qremove, 2) == 1);
	checkQueueInternals(&q, (int[]){ 0 });

	pqueue_PQNode_destroy(&q);
	PQMEM_END
}

void priorityqueue_tests(void)
{
	pqueue_create_tests();
	pqueue_remove_tests();
}
