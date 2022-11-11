// Compile it with something like:
// cc -I. tests/b64test.c -o bin/b64test
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#define SHLAG_B64_IMPL
#define SHLAG_B64_DEF static // not needed, used just for presetation
#include "shlag_b64.h"
#define SHITEST_IMPL
#define SHITEST_DEF static // not needed, used just for presetation
#include "shitest.h"

typedef struct TestPair
{
    uint8_t* plain; // plain, possibly binary data
    char* plainStringized; // stringized form of possibly binary @plain, meant for debug info
    char* encoded;
    int64_t plainSize;
    int64_t encodedLen;
} TestPair;

// construct test pair.
// Last byte of @plain (preasumbly null terminator) is ignored
#define TEST_PAIR(plain, encoded) \
{(uint8_t*)(plain), #plain, encoded, sizeof((plain))-1, strlen(encoded)}

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

void b64enc_test(TestPair p, bool inplace)
{
    shi_test("b64enc(%s)", p.plainStringized);
    // We don't use one big buffer for all tests despite we can, because it
    // it could potentialy hide OOB bugs from sanitizer
    uint8_t* in;
    char* out = malloc(p.encodedLen + 1);
    if(inplace) {
        in = (uint8_t*)out;
        memcpy(in, p.plain, p.plainSize);
    } else {
        in = p.plain;
    }
    shlag_b64enc(in, p.plainSize, out);
    shi_assert_streq(p.encoded, out);
    free(out);
    shi_test_end();
}

void b64enc_testsuite(bool inplace)
{
    fprintf(stderr, "test %s b64enc\n", inplace ? "inplace" : "outplace");
    for(unsigned i = 0; i<ARRSIZE(valid_b64_pairs); ++i) {
        b64enc_test(valid_b64_pairs[i], inplace);
    }
    fputs(SHI_SEP, stderr);
}

void b64encsize_test(int64_t in, int64_t expected)
{
    shi_test("b64encsize(%lld)", in);
    shi_assert_eq(expected, SHLAG_B64_ENCSIZE(in), "%lld", int64_t);
    shi_test_end();
}

void b64encsize_testsuite()
{
    fprintf(stderr, "test if b64encsize() returns correct sizes\n");
    int64_t expected_lookup[] = {1,5,5,5,9,9,9,13,13,13,17,17,17,21};
    for(unsigned i = 0; i<ARRSIZE(expected_lookup); ++i) {
        b64encsize_test(i, expected_lookup[i]);
    }
    // max input not causing overfow
    b64encsize_test(6917529027641081853, 9223372036854775805);
    fputs(SHI_SEP, stderr);
}

void* memdup(void* src, size_t n)
{
    void* ptr = malloc(n);
    memcpy(ptr, src, n);
    return ptr;
}

void b64dec_test(TestPair p, bool inplace)
{
    shi_test("b64dec(\"%s\")", p.encoded);
    char* in; uint8_t* out;
    int64_t outsize;
    if(inplace) {
        in = memdup(p.encoded, p.encodedLen+1);
        out = (uint8_t*)in;
    } else {
        in = p.encoded;
        out = malloc(p.plainSize);
    }
    outsize = shlag_b64dec(in, p.encodedLen, out);
    shi_assert_f(p.plainSize == outsize, 
            "expected_size: %lld, actual_size: %lld", p.plainSize, outsize);
    shi_assert_memeq_f(p.plain, out, p.plainSize, "out != %s", p.plainStringized);
    shi_test_end();
    free(out);
}

void b64dec_padded_testsuite(bool inplace)
{
    fprintf(stderr, "test %s b64dec with valid padded data\n", 
            inplace ? "inplace" : "outplace");
    for(unsigned i = 0; i<ARRSIZE(valid_b64_pairs); ++i) {
        b64dec_test(valid_b64_pairs[i], inplace);
    }
    fputs(SHI_SEP, stderr);
}
// return unpadded copy of suplied string (aka with `=` chars removed).
// It assumes valid encoding. You have to free it yourself
char* unpad(char* in, int64_t inLen, int64_t* outLen)
{
    char* padStart = strchr(in, '=');
    *outLen = (padStart == NULL) ? inLen : padStart - in;
    char* out = memdup(in, *outLen+1);
    out[*outLen] = '\0';
    return out;
}
void b64dec_unpadded_testsuite(bool inplace)
{
    fprintf(stderr, "test %s b64dec with valid unpadded data\n",
            inplace ? "inplace" : "outplace");
    for(unsigned i = 0; i<ARRSIZE(valid_b64_pairs); ++i) {
        TestPair p = valid_b64_pairs[i];
        p.encoded = unpad(p.encoded, p.encodedLen, &p.encodedLen);
        b64dec_test(p, inplace);
        free(p.encoded);
    }
    fputs(SHI_SEP, stderr);
}
void b64decsize_test(int64_t in, int64_t expected)
{
    shi_test("b64decsize(%lld)", in);
    shi_assert_eq(expected, SHLAG_B64_DECSIZE(in), "%lld", int64_t);
    shi_test_end();
}
void b64decsize_testsuite()
{
    fprintf(stderr, "test if b64decsize() returns correct sizes\n");
    // input 1, 5, 9, 13 is invalid, but we also test it, as it can be passed to b64dec
    int64_t expected_lookup[] = {0,0,1,2,3,3,4,5,6,6,7,8,9,9,10};
    for(unsigned i = 0; i<ARRSIZE(expected_lookup); ++i) {
        b64decsize_test(i, expected_lookup[i]);
    }
    // max input not causing overfow
    b64decsize_test(3074457345618258602, 2305843009213693951);
    fputs(SHI_SEP, stderr);
}
void b64dec_invalid_test(char* in, int inLen)
{
    shi_test("b64dec(\"%s\")", in);
    uint8_t* buf = malloc(SHLAG_B64_DECSIZE(inLen));
    shi_assert_eq(-1, shlag_b64dec(in, inLen, buf), "%lld", int64_t);
    free(buf);
    shi_test_end();
}
void b64dec_null_ch_test()
{
    shi_test("b64_dec(\"a\\0\")");
    uint8_t* buf = malloc(SHLAG_B64_DECSIZE(2));
    shi_assert_eq(-1, shlag_b64dec("a\\0", 2, buf), "%lld", int64_t);
    free(buf);
    shi_test_end();
}
void b64dec_invalid_testsuite()
{
    fprintf(stderr, "test if b64dec reports error on invalid char\n");
    b64dec_invalid_test("a}", 2);
    b64dec_invalid_test("a ", 2);
    b64dec_null_ch_test();
    fprintf(stderr, "test if b64dec reports error on invalid lenght\n");
    b64dec_invalid_test("a", 1);
    b64dec_invalid_test("aaaaa", 5);
    fprintf(stderr, "test if b64dec reports error on invalid padding\n");
    b64dec_invalid_test("aa=====", 7);
    b64dec_invalid_test("aa======", 8);
    b64dec_invalid_test("aa=======", 9);
    b64dec_invalid_test("aa=aaa", 6);
    b64dec_invalid_test("aa=a", 4);
    b64dec_invalid_test("a=a", 3);
    b64dec_invalid_test("=a", 2);
    b64dec_invalid_test("a=", 2);
    fputs(SHI_SEP, stderr);
}
int main()
{
    enum {OUTPLACE = 0, INPLACE = 1};
    b64enc_testsuite(OUTPLACE);
    b64enc_testsuite(INPLACE);
    b64encsize_testsuite();

    b64dec_padded_testsuite(OUTPLACE);
    b64dec_padded_testsuite(INPLACE);
    b64dec_unpadded_testsuite(OUTPLACE);
    b64dec_unpadded_testsuite(INPLACE);

    b64decsize_testsuite();
    b64dec_invalid_testsuite();
    return (shi_test_summary() > 0);
}
