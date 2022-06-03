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

void b64enc_test(TestPair p, bool inplace)
{
    // We don't use one big buffer for all tests despite we can, because it
    // it could potentialy hide OOB bugs from sanitizer
    char* buf = malloc(p.encodedLen + 1);
    if(inplace) {
        shi_test("b64enc_inplace(%s)", p.plainStringized);
        memcpy(buf, p.plain, p.plainSize);
        shlag_b64enc((uint8_t*)buf, p.plainSize, buf);
    } else {
        shi_test("b64enc(%s)", p.plainStringized);
        shlag_b64enc(p.plain, p.plainSize, buf);
    }
    shi_assert_streq(p.encoded, buf);
    shi_test_end();
    free(buf);
}

void b64dec_test(TestPair p, bool inplace)
{
    uint8_t* buf = malloc(p.plainSize);
    if(inplace) {
        shi_test("b64dec_inplace(\"%s\")", p.encoded);
        memcpy(buf, p.encoded, p.encodedLen+1);
        shlag_b64dec((char*)buf, p.encodedLen, buf);
    } else {
        shi_test("b64dec(\"%s\")", p.encoded);
        shlag_b64dec(p.encoded, p.encodedLen, buf);
    }
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
#define ARRSIZE(arr) sizeof(arr)/sizeof(arr[0])
void b64dec_padded_testsuite(bool inplace)
{
    fprintf(stderr, "test b64dec %s, with valid, padded data\n",
            inplace ? "into same buffer (inplace)" : "into external buffer");
    for(unsigned i = 0; i<ARRSIZE(valid_b64_pairs); ++i) {
        b64dec_test(valid_b64_pairs[i], inplace);
    }
    fputs(SHI_SEP, stderr);
}
void b64enc_testsuite(bool inplace)
{
    fprintf(stderr, "test b64enc %s\n",
            inplace ? "into same buffer (inplace)" : "into external buffer");
    for(unsigned i = 0; i<ARRSIZE(valid_b64_pairs); ++i) {
        b64enc_test(valid_b64_pairs[i], inplace);
    }
    fputs(SHI_SEP, stderr);
}
// return unpadded copy of suplied string (aka with `=` chars removed).
// It assumes valid encoding. You have to free it yourself
char* unpad(char* in, unsigned inLen, unsigned* outLen)
{
    char* padStart = strchr(in, '=');
    *outLen = (padStart == NULL) ? inLen : padStart - in;
    char* out = malloc(*outLen+1);
    memcpy(out, in, *outLen);
    out[*outLen] = '\0';
    return out;
}
void b64dec_unpadded_testsuite(bool inplace)
{
    fprintf(stderr, "b64dec%s with valid, unpadded data\n", inplace ? " inplace" : "");
    for(unsigned i = 0; i<ARRSIZE(valid_b64_pairs); ++i) {
        TestPair p = valid_b64_pairs[i];
        p.encoded = unpad(p.encoded, p.encodedLen, &p.encodedLen);
        b64dec_test(p, inplace);
        free(p.encoded);
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
    shi_test("b64encsize(ENCSIZE_LIMIT)");
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
    enum {OUTPLACE = 0, INPLACE = 1};
    b64enc_testsuite(OUTPLACE);
    b64enc_testsuite(INPLACE);
    b64dec_padded_testsuite(OUTPLACE);
    b64dec_padded_testsuite(INPLACE);
    b64dec_unpadded_testsuite(OUTPLACE);
    b64dec_unpadded_testsuite(INPLACE);
    b64encsize_testsuite();
    return (shi_test_summary() > 0);
}
