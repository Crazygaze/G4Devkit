#include "testframework.h"

const char* g_test_File = "";
int g_test_Line = 0;
const char* g_test_Test = "";

void test_logFailed(const char* expr, const char* line, const char* file)
{
	test_printXY(4, 0, "**** CHECK FAILED ****");
	test_printXY(0, 1, g_test_Test);
	test_printXY(0, 2, expr);
	test_printXY(0, 3, line);
	test_printXY(0, 4, file);
}

int test_hwfsmall(int bus, int funcNum, const int* regs);

void test_log(const char* str)
{
	int regs[4] = { 0 };
	regs[0] = 0;
	regs[1] = (int)str;

	// get string size manually, so we don't need strlen
	int len = 1;
	const char* tmp = str;
	while (*tmp) {
		len++;
		tmp++;
	}
	regs[2] = len;

	test_hwfsmall(
		4, // Network card bus id
		2, // SEND function
		regs);
}
