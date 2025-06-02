#include "testframework/testframework.h"
#include "utils/dynamicarray.h"
#include <stdlib.h>
#include <stdc_init.h>

typedef struct Foo {
	uint32_t id;
	// We allocate some memory for each Foo, so we can test that Foo_create and
	// Foo_destroy are called
	uint32_t* dummy;
} Foo;

#define WAS_FREED 0xDDDDDDDD
#define WAS_MOVED 0xCCCCCCCC

typedef struct FooStats {
	uint32_t idCounter;
	uint32_t created;
	uint32_t destroyed;

	uint32_t memBefore;
	uint32_t memAfter;
} FooStats;
static FooStats gStats;
#define AMEM_START                      \
	memset(&gStats, 0, sizeof(gStats)); \
	_mem_getstats(&gStats.memBefore, NULL, NULL);

#define AMEM_END                                 \
	_mem_getstats(&gStats.memAfter, NULL, NULL); \
	CHECK(gStats.destroyed == gStats.created);   \
	CHECK(gStats.memBefore == gStats.memAfter);

void validateFoo(const Foo* p)
{
	CHECK(p->id > 0);
	CHECK(p->dummy && ((*(p->dummy) == p->id)));
}

// Akin to a C++ construtor
static void Foo_create_impl(Foo* p)
{
	p->id = ++gStats.idCounter;
	p->dummy = malloc(4);
	*(p->dummy) = p->id;
	gStats.created++;
}

static Foo createFoo(void)
{
	Foo foo;
	Foo_create_impl(&foo);
	return foo;
}

// Akin to a C++ destructor
static void Foo_destroy_impl(Foo* p)
{
	CHECK(p->id != WAS_FREED);

	p->id = WAS_FREED;
	free(p->dummy);
	p->dummy = NULL;
	gStats.destroyed++;
}

// Akin to a C++ assignment operator
// Both dst and src need to be valid (initialize) Foos
static void Foo_copy_impl(Foo* dst, const Foo* src)
{
	validateFoo(dst);
	validateFoo(src);
	dst->id = src->id;
	*(dst->dummy) = *(src->dummy);
}

// Akin to a C++ move assignment operator
// Both dst and src need to be valid (initialized) Foos
static void Foo_move_impl(Foo* dst, Foo* src)
{
	validateFoo(dst);
	validateFoo(src);
	if (dst->dummy)
		free(dst->dummy);
	*dst = *src;
	src->id = WAS_MOVED;
	src->dummy = NULL;
}

ARRAY_TYPEDECLARE(Foo)
ARRAY_DECLARE_FIND(Foo, uint32_t)

ARRAY_TYPEDEFINE(Foo, Foo_create_impl(p), Foo_destroy_impl(p),
				 Foo_copy_impl(dst, src), Foo_move_impl(dst, src))

#define FOO_COMP(p, key) (p->id == key)
ARRAY_DEFINE_FIND(Foo, uint32_t, FOO_COMP)

static void checkArray(Array_Foo* a, const int* ids)
{
	int i = 0;
	while (*ids) {
		CHECK(i < a->size);

		// Putting it in a variable, so we can see it in the debugger
		Foo* f = &a->data[i];
		CHECK(f->id == *ids);

		i++;
		ids++;
	}

	CHECK(i == a->size);
}

static void createAndAdd4(Array_Foo* a)
{
	array_Foo_create(a, 4);

	// Create all local ones
	Foo foos[4];
	for (int i = 0; i < 4; i++) {
		Foo_create(&foos[i]);
	}

	// add to the array and destroy the local one
	for (int i = 0; i < 4; i++) {
		CHECK(array_Foo_pushPtr(a, &foos[i]));
		Foo_destroy(&foos[i]);
	}

	CHECK(a->size == 4);
	CHECK(a->capacity == 4);
	checkArray(a, (int[]){ 1, 2, 3, 4, 0 });
}

// An array initialized with zero capacity should not allocate memory until
// we push the first element
TEST(array_T_create_ZERO_CAPACITY)
{
	AMEM_START;

	Array_Foo a;
	CHECK(array_Foo_create(&a, 0) == true);
	CHECK(a.size == 0);
	CHECK(a.capacity == 0);
	CHECK(a.data == NULL);
	CHECK(a.elementsize == 8);

	checkArray(&a, (int[]){ 0 });

	// Since we've specified capacity as 0, no memory was allocated
	AMEM_END;
	array_Foo_destroy(&a);
	AMEM_END;
}

TEST(array_T_create_CAPACITY)
{
	AMEM_START;

	Array_Foo a;
	CHECK(array_Foo_create(&a, 2) == true);
	CHECK(a.size == 0);
	CHECK(a.capacity == 2);
	CHECK(a.data);
	CHECK(a.elementsize == 8);

	checkArray(&a, (int[]){ 0 });

	array_Foo_destroy(&a);
	AMEM_END;
}

