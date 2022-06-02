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
    uint8_t* plain; // plain, possibly binary data
    char* plainStringized; // stringized form of possibly binary @plain, meant for debug info
    char* encoded;
    unsigned plainSize;
    unsigned encodedLen;
} TestPair;

// construct test pair.
// Last byte of @plain (preasumbly null terminator) is ignored
#define TEST_PAIR(plain, encoded) \
{(uint8_t*)(plain), #plain, encoded, sizeof((plain))-1, strlen(encoded)}

void b64_test_outplace_enc(TestPair p)
{
    // We don't use one big buffer for all tests despite we can, because it
    // it could potentialy hide OOB bugs from sanitizer
    shi_test("b64enc(%s): ", p.plainStringized);
    char* out = malloc(p.encodedLen + 1);
    shlag_b64enc(p.plain, p.plainSize, out);
    shi_assert_streq(p.encoded, out);
    shi_test_end();
    free(out); 
}

void b64_test_inplace_enc(TestPair p)
{
    shi_test("b64enc_inplace(%s): ", p.plainStringized);
    char* buf = malloc(p.encodedLen + 1);
    memcpy(buf, p.plain, p.plainSize);
    shlag_b64enc((uint8_t*)buf, p.plainSize, buf);
    shi_assert_streq(p.encoded, buf);
    shi_test_end();
    free(buf); 
}

void b64_test_outplace_dec(TestPair p)
{
    shi_test("b64dec(\"%s\")", p.encoded);
    uint8_t* out = malloc(p.plainSize);
    shlag_b64dec(p.encoded, p.encodedLen, out);
    shi_assert_memeq_f(p.plain, out, p.plainSize, "result != %s", p.plainStringized);
    shi_test_end();
    free(out);
}

void b64_test_inplace_dec(TestPair p)
{
    shi_test("b64dec_inplace(\"%s\")", p.encoded);
    uint8_t* buf = malloc(p.plainSize);
    memcpy(buf, p.encoded, p.encodedLen+1);
    shlag_b64dec(p.encoded, p.encodedLen, buf);
    shi_assert_memeq_f(p.plain, buf, p.plainSize, "result != %s", p.plainStringized);
    shi_test_end();
    free(buf);
}

TestPair valid_b64_pairs[] = {
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
void b64enc_testsuite()
{
    const unsigned pairs_count = sizeof(valid_b64_pairs)/sizeof(valid_b64_pairs[0]);

    fprintf(stderr, "test if b64enc encodes data correctly, to separate buffer\n");
    for(unsigned i = 0; i<pairs_count; ++i) {
        b64_test_outplace_enc(valid_b64_pairs[i]);
    }
    fputs(SHI_SEP, stderr);

    fprintf(stderr, "test if b64enc encodes data correctly, to same buffer (inplace)\n");
    for(unsigned i = 0; i<pairs_count; ++i) {
        b64_test_inplace_enc(valid_b64_pairs[i]);
    }
    fputs(SHI_SEP, stderr);
}

void b64dec_testsuite()
{
    const unsigned pairs_count = sizeof(valid_b64_pairs)/sizeof(valid_b64_pairs[0]);

    fprintf(stderr, "test if b64dec decodes valid, padded data correctly, to separate buffer\n");
    for(unsigned i = 0; i<pairs_count; ++i) {
        b64_test_outplace_dec(valid_b64_pairs[i]);
    }
    fputs(SHI_SEP, stderr);

    fprintf(stderr, "test if b64dec decodes valid, padded data correctly, to same buffer (inplace)\n");
    for(unsigned i = 0; i<pairs_count; ++i) {
        b64_test_inplace_dec(valid_b64_pairs[i]);
    }
    fputs(SHI_SEP, stderr);
}

void b64encsize_test_smallsizes()
{
    long long sizelookup[] = {1,5,5,5,9,9,9,13,13,13,17,17,17,21};
    for(unsigned i = 0; i<sizeof(sizelookup)/sizeof(sizelookup[0]); ++i) {
        shi_test("b64encsize(%u)", i);
        long long result = SHLAG_B64_ENCSIZE(i);
        shi_assert_eq(sizelookup[i], result, "%ll", long long);
        shi_test_end();
    }
}
void b64encsize_test_maxsize()
{
    shi_test("b64_encsize(ENCSIZE_LIMIT): ");
    long long result = SHLAG_B64_ENCSIZE(SHLAG_BTT_ENCSIZE_LIMIT);
    shi_assert_eq(1466015503705LL, result,  "%ll", long long);
    shi_test_end();
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
    b64dec_testsuite();
    b64encsize_testsuite();
    return (shi_test_summary() > 0);
}
