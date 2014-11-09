#include <stdlib.h>
#include "app_process.h"
#include "app_txtui.h"
#include "utilshared/queue.h"
int derp;

typedef struct Foo
{
	int num;
	int v;
} Foo;

static Foo* getStaticFoo(int v)
{
	static int n;
	static Foo foo;
	foo.num = n++;
	foo.v = v;
	return &foo;
}

static bool isequal(const void* val, void* cookie)
{
	return (((Foo*)val)->v == (int)cookie) ? TRUE : FALSE;
}

static void doPop(Queue* q)
{
	Foo v;
	queue_pop(q, &v);
	LOG("Pop %d:%d\n", v.num, v.v);
}

void printQueue(const char* title, Queue* q)
{
	LOG("\n%s : size=%d\n", title, queue_size(q));
	for (int i = 0; i < queue_size(q); i++)
	{
		Foo* f = queue_getAtIndex(q,i);
		LOG("  %d:%d, ", f->num, f->v);
	}
}

void testQueue(void)
{
	Queue q;
	queue_create(&q, sizeof(Foo), 4);

	queue_push(&q, getStaticFoo(0));
	queue_push(&q, getStaticFoo(1));
	queue_push(&q, getStaticFoo(2));
	queue_push(&q, getStaticFoo(3));
	
	doPop(&q);
	doPop(&q);

	queue_push(&q, getStaticFoo(2));
	queue_push(&q, getStaticFoo(5));
//	queue_push(&q, getStaticFoo(2));
//	queue_push(&q, getStaticFoo(6));
//	queue_push(&q, getStaticFoo(2));

	printQueue("Before", &q);
	queue_delete(&q, &isequal, (void*)2);
	printQueue("After removing v 2",&q);

	Foo* f;
	LOG("\nFinishing\n");
	while (queue_peek(&q, &f))
	{
		LOG("   %d:%d, ", f->num, f->v);
		queue_pop(&q, f);
	}

	queue_destroy(&q);
}


// TODO : REMOVE THIS
int testapp2(int p1)
{
	txtui_printfAtXY(&rootCanvas, 0,0, "Helllo world from app %d!! :)", p1);
	testQueue();
	LOG("**** testapp2 launched....");
		
	if (p1==2) {
		app_sleep(1000);
		app_focus();
	}
	
	int counter=0;
	ThreadMsg msg;
	while(app_getMessage(&msg)) {
	}
	
	return EXIT_SUCCESS;
}
