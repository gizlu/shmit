// Compile it with something like:
// cc -I. tests/btt_test.c -o bin/btt_test
// TODO: add makefile or something
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#define SHLAG_BTT_IMPL
#define SHLAG_BTT_DEF static // optional, but makes smaller exec
#include "shlag_btt.h"
#define SHITEST_IMPL
#define SHITEST_DEF static
#include "shitest.h"

typedef struct TestPair
{
    unsigned char* plain; // plain, possibly binary data
    char* plainStringized; // stringized form of possibly binary @plain, meant for debug info
    char* enc; // encoded data
    unsigned plainSize;
    unsigned encodedSize;
} TestPair;

// construct test pair.
// Last byte of @plain (preasumbly null terminator) is ignored
#define TEST_PAIR(plain, encoded) \
{(unsigned char*)(plain), #plain, encoded, sizeof((plain))-1, sizeof(encoded)}

void b64_test_outplace_enc(TestPair pair)
{
    // We don't use one big buffer for all tests despite we can, because it
    // it could potentialy hide OOB bugs from sanitizer
    SHI_TESTf("b64enc(%s): ", pair.plainStringized);
    unsigned char* out = malloc(pair.encodedSize);
    shlag_b64enc(pair.plain, pair.plainSize, out);
    if(SHI_ASSERT_STREQ(pair.enc, (char*)out)) SHI_PASS();
    free(out); 
}

void b64_test_inplace_enc(TestPair pair)
{
    SHI_TESTf("b64enc_inplace(%s): ", pair.plainStringized);
    unsigned char* out = malloc(pair.encodedSize);
    memcpy(out, pair.plain, pair.plainSize);
    shlag_b64enc(out, pair.plainSize, out);
    if(SHI_ASSERT_STREQ(pair.enc, (char*)out)) SHI_PASS();
    free(out); 
}

void b64enc_testsuite()
{
    TestPair pairs[] = { 
        // RFC 4648 examples
        TEST_PAIR("", ""),
        TEST_PAIR("f", "Zg=="),
        TEST_PAIR("fo", "Zm8="),
        TEST_PAIR("foo", "Zm9v"),
        TEST_PAIR("foob", "Zm9vYg=="),
        TEST_PAIR("fooba", "Zm9vYmE="),
        TEST_PAIR("foobar", "Zm9vYmFy"),
        // Wikipedia examples
        TEST_PAIR("sure.", "c3VyZS4="),
        TEST_PAIR("sure", "c3VyZQ=="),
        TEST_PAIR("sur", "c3Vy"),
        TEST_PAIR("su", "c3U="),
        TEST_PAIR("leasure.", "bGVhc3VyZS4="),
        TEST_PAIR("easure.", "ZWFzdXJlLg=="),
        TEST_PAIR("asure.", "YXN1cmUu"),
        TEST_PAIR("sure.", "c3VyZS4="),
        // RFC 3548 examples
        TEST_PAIR("\x14\xfb\x9c\x03\xd9\x7e", "FPucA9l+"),
        TEST_PAIR("\x14\xfb\x9c\x03\xd9", "FPucA9k="),
        TEST_PAIR("\x14\xfb\x9c\x03", "FPucAw=="),
        // My own test cases with nulls
        TEST_PAIR("\0", "AA=="),
        TEST_PAIR("\0a", "AGE="),
        TEST_PAIR("a\0b\0", "YQBiAA=="),
        TEST_PAIR("\0\0\0\0", "AAAAAA==")
    };
    const unsigned pairs_count = sizeof(pairs)/sizeof(pairs[0]);

    fprintf(stderr, "test if b64enc encodes data correctly, to separate buffer\n");
    for(unsigned i = 0; i<pairs_count; ++i) {
        b64_test_outplace_enc(pairs[i]);
    }
    fputs(SHI_SEP, stderr);

    fprintf(stderr, "test if b64enc encodes data correctly, to same buffer (inplace)\n");
    for(unsigned i = 0; i<pairs_count; ++i) {
        b64_test_inplace_enc(pairs[i]);
    }
    fputs(SHI_SEP, stderr);
}

void b64encsize_test_smallsizes()
{
    long long sizelookup[] = {1,5,5,5,9,9,9,13,13,13,17,17,17,21};
    for(unsigned i = 0; i<sizeof(sizelookup)/sizeof(sizelookup[0]); ++i) {
        SHI_TESTf("b64encsize(%u)", i);
        long long result = SHLAG_B64_ENCSIZE(i);
        if(SHI_ASSERT_EQ(sizelookup[i], result, "%ll")) SHI_PASS();
    }
}
void b64encsize_test_maxsize()
{
    SHI_TESTm("b64_encsize(ENCSIZE_LIMIT): ");
    long long result = SHLAG_B64_ENCSIZE(SHLAG_BTT_ENCSIZE_LIMIT);
    if(SHI_ASSERT_EQ(1466015503705LLU, result,  "%ll")) SHI_PASS();
}
void b64encsize_testsuite()
{
    fprintf(stderr, "test if b64encsize() returns correct sizes\n");
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
