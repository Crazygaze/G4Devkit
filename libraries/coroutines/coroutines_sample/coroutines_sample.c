#include "coroutines/coroutine.h"
#include "testframework/testframework.h"

#include <stdio.h>

Coroutine co1, co2;
#define CO_STACK_SIZE 2000
char stack1[CO_STACK_SIZE], stack2[CO_STACK_SIZE];

#define PRINTFATXY_BUFSIZE 64
int printfAtXY(int x, int y, const char* fmt, ...)
{
	char buf[PRINTFATXY_BUFSIZE];
	va_list ap;
	va_start(ap, fmt);
	int res = vsnprintf(buf, PRINTFATXY_BUFSIZE, fmt, ap);
	test_printXY(x, y, buf);
	return res;
}

//
// spins for N seconds, display a counter for every 100ms
//
void spinSeconds(int sec)
{
	double now = hwclk_getSecsSinceBoot();
	double end = now + sec;
	double nextDrawTime = now;

	while (now < end) {
		if (now >= nextDrawTime) {
			printfAtXY(40, 0, " %3.3f", now);
			nextDrawTime += 0.001f;
		}
		now = hwclk_getSecsSinceBoot();
	}
}

// We use "yielded" as the row to print at
// "cookie" is the other coroutine we want to switch to
int run(int yielded)
{
	CO_LOGF("run - started with %d", yielded);
	Coroutine* this = co_this();
	Coroutine* other = this->cookie;
	int x;
	for (x = 0; x < 5; x++) {
		printfAtXY(x, yielded, " Coroutine %d", yielded);
		spinSeconds(1);
		co_yield(other, yielded == 1 ? 2 : 1);
	}

	printfAtXY(x, yielded, " Finished ");
	CO_LOGF("run - finished with %d", yielded);
	return yielded;
}

int main(void)
{
	printfAtXY(0, 0, "Coroutines example");

	co_create(&co1, stack1, sizeof(stack1), &run, &co2); // Cookie is co2
	co_create(&co2, stack2, sizeof(stack2), &run, &co1); // Cookie is co1

	// switch execution to first coroutine in the chain
	int yielded = 1;
	// Wait until all coroutines finish
	while (co_hasAlive()) {
		yielded = co_yield(NULL, yielded);
	}

	printfAtXY(0, 4, "Result=%d", yielded);

	// As a bonus, lets calculate how much stack the coroutines used
	// This is useful so to decide if we can provide smaller stack buffers
	printfAtXY(0, 5, "Coroutine 1 used stack = %d/%d",
					 co_calcUsedStack(&co1), sizeof(stack1));
	printfAtXY(0, 6, "Coroutine 2 used stack = %d/%d",
					 co_calcUsedStack(&co2), sizeof(stack2));

	return 0;
}
