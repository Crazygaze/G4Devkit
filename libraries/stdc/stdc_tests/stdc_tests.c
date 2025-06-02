#include "testframework/testframework.h"

void ctype_tests(void);
void math_tests(void);
void stdlib_tests(void);
void string_tests(void);

int main(void)
{
	ctype_tests();
	math_tests();
	stdlib_tests();
	string_tests();

	test_printXY(4, 0, "**** TESTS FINISHED ****");
	return 0;
}
