// shitest - poor's man test lib. I wrote it due to my discontent with fullbown
// frameworks, such as gtest, which add lot of unnecesary abstraction that just
// goes in your way (for instance - you can't just run tests in loop, good boys
// use "parametrized test" magic). Shitest is just bunch of assert routines, wrappers
// around printf, and short summary. Of course it lacks some fancier, yet
// sometimes useful features like timing, test order randomization or forking each
// test into separate process (so other tests won't be affected by crash or
// something). Too bad!
//
// Used namespaces: SHITEST_, SHI_, shi_
// In *one* of C or C++ file, you have to define SHITEST_IMPL before including
// shitest. For examples see `shlag/examples/shitest_example.c` and `shlag/tests/`
// directory
//
// KNOWN BUGS:
// - Probably not thread safe. To be honest I haven't use threads yet, but I
// hope that this is trivialy mitigable
// - If you print some debug info during test case, sometimes ouput may
// look kinda strange (it still should be readable tho). I don't have any simple 
// mitigation idea

#ifndef SHITEST_H
#define SHITEST_H
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

// Prepend public function definitions with whatever you want. You can use it
// for example to make functions static. By default it does nothing
#ifndef SHITEST_DEF
 #define SHITEST_DEF
#endif

#ifdef __cplusplus
 extern "C" {
#endif

// Begin test case with formatted name
SHITEST_DEF void shi_test(const char* fmt, ...);
// End test case, and print "OK" if it passed (otherwise nothing, fails are reported earlier)
// It have to be called before starting another test
SHITEST_DEF void shi_test_end();

// Assert routines. On fail they print some info, and mark test as failed
// Ones with '_f' suffix allow you to supply printf-like formatted msg
// Ones without suffix compose msg themselves

// assert that @cond is true, fail with suplied msg if not
SHITEST_DEF void shi_assert_f(bool cond, const char* fmt, ...);
// Assert that expected == actual and print their values using suplied type formatter if not
// For example shi_assert_eq(2, 3, "%d", int) will call shi_assert_f(2 == 3, "expected: %d, actual: %d", 2, 3)
// @type is used for temp vars storing @expected and @actual
#define shi_assert_eq(expected, actual, typefmt, type) do { \
 type shi_expected_ = (expected); /* hack avoiding param re-evaluation */ \
 type shi_actual_ = (actual); \
 shi_assert_f(shi_expected_ == shi_actual_, \
    "expected: "typefmt", actual: "typefmt, shi_expected_, shi_actual_); \
} while(0)
// Assert that suplied c strings are equal, fail with suplied msg
SHITEST_DEF void shi_assert_streq_f(const char* expected, const char* actual, const char* fmt, ...);
// As above but with automatic failure msg
SHITEST_DEF void shi_assert_streq(const char* expected, const char* actual);
// Assert that first @n bytes of memory pointed by @expected and @actual are equal.
// fail with suplied formatted message
SHITEST_DEF void shi_assert_memeq_f(const void *expected, const void *actual, size_t n, const char* fmt, ...);
// Like shi_assert_memeq_f(), but with automatic failure msg. NOTE: Unfortunately in case
// of input buffers not being literals, their content isn't printed (only suplied names)
#define shi_assert_memeq(expected, actual, n) do {\
    size_t shi_n_ = n; \
    shi_assert_memeq_f(expected, actual, shi_n_, \
        "first %d bytes of %s and %s should be same, but aren't", shi_n_, #expected, #actual); \
} while(0)
// summarize tests. Returns count of failed tests
SHITEST_DEF unsigned shi_test_summary();

// Separator, you can for example print it beetween groups of tests
#define SHI_SEP "-----\n"

// -- Lower-level stuff --

// Like shi_assert_f() but with automatic failure msg. Condition is printed as is - like
// with shi_assert_memeq, values aren't expanded. It can be useful for fast prototyping,
// but you should prefer using other routines as they often give more useful info
#define shi_assert(cond) shi_assert_f((cond), "shi_assert(%s) failed", #cond)

// shi_assert_f equivalent except that it is called with a va_list instead of a variable number of
// arguments (like printf vs vprintf). You can use it for defining custom assert routines.
SHITEST_DEF void shi_assert_vf(bool cond, const char* fmt, va_list ap);

// private globals
extern unsigned testCount;
extern unsigned passCount;
extern unsigned curTestStatus;

#ifdef __cplusplus
 }
#endif
#endif /* SHLAG_PCG_H */

#ifdef SHITEST_IMPL
// Use ANSI escapes for colored output if possible. Older versions of CMD doesn't support it,
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

unsigned shi_testcount = 0;
unsigned shi_passcount = 0;
#define SHI_OK 0
#define SHI_FAIL 1
unsigned shi_curTestStatus = SHI_OK; // status of currently running test

SHITEST_DEF unsigned shi_test_summary()
{
    unsigned failcount = shi_testcount - shi_passcount;
    fprintf(stderr, "total: %u, passed: %u, failed: %u\n", 
            shi_testcount, shi_passcount, failcount);
    return failcount;
}

SHITEST_DEF void shi_test(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fputs(":", stderr);
    ++shi_testcount;
    shi_curTestStatus = SHI_OK;
}
SHITEST_DEF void shi_test_end()
{
    if(shi_curTestStatus == SHI_OK) {
        fputs(SHI_GREEN " OK\n" SHI_RESET, stderr); ++shi_passcount;
    }
}
SHITEST_DEF void shi_assert_vf(bool cond, const char* fmt, va_list ap)
{
    if(cond) return;
    if(shi_curTestStatus != SHI_FAIL) { // avoid duplication
        fputs(SHI_RED" FAIL\n"SHI_RESET, stderr);
    }
    fputs(" ", stderr);
    shi_curTestStatus = SHI_FAIL;
    vfprintf(stderr, fmt, ap);
    fputs("\n", stderr);
}
SHITEST_DEF void shi_assert_f(bool cond, const char* fmt, ...)
{
    va_list ap; va_start(ap, fmt); 
    shi_assert_vf(cond, fmt, ap);
    va_end(ap);
}
SHITEST_DEF void shi_assert_streq_f(const char* s1, const char* s2, const char* fmt, ...)
{
    va_list ap; va_start(ap, fmt); 
    shi_assert_vf(strcmp(s1, s2) == 0, fmt, ap);
    va_end(ap);
}
SHITEST_DEF void shi_assert_memeq_f(const void* mem1, const void* mem2, size_t n, const char* fmt, ...)
{
    va_list ap; va_start(ap, fmt); 
    shi_assert_vf(memcmp(mem1, mem2, n) == 0, fmt, ap);
    va_end(ap);
}
SHITEST_DEF void shi_assert_streq(const char* expected, const char* actual)
{
    return shi_assert_streq_f(expected, actual, "expected: \"%s\", actual: \"%s\"", expected, actual);
}
#endif // SHITEST_IMPL
