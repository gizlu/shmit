// Compile it with something like:
// cc -I. tests/btt_test.c -o bin/btt_test
// TODO: add makefile or something
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#define SHLAG_BTT_IMPL
#define SHLAG_BTT_DEF static // optional, but makes smaller exec
#include "shlag_btt.h"
#define SHITEST_IMPL
#define SHITEST_DEF static
#include "shitest.h"

// Test inplace and outplace encoding. Last byte of @in is not encoded
// This is the cleanest (xD) way I found, to avoid long repetitive test cases without
// sharing big buffer across unrelated tests (it could potentialy hide OOBs from sanitizer)
#define B64_ENC_TEST(in, expected) \
do { \
    char buf[sizeof(expected)]; \
    SHI_TESTm("b64enc("#in"): "); \
    shlag_b64enc((unsigned char*)in, sizeof(in)-1, (unsigned char*)buf); \
    if(SHI_ASSERT_STREQ(expected, buf)) SHI_PASS(); \
    \
    SHI_TESTm("b64enc_inplace("#in"): "); \
    memcpy(buf, in, sizeof(in)-1); /* reuse buff for inplace enc */ \
    shlag_b64enc((unsigned char*)buf, sizeof(in)-1, (unsigned char*)buf); \
    if(SHI_ASSERT_STREQ(expected, buf)) SHI_PASS(); \
} while(0)

void b64enc_testsuite()
{
    fprintf(stderr, "test if b64enc encodes data correctly (normally and inplace)\n");
    // RFC 4648 examples
    B64_ENC_TEST("", "");
    B64_ENC_TEST("f", "Zg==");
    B64_ENC_TEST("fo", "Zm8=");
    B64_ENC_TEST("foo", "Zm9v");
    B64_ENC_TEST("foob", "Zm9vYg==");
    B64_ENC_TEST("fooba", "Zm9vYmE=");
    B64_ENC_TEST("foobar", "Zm9vYmFy");
    // Wikipedia examples
    B64_ENC_TEST("sure.", "c3VyZS4=");
    B64_ENC_TEST("sure", "c3VyZQ==");
    B64_ENC_TEST("sur", "c3Vy");
    B64_ENC_TEST("su", "c3U=");
    B64_ENC_TEST("leasure.", "bGVhc3VyZS4=");
    B64_ENC_TEST("easure.", "ZWFzdXJlLg==");
    B64_ENC_TEST("asure.", "YXN1cmUu");
    B64_ENC_TEST("sure.", "c3VyZS4=");
    // RFC 3548 examples
    B64_ENC_TEST("\x14\xfb\x9c\x03\xd9\x7e", "FPucA9l+");
    B64_ENC_TEST("\x14\xfb\x9c\x03\xd9", "FPucA9k=");
    B64_ENC_TEST("\x14\xfb\x9c\x03", "FPucAw==");
    // My own test cases with nulls
    B64_ENC_TEST("\0", "AA==");
    B64_ENC_TEST("\0a", "AGE=");
    B64_ENC_TEST("a\0b\0", "YQBiAA==");
    B64_ENC_TEST("\0\0\0\0", "AAAAAA==");
    fputs(SHI_SEP, stderr);
}

void b64encsize_test_smallsizes()
{
    unsigned long long sizelookup[] = {1,5,5,5,9,9,9,13,13,13,17,17,17,21};
    for(unsigned i = 0; i<sizeof(sizelookup)/sizeof(sizelookup[0]); ++i) {
        SHI_TESTf("b64encsize(%u)", i);
        unsigned long long result = SHLAG_B64_ENCSIZE(i);
        if(SHI_ASSERT_EQ(sizelookup[i], result, "%llu")) SHI_PASS();
    }
}
void b64encsize_test_maxsize()
{
    SHI_TESTm("b64_encsize(ENCSIZE_LIMIT): ");
    unsigned long long result = SHLAG_B64_ENCSIZE(SHLAG_BTT_ENCSIZE_LIMIT);
    if(SHI_ASSERT_EQ(1466015503705LLU, result,  "%llu")) SHI_PASS();
}
void b64encsize_testsuite()
{
    fprintf(stderr, "test if b64encsize() reports correct sizes\n");
    b64encsize_test_smallsizes();
    b64encsize_test_maxsize();
    fputs(SHI_SEP, stderr);
}
int main()
{
    b64enc_testsuite();
    b64encsize_testsuite();
    return shi_test_summary();
}