TEST(array_T_pushPtr)
{
	AMEM_START;

	Array_Foo a;
	CHECK(array_Foo_create(&a, 0) == true);

	// Push 1 element
	Foo foo1 = { 0 };
	Foo_create(&foo1);
	Foo foo2 = { 0 };
	Foo_create(&foo2);

	CHECK(array_Foo_pushPtr(&a, &foo1));
	validateFoo(&foo1);
	CHECK(gStats.idCounter == 3); // 2 outside (stack), 1 in the array
	CHECK(a.capacity > 0);
	CHECK(a.size == 1);

	CHECK(array_Foo_pushPtr(&a, &foo2));
	validateFoo(&foo2);
	CHECK(gStats.idCounter == 4); // 2 outside (stack), 2 in the array
	CHECK(a.capacity > 0);
	CHECK(a.size == 2);

	checkArray(&a, (int[]){ 1, 2, 0 });

	Foo_destroy(&foo1);
	Foo_destroy(&foo2);

	// Destroying our local ones shouldn't affect the array
	checkArray(&a, (int[]){ 1, 2, 0 });

	CHECK(gStats.created == 4 && gStats.destroyed == 2);

	array_Foo_destroy(&a);
	AMEM_END;
}

TEST(array_T_pushVal)
{
	AMEM_START;
	Array_Foo a;
	CHECK(array_Foo_create(&a, 0) == true);

	Foo foo;
	Foo_create(&foo);
	CHECK(array_Foo_pushVal(&a, foo));
	Foo_destroy(&foo);
	CHECK(gStats.idCounter == 2);
	CHECK(a.data[0].id == 1);

	array_Foo_destroy(&a);
	AMEM_END;
}

TEST(array_T_pop)
{
	AMEM_START;

	Array_Foo a;
	createAndAdd4(&a);

	Foo foo = createFoo();
	CHECK(array_Foo_pop(&a, &foo));
	CHECK(a.size == 3);
	CHECK(foo.id == 4);
	CHECK(*foo.dummy == 4);
	Foo_destroy(&foo);

	array_Foo_destroy(&a);
	AMEM_END;
}

TEST(array_T_popAndDrop)
{
	AMEM_START;

	Array_Foo a;
	createAndAdd4(&a);

	CHECK(array_Foo_popAndDrop(&a));
	CHECK(a.size == 3);

	array_Foo_destroy(&a);
	AMEM_END;
}

TEST(array_T_clear)
{
	AMEM_START;

	Array_Foo a;
	createAndAdd4(&a);

	CHECK(array_Foo_popAndDrop(&a));
	int capacity = a.capacity;
	CHECK(a.size == 3);
	// Number of objects still alive
	CHECK((gStats.created - gStats.destroyed) == 3);
	array_Foo_clear(&a);
	CHECK(a.size == 0);
	CHECK(a.capacity == capacity);

	CHECK(gStats.created == gStats.destroyed);

	array_Foo_destroy(&a);
	AMEM_END;
}

TEST(array_T_destroy)
{
	// Nothing to tests. This is tested plenty as part of the other ones
}

TEST(array_T_find)
{
	AMEM_START;

	Array_Foo a;
	createAndAdd4(&a);

	// Change one of the Foos's id, so it's not sequential and thus we can test
	// that the find function is working
	a.data[2].id = 10;
	*a.data[2].dummy = 10;
	CHECK(array_Foo_find(&a, 10) == 2);
	CHECK(array_Foo_find(&a, 20) == -1);

	array_Foo_destroy(&a);
	AMEM_END;
}

TEST(array_T_FOREACH)
{
	AMEM_START;

	Array_Foo a;
	createAndAdd4(&a);

	// We iterate the elements and multiply the id by 10, so we can then check
	// if they were all touched.
	ARRAY_FOREACH(a, Foo*, foo) {
		foo->id *= 10;
	}

	checkArray(&a, (int[]){ 10, 20, 30, 40, 0 });

	array_Foo_destroy(&a);
	AMEM_END;
}

void dynamicarray_tests(void)
{
	size_t totalUsedBefore = 0;
	_mem_getstats(&totalUsedBefore, NULL, NULL);

	array_T_create_ZERO_CAPACITY_tests();
	array_T_create_CAPACITY_tests();
	array_T_pushPtr_tests();
	array_T_pushVal_tests();
	array_T_pop_tests();
	array_T_popAndDrop_tests();
	array_T_clear_tests();
	array_T_destroy_tests();
	array_T_find_tests();
	array_T_FOREACH_tests();

	size_t totalUsedAfter = 0;
	_mem_getstats(&totalUsedAfter, NULL, NULL);
	CHECK(totalUsedBefore == totalUsedAfter);
}
