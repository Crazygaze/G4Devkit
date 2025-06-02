#include "testframework/testframework.h"
#include "utils/linkedlist.h"
#include "utils/queue.h"
#include <stdlib.h>
#include <stdc_init.h>

typedef struct LLNode {
	struct LLNode* next;
	struct LLNode* previous;
	int n;
} LLNode;
LINKEDLIST_VALIDATE_TYPE(LLNode)

// Index 0 is not used. Since the tests consider id 0 as "no node".
// We still have it in the array, so the id matches the index.
// E.g nodes[1]->id == 1
#define DEFINE_NODES        \
static LLNode nodes[5] = {  \
	{NULL, NULL, 0},        \
	{NULL, NULL, 1},        \
	{NULL, NULL, 2},        \
	{NULL, NULL, 3},        \
	{NULL, NULL, 4},        \
};

static void checkLinkedList(LLNode* startNode, const int* ids)
{
	int numExpectedIds = 0;
	while (ids[numExpectedIds]) {
		numExpectedIds++;
	}

	int count = 0;
	LINKEDLIST_FOREACH(startNode, LLNode*, it) {
		CHECK(*ids != 0);
		CHECK(it->n == *ids);
		CHECK(it == it->next->previous);
		++ids;
		++count;
	}

	// Check if we went through all expected ids
	CHECK(count == numExpectedIds);
}

TEST(linkedlist_addAfter)
{
	DEFINE_NODES

	//
	// Add elements
	//

	// Specifying the same node for both parameter causes the linked list to
	// be effectively just that 1 element
	linkedlist_addAfter(&nodes[1], &nodes[1]);
	CHECK(linkedlist_size(&nodes[1]) == 1);

	 // Skipping [2], so we can test inserting out of order
	linkedlist_addAfter(&nodes[1], &nodes[3]);

	linkedlist_addAfter(&nodes[3], &nodes[4]);
	linkedlist_addAfter(&nodes[1], &nodes[2]); // Insert the one we skip
	checkLinkedList(&nodes[1], (int[]){ 1, 2, 3, 4, 0 });
	CHECK(linkedlist_size(&nodes[1]) == 4);
}

TEST(linkedlist_remove)
{
	DEFINE_NODES

	// Removing an item that in a linked list should not do anything
	linkedlist_remove(&nodes[1]);
	CHECK(nodes[1].next == NULL && nodes[1].previous == NULL);

	linkedlist_addAfter(&nodes[1], &nodes[1]);
	linkedlist_addAfter(&nodes[1], &nodes[2]);
	linkedlist_addAfter(&nodes[2], &nodes[3]);
	linkedlist_addAfter(&nodes[3], &nodes[4]);
	checkLinkedList(&nodes[1], (int[]){ 1, 2, 3, 4, 0 });

	// Remove one item
	linkedlist_remove(&nodes[2]);
	CHECK(nodes[2].next == NULL && nodes[2].previous == NULL);
	checkLinkedList(&nodes[1], (int[]){ 1, 3, 4, 0 });
	// Insert it at the end
	linkedlist_addAfter(&nodes[4], &nodes[2]);
	checkLinkedList(&nodes[1], (int[]){ 1, 3, 4, 2, 0 });
}

TEST(linkedlist_FOREACH)
{
	DEFINE_NODES

	//  When a node is not in a linked list, it should not do anything
	checkLinkedList(&nodes[1], (int[]){ 0 });
	// When there is a single node...
	linkedlist_addAfter(&nodes[1], &nodes[1]);
	checkLinkedList(&nodes[1], (int[]){ 1, 0 });

	linkedlist_addAfter(&nodes[1], &nodes[2]);
	linkedlist_addAfter(&nodes[2], &nodes[3]);
	linkedlist_addAfter(&nodes[3], &nodes[4]);
	checkLinkedList(&nodes[1], (int[]){ 1, 2, 3, 4, 0 });
}

TEST(linkedlist_size)
{
	DEFINE_NODES

	//  When a node is not in a linked list, it should still work
	CHECK(linkedlist_size(&nodes[1]) == 0);

	// 1 element
	linkedlist_addAfter(&nodes[1], &nodes[1]);
	CHECK(linkedlist_size(&nodes[1]) == 1);

	linkedlist_addAfter(&nodes[1], &nodes[2]);
	CHECK(linkedlist_size(&nodes[1]) == 2);
}

void linkedlist_tests(void)
{
	linkedlist_addAfter_tests();
	linkedlist_remove_tests();
	linkedlist_FOREACH_tests();
	linkedlist_size_tests();
}

