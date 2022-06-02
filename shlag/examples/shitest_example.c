#define SHITEST_IMPL
#define SHITEST_DEF static
#include "shitest.h"

// return square of suplied number. Intentionaly broken
int square(int x) 
{
    return 0;
}
// return copy of suplied string. Intentionaly broken
char* my_strdupa(char* str)
{
    return "";
}
// return copy of suplied memory. Intentionaly broken
unsigned char* my_memdupa(unsigned char* mem, int size)
{
    return (unsigned char*)"\0\0aaa";
}

// "stringize" compile time literal. We use it for "escaping" binary stuff
#define STR(x) #x

void binary_memory_advanced_test();

int main()
{
    // simple passing test
    shi_test("square(0)");
    shi_assert_eq(square(0), 0, "%d", int);
    shi_test_end();

    // simple failing test, with formatted name
    shi_test("square(%d)", 1);
    shi_assert_eq(square(1), 1, "%d", int);
    shi_test_end();

    // as above, but with strings
    shi_test("my_strcpy(\"%s\")", "dupa");
    shi_assert_streq(my_strdupa("dupa"), "dupa");
    shi_test_end();

    // Test can use multiple asserts
    shi_test("multiple_assert_test");
    shi_assert_eq(square(1), 1, "%d", int); // fail
    shi_assert_eq(square(2), 4, "%d", int); // fail
    shi_assert_eq(square(0), 0, "%d", int); // OK
    shi_test_end();

    // simple, failing test with memory comparsion
    shi_test("arbitrary_memory_test");
    shi_assert_memeq("\x14\xfb\0\x9c\x03", "\0\0aaa", 5);
    shi_test_end();

    // Another failing test with memory comparsion. Note: Unfortunately in case of input
    // buffers not being literals, their content isn't printed (only suplied names)
    shi_test("arbitrary_memory_test2");
    unsigned char* mem = (unsigned char*)"\0\x14";
    shi_assert_memeq(mem, my_memdupa(mem, 2), 2);
    shi_test_end();

    // If you store your expected buffer in variable, but still want to display
    // its content on failure, you can use hack like this (see func implementation)
    binary_memory_advanced_test();

    // assert routines with '_f' suffix let you specify your own msg
    shi_test("custom_msg_test");
    shi_assert_f(square(0) == 0, "0 != 0"); // OK
    shi_assert_f(square(2) == 4, "result != 4"); //FAIL
    shi_assert_f(square(2) == 4, "result != %d", 4); //FAIL
    shi_assert_streq_f("test", my_strdupa("test"), "yet another fail msg"); // FAIL
    shi_test_end();

    // stdlib-like assert
    shi_test("stdlib_like_test");
    shi_assert(square(2) == 4);
    shi_test_end();

    fputs(SHI_SEP, stderr);

    return (shi_test_summary() > 0);
}

// Example showing how you can store expected memory as variable,
// and still be able to print it content directly on failure

typedef struct ExpectedMemory
{
    unsigned char* bin;
    int binSize;
    char* str; // stringized form of binary
} ExpectedMemory;
// constructor for ExpectedMemory
#define EXPECTED_MEMORY(expected) {(unsigned char*)expected, sizeof(expected), #expected}

void binary_memory_advanced_test()
{
    shi_test("arbitrary_memory_test3");
    ExpectedMemory expected = EXPECTED_MEMORY("\0\x14");
    unsigned char* actual = my_memdupa(expected.bin, expected.binSize);
    shi_assert_memeq_f(expected.bin, actual, expected.binSize, "actual != %s", expected.str);
    shi_test_end();
}
