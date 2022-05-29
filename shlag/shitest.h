// shitest - poor's man test lib. I wrote it due to my discontent with fullbown
// frameworks, such as gtest, which add lot of unnecesary abstraction that just
// goes in your way (for instance - you can't just run tests in loop, good boys
// use "parametrized test" magic). Shitest is just bunch of assert macros, wrappers
// around printf, and short summary func. Of course it lacks some fancier, yet
// sometimes useful features like timing, test order randomization or forking each
// test into separate process (so other tests won't be affected by SEGFAULT). 
// Timing or forking could be easily hacked on top of this lib, but order random-
// ization obviously won't work with shitest aproach (at least not that smoothly
// like with other libs)
//
// Used namespaces: SHITEST_, SHI_, shi_
// In *one* of C or C++ file, you have to define SHITEST_IMPL before including
// shitest. For examples see `shlag/tests/` directory

#ifndef SHITEST_H
#define SHITEST_H
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>

// Prepend public function definitions with whatever you want. You can use it
// for example to make functions static. By default it does nothing
#ifndef SHITEST_DEF
 #define SHITEST_DEF
#endif

#ifdef __cplusplus
 extern "C" {
#endif

extern unsigned testCount;
extern unsigned passCount;

// summarize tests. Returns count of failed tests
SHITEST_DEF unsigned shi_test_summary();

// Separator, you can for example print it beetween groups of tests
#define SHI_SEP "-----\n"

// Use ANSI escapes for colored output if possible. CMD doesn't support it,
// so we disable colors on windows by default
#if (defined(_WIN32) && !defined(SHITEST_FORCE_COLORS)) || defined(SHITEST_DISABLE_COLORS)
 #define SHI_RED ""
 #define SHI_GREEN ""
 #define SHI_RESET ""
#else 
 #define SHI_RED "\x1b[31m"
 #define SHI_GREEN "\x1b[32m"
 #define SHI_RESET "\x1b[0m" // return to default color
#endif

// begin test case, with specified printf-like formatted name
#define SHI_TESTf(fmt, ...) do { \
 fprintf(stderr, fmt, __VA_ARGS__); \
 fputs(": ", stderr); \
 ++shi_testcount; \
} while(0)
// like SHI_TESTf but without formatting
#define SHI_TESTm(msg) SHI_TESTf("%s", msg)

// mark test as passed. If you don't call it, test is considered as failed in stats
#define SHI_PASS() do { fputs(SHI_GREEN "OK\n" SHI_RESET, stderr); ++shi_passcount; } while(0)

// fail with specified formatted message (printf-like).
// It always return false, for convieniece when writing assert macros
SHITEST_DEF bool shi_fail_f(const char* fmt, ...);
// just alias to shi_fail_f() func for style consistency
#define SHI_FAILf(fmt, ...) shi_fail_f(fmt, __VA_ARGS__)

// Assert macros. On success they just return true, otherwise they print fail msg and return false.
// Ones with 'f' suffix allow you to supply printf-like formatted msg
// Ones with 'm' suffix allow you to supply msg without formatting (like puts)
// Ones without suffix compose msg themselves

// Assert that cond is true
#define SHI_ASSERTf(cond, fmt, ...) (cond) ? true : SHI_FAILf(fmt, __VA_ARGS__)

// Assert that suplied c strings are equal.
#define SHI_ASSERT_STREQ(expected, actual) \
 SHI_ASSERTf(strcmp((expected), (actual)) == 0, "expected: \"%s\", actual: \"%s\"", (expected), (actual))

// Assert that expected == actual and print their values using suplied type formatter if not
// For example ASSERT_EQ(2, 3, "%d"); will call FAILf("expected: %d, actual: %d", 2, 3)
#define SHI_ASSERT_EQ(expected, actual, typefmt) \
 SHI_ASSERTf((expected) == (actual), "expected: "typefmt", actual: "typefmt, (expected), (actual))


#ifdef __cplusplus
 }
#endif
#endif /* SHLAG_PCG_H */

#ifdef SHITEST_IMPL

unsigned shi_testcount = 0;
unsigned shi_passcount = 0;

SHITEST_DEF bool shi_fail_f(const char* fmt, ...)
{
    fputs(SHI_RED"FAIL\n"SHI_RESET, stderr);
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputs("\n", stderr);
    return false;
}
SHITEST_DEF unsigned shi_test_summary()
{
    unsigned failcount = shi_testcount - shi_passcount;
    fprintf(stderr, "total: %u, passed: %u, failed: %u\n", 
            shi_testcount, shi_passcount, failcount);
    return (failcount > 0);
}
#endif // SHITEST_IMPL
