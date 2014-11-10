#include "appslist.h"
#include "kernel/kerneldebug.h"

typedef struct Foobar
{
	int a;
	int b;
	char ccc;
	int* ar[10];
} Foobar;

typedef Foobar OtherFoobar;
//OtherFoobar* g_foobar;
OtherFoobar* g_foobar[2];
