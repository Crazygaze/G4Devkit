#include "testframework/testframework.h"
#include <string.h>

TEST(strlen)
{
	CHECK(strlen("") == 0);
	CHECK(strlen("H") == 1);
	CHECK(strlen("Hello") == 5);
}

TEST(memset)
{
	char buf[4];
	buf[3] = 126;
	memset(buf, 127, sizeof(buf) - 1);
	CHECK(buf[0] == 127);
	CHECK(buf[1] == 127);
	CHECK(buf[2] == 127);
	// Last character shouldn't be touched, because we only asked to set 3
	CHECK(buf[3] == 126);
}

// No need to test memmove. APCPU's memcpy and memove are the same instruction
TEST(memcpy)
{
	char buf[4];
	memset(buf, 'A', sizeof(buf));
	memcpy(buf, "012", 3);
	CHECK(buf[0] == '0');
	CHECK(buf[1] == '1');
	CHECK(buf[2] == '2');
	// We are only writing 3 character, so last one should be untouched
	CHECK(buf[3] == 'A');
}

TEST(strcpy)
{
	char buf[4];
	char* res;

	memset(buf, 'A', sizeof(buf));
	res = strcpy(buf, "");
	CHECK(res == buf);
	CHECK(buf[0] == 0)
	CHECK(buf[1] == 'A');

	memset(buf, 'A', sizeof(buf));
	res = strcpy(buf, "0");
	CHECK(res == buf);
	CHECK(buf[0] == '0');
	CHECK(buf[1] == 0);
	CHECK(buf[2] == 'A');

	memset(buf, 'A', sizeof(buf));
	res = strcpy(buf, "01");
	CHECK(res == buf);
	CHECK(buf[0] == '0');
	CHECK(buf[1] == '1');
	CHECK(buf[2] == 0);
	CHECK(buf[3] == 'A'); // Last position should not be touched
}

TEST(strncpy)
{
	char buf[4];
	char* res;

	// Copy wihout the terminating null
	memset(buf, 'A', sizeof(buf));
	res = strncpy(buf, "01", 2);
	CHECK(res == buf);
	CHECK(buf[0] == '0');
	CHECK(buf[1] == '1');
	CHECK(buf[2] == 'A'); // Should not have the terminating null

	// Copy with the terminating null
	memset(buf, 'A', sizeof(buf));
	res = strncpy(buf, "01", 3);
	CHECK(res == buf);
	CHECK(buf[0] == '0');
	CHECK(buf[1] == '1');
	CHECK(buf[2] == 0); // Should not have the terminating null
	CHECK(buf[3] == 'A'); // Should not touch this
}

TEST(strcmp)
{
	// NOTE: Due to the -merge-strings optimization, we don't use string
	// literals. Instead, we put at least one of them in a stack buffer, to
	// force the underlying assembly to use different pointers (Better to catch
	// bugs)
	char buf[4];
	strcpy(buf, "12");
	CHECK(strcmp(buf, "12") == 0);
	CHECK(strcmp("1", "12") < 0);
	CHECK(strcmp("2", "12") > 0);
	CHECK(strcmp("13", "12") > 0);
	CHECK(strcmp("123", "12") > 0);
}

TEST(strchr)
{
	// Putting the string in the stack, because of the merge-strings
	// optimization
	char str[4];
	strcpy(str, "Hi!");

	// According to https://en.cppreference.com/w/c/string/byte/strchr,
	// searching for '\0' is a valid thing, and should return its position
	// instead of NULL
	CHECK(strchr(str, '\0') == &str[3]);
	
	CHECK(strchr(str, '!') == &str[2]);
	CHECK(strchr(str, 'i') == &str[1]);
	CHECK(strchr(str, 'H') == &str[0]);
	CHECK(strchr(str, '_') == NULL);
}

TEST(strrchr)
{
	// Putting the string in the stack, because of the merge-strings
	// optimization
	char str[4];
	strcpy(str, "Hi!");

	// According to https://en.cppreference.com/w/c/string/byte/strrchr,
	// searching for '\0' is a valid thing, and should return its position
	// instead of NULL
	CHECK(strrchr(str, '\0') == &str[3]);
	CHECK(strrchr(str, '!') == &str[2]);
	CHECK(strrchr(str, 'i') == &str[1]);
	CHECK(strrchr(str, 'H') == &str[0]);
	CHECK(strrchr(str, '_') == NULL);
}

TEST(strcat)
{
	// Putting the string in the stack, because of the merge-strings
	// optimization
	char str1[3];
	strcpy(str1, "Hi");
	char str2[2];
	strcpy(str2, " ");
	char str3[7];
	strcpy(str3, "there!");
	char str4[1] = {0};
	
	char buf[12];
	// Fill with garbage, so we can detect that the null terminator was added
	// correctly
	memset(buf, 255, sizeof(buf));
	buf[0] = 0;
	strcat(buf, str1);
	strcat(buf, str2);
	strcat(buf, str3);
	strcat(buf, str4);
	CHECK(strcmp(buf, "Hi there!") == 0);

	buf[0] = 0;
}

void string_tests(void)
{
	strlen_tests();
	memset_tests();
	memcpy_tests();
	strcpy_tests();
	strncpy_tests();
	strcmp_tests();
	strchr_tests();
	strrchr_tests();
	strcat_tests();
}
