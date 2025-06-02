#ifndef _testframework_testframework_h_
#define _testframework_testframework_h_

/*
 * These are set when a CHECK fails, to point to the file and line location of
 * the failed CHECK
 */
extern const char* g_test_File;
extern int g_test_Line;
extern const char* g_test_Test;

/* This is so we define inline VBCC style assembly functions,
 * without the IDE's syntax parser throwing errors.
 * Otherwise we would need separate blocks with #ifdef __syntax_parser__
 */
#ifdef __syntax_parser__
	#define __reg(x)
	#define TEST_INLINEASM(str) { return 0; }
#else
	#define TEST_INLINEASM(str) =str
#endif

/*!
 * Prints a string on screen, at the specified x/y position
 */
void test_printXY(int x, int y, const char* expr);

/*!
 * Emits a dbgbrk instruction.
 * No need to use this directly. Use the CHECK macros
 */
void testframework_dbgbrk(int imm)
TEST_INLINEASM("\t\
mov r0, r0\n\
dbgbrk 0");

#define TEST_STRINGIFY(a) TEST_STRINGIFY_(a)
#define TEST_STRINGIFY_(a) #a

void test_logFailed(const char* expr, const char* line, const char* file);

//
// Sends a string to the log device (NIC)
void test_log(const char* str);

// gTestLine variables and stops the application
#define CHECK(expr)                            \
	if (!(expr)) {                             \
		test_logFailed(                        \
			"EXPRESSION: " #expr,              \
			"LINE: " TEST_STRINGIFY(__LINE__), \
			"FILE: " __FILE__);                \
		g_test_File = __FILE__;                \
		g_test_Line = __LINE__;                \
		testframework_dbgbrk(0);               \
	}
	
#define TEST(name) \
	void name##_tests_impl(void);               \
	static void name##_tests()                  \
	{                                           \
		g_test_Test = #name;                    \
		test_log("Starting " #name  " tests");  \
		name##_tests_impl();                    \
		test_log("Finished " #name " tests");   \
	}                                           \
	void name##_tests_impl(void)

// returns true if `abs(a-b)<=margin`
int test_nearlyEqual(double a, double b, double margin);

#endif
