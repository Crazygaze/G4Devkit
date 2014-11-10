
#if !APCPU

#include "queue32.h"
#include "ringbuffer.h"
#include "queue.h"
#include "linkedlist.h"
#include "dynamicarray.h"
#include "priorityqueue32.h"
#include "priorityqueue.h"

void printqueue(Queue32* q, const char* title)
{
	printf("--- %s : size=%d ---\n", title, queue32_size(q));
	for(int i=0; i<queue32_size(q); i++) {
		int v = queue32_getAtIndex(q, i );
		printf("%d, ", v);
	}
	printf("\n");
}

typedef struct Foo
{
	int num;
	int v;
} Foo;

Foo* getFoo(int v)
{
	static int n;

	Foo* foo = utilshared_malloc(sizeof(Foo));
	foo->num = n++;
	foo->v = v;
	return foo;
}

Foo* getStaticFoo(int v)
{
	static int n;
	static Foo foo;
	foo.num = n++;
	foo.v = v;
	return &foo;
}

int sortint(const void* a_, const void* b_)
{
	Foo* a = *(Foo**)a_;
	Foo* b = *(Foo**)b_;
	return a->v - b->v;
}

int sortFoo(const void* a_, const void* b_)
{
	return ((Foo*)a_)->v - ((Foo*)b_)->v;
}
bool isequal(const void* val, void* cookie)
{
	return (((Foo*)val)->v == (int)cookie) ? TRUE : FALSE;
}

int sortint2(const void* a_, const void* b_)
{
	Foo* a = *(Foo**)a_;
	Foo* b = *(Foo**)b_;
	return b->v - a->v;
}

void printArray(const char* title, array_int* a)
{
	printf("%s\n    ", title);
	for (int i = 0; i < a->size; i++)
	{
		printf("%d:%d, ", a->data[i], ((Foo*)a->data[i])->v);
	}
	printf("\n");
}

void printArrayFoo(const char* title, Array_generic* a)
{
	printf("%s\n    ", title);
	for (int i = 0; i < a->size; i++)
	{
		Foo* f = (Foo*)a->data + i;
		printf("%d:%d, ", f->num, f->v);
	}
	printf("\n");
}

//int main(int argc, const char** argv)
//{
//	array_int a;
//	array_int_create(&a, 5);
//
//	array_int_pushVal(&a, 0);
//	array_int_pushVal(&a, 2);
//	array_int_pushVal(&a, 1);
//	array_int_pushVal(&a, 5);
//	array_int_pushVal(&a, 4);
//	array_int_pushVal(&a, 3);
//	printArray("Before", &a);
//	qsort(a.data, a.size, a.elementsize, &sortint);
//	printArray("After", &a);
//
//	return EXIT_SUCCESS;
//}

/*
int main(int argc, const char** argv)
{
	PriorityQueue32 q;
	priorityQueue32_create(&q, 4, &sortint);


	Foo* f = getFoo(3);
	priorityQueue32_push(&q, (int)getFoo(0));
	priorityQueue32_push(&q, (int)getFoo(2));
	priorityQueue32_push(&q, (int)getFoo(1));
	priorityQueue32_push(&q, (int)getFoo(1));
	priorityQueue32_push(&q, (int)getFoo(1));
	priorityQueue32_push(&q, (int)getFoo(5));
	priorityQueue32_push(&q, (int)getFoo(4));
	priorityQueue32_push(&q, (int)getFoo(3));
	priorityQueue32_push(&q, (int)f);
	priorityQueue32_push(&q, (int)f);
	priorityQueue32_push(&q, (int)f);

	int val;
	printArray("Before", &q.a);
	priorityQueue32_delete(&q, (int)f);
	printArray("After", &q.a);

	while (priorityQueue32_peek(&q, &val)) {
		Foo* f =(Foo*)val;
		printf("%d, ", f->v);
		priorityQueue32_popAndDrop(&q);
	}
	printArray("After", &q.a);

	return EXIT_SUCCESS;
}
*/

bool deleteFoo(const void* a, void* cookie)
{
	if (((Foo*)a)->v == (int)cookie)
		return TRUE;
	else
		return FALSE;
}

int main(int argc, const char** argv)
{
	PriorityQueue q;
	priorityQueue_create(&q, sizeof(Foo), 4, &sortFoo);

	Foo* f = getFoo(3);
	priorityQueue_push(&q, getFoo(0));
	priorityQueue_push(&q, getFoo(2));
	priorityQueue_push(&q, getFoo(1));
	priorityQueue_push(&q, getFoo(1));
	priorityQueue_push(&q, getFoo(2));
	priorityQueue_push(&q, getFoo(5));
	priorityQueue_push(&q, getFoo(4));
	priorityQueue_push(&q, getFoo(3));
	priorityQueue_push(&q, f);
	priorityQueue_push(&q, f);
	priorityQueue_push(&q, f);

	printArrayFoo("Before", &q.a);
	priorityQueue_delete(&q, &deleteFoo, (void*)2);
	printArrayFoo("After", &q.a);

	Foo* val;
	while (priorityQueue_peek(&q, &val)) {
		printf("%d, ", val->v);
		priorityQueue_popAndDrop(&q);
	}
	printArrayFoo("After", &q.a);

	priorityQueue_destroy(&q);
	return EXIT_SUCCESS;
}

void doPop(Queue* q)
{
	Foo v;
	queue_pop(q, &v);
	printf("Pop %d:%d\n", v.num, v.v);
}

void printQueue(const char* title, Queue* q)
{
	printf("\n%s : size=%d\n", title, queue_size(q));
	for (int i = 0; i < queue_size(q); i++)
	{
		Foo* f = queue_getAtIndex(q,i);
		printf("%d:%d, ", f->num, f->v);
	}
}



typedef struct Point
{
	int x;
	int y;
} Point;

typedef struct Rect
{
	int x1;
	int y1;
	int x2;
	int y2;
} Rect;

#define SETRECT(var, x1_, y1_, x2_, y2_) \
	var.x1 = x1_;                        \
	var.y1 = y1_;                        \
	var.x2 = x2_;                        \
	var.y2 = y2_;

#define MAKERECT(var, x1_, y1_, x2_, y2_) \
	Rect var;                             \
	SETRECT(var, x1_, y1_, x2_, y2_);


bool rectIntersect(Rect* dst, const Rect* a, const Rect* b)
{
	bool intersects =
		(a->x1 < b->x2) &&
		(a->x2 > b->x1) &&
		(a->y1 < b->y2) &&
		(a->y2 > b->y1);

	if (intersects) {
		dst->x1 = max(a->x1, b->x1);
		dst->y1 = max(a->y1, b->y1);
		dst->x2 = min(a->x2, b->x2);
		dst->y2 = min(a->y2, b->y2);
		return TRUE;
	} else {
		return FALSE;
	}
}

/*
int main(int argc, const char** argv)
{
	MAKERECT(a, 0, 0, 24, 79);
	MAKERECT(b, -5, -2, 1, 1);

	Rect c;
	bool res = rectIntersect(&c, &a, &b);


	return EXIT_SUCCESS;
}
*/

/*
int main(int argc, const char** argv)
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

	printQueue("Before", &q);
	queue_delete(&q, &isequal, (void*)2);
	printQueue("After removing v 2",&q);

	Foo* f;
	printf("\nFinishing\n");
	while (queue_peek(&q, &f))
	{
		printf("   %d:%d, ", f->num, f->v);
		queue_pop(&q, f);
	}

	queue_destroy(&q);

	return EXIT_SUCCESS;
}
*/

#endif


