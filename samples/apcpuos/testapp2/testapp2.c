/*!
 *	As its name suggest, testapp2 is used as a sample and test
 *	Built on top of defaultOS, it tests the queueing of processes.
 *
 *	It manages 5 basic C structures.
 *
 *	@file: testapp2.c
 */



#include <stdlib.h>

#include "app_process.h" // Process API shared by both OS and application

#include "app_txtui.h" // Display shared by both OS and application

#include "utilshared/queue.h" // Queue utility shared by both OS and application

/*!
 *	Used for derp purpose (derp count: 1)
 *
 *	@property derp
 */
int derp;

/*!
 *	The structure to enqueue/dequeue, containing
 *	sample variables
 *
 *	@struct Foo
 */
typedef struct Foo
{
	/*!
	 *	index of the structure in the queue stack
	 *
	 *	@property num
	 */
	int num;

	/*!
	 *	Defined by user
	 *
	 *	@property v
	 */
	int v;
} Foo;

/*!
 *	Initialize and get a structure
 *
 *	@method getStaticFoo
 *	@type static
 *	@param v
 *	@return struct
 */
static Foo* getStaticFoo(int v)
{
	static int n;
	static Foo foo;
	foo.num = n++;
	foo.v = v;
	return &foo;
}

/*!
 *	Check if a queue has a structure with a value
 *
 *	@method isEqual
 *	@param val
 *	@param cookie
 *	@return bool - a structure has the wanted value or not
 */
static bool isEqual(const void* val, void* cookie)
{
	return (((Foo*)val)->v == (int)cookie) ? TRUE : FALSE;
}

/*!
 *	Delete a structure at last index of (pop) the queue
 *
 *	@method doPop
 *	@param q
 */
static void doPop(Queue* q)
{
	Foo v;
	queue_pop(q, &v);
	LOG("Pop %d:%d\n", v.num, v.v); // log structure deleted
}

/*!
 *	Display queue size along with a title,
 *	defined by user
 *
 *	@method printQueue
 *	@param title
 *	@queue q
 */
void printQueue(const char* title, Queue* q)
{
	LOG("\n%s : size=%d\n", title, queue_size(q));
	for (int i = 0; i < queue_size(q); i++)
	{
		Foo* f = queue_getAtIndex(q,i);
		LOG("  %d:%d, ", f->num, f->v);
	}
}

/*!
 *	Test managment of queue
 *
 *	@method testQueue
 */
void testQueue(void)
{
	// Create a queue
	Queue q;
	queue_create(&q, sizeof(Foo), 4);

	// Add structure to the queue
	queue_push(&q, getStaticFoo(0));
	queue_push(&q, getStaticFoo(1));
	queue_push(&q, getStaticFoo(2));
	queue_push(&q, getStaticFoo(3));

	// Delete last index of the Queue stack
	doPop(&q);
	doPop(&q);

	// Then push again some structures
	queue_push(&q, getStaticFoo(2));
	queue_push(&q, getStaticFoo(5));
//	queue_push(&q, getStaticFoo(2));
//	queue_push(&q, getStaticFoo(6));
//	queue_push(&q, getStaticFoo(2));

	// Display the queue size before a queue deletion
	printQueue("Before", &q);

	// Delete the structure at queue index 2
	queue_delete(&q, &isEqual, (void*)2);

	// Display the queue size after a queue deletion
	printQueue("After removing v 2",&q);

	Foo* f;
	LOG("\nFinishing\n");

	// While the queue index have a Foo structure
	while (f=queue_peek(&q))
	{
		// Log the structure properties
		LOG("   %d:%d, ", f->num, f->v);

		// And remove the structure from the queue
		queue_pop(&q, NULL);
	}

	// Destroy the queue
	queue_destroy(&q);
}

/*!
 *	Launch the application
 *
 *	@method testapp2
 *	@param p1 - app index
 *	@return int - exit_code
 */
int testapp2(int p1)
{
	// Display list of apps running
	txtui_printfAtXY(&rootCanvas, 0,0, "Helllo world from app %d!! :)", p1);

	// Launch the queue managment test
	testQueue();
	LOG("**** testapp2 launched....");

	// Throttle the application with index 2
	if (p1==2) {
		app_sleep(1000); // sleep for 1000 ms
		app_focus(); // give back focus to the application
	}

	// Look! derp is evolving into counter!
	int counter=0;

	// Infinite loop to lock the application
	ThreadMsg msg;
	while(app_getMessage(&msg)) {
	}

	return EXIT_SUCCESS;
}
