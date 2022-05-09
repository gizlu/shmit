// Compile it with something like:
// cc -I. -Itests/vendor tests/btt_test.c -o bin/btt_test
// TODO: add makefile or something
#include "vendor/greatest.h"
#define SHLAG_BTT_IMPL
#define SHLAG_BTT_DEF static // optional, but makes smaller exec
#include "shlag_btt.h"

GREATEST_MAIN_DEFS();

// TODO: Test big sizes.

TEST b64EncodeTest(const char* in, const unsigned inSize, const char* expected, char* out)
{
    shlag_b64enc((unsigned char*)in, inSize, (unsigned char*)out);
    ASSERT_STR_EQ(expected, out);
    PASS();
}

// Test inplace and outplace encoding
// This is the cleanest way I found, to avoid long repetitive test cases without
// sharing buffer between unrelated tests (it could potentialy hide OOBs from sanitizer)
#define BASE64_ENC_TEST(plain, expected) \
do { \
    char out[sizeof(expected)]; \
    greatest_set_test_suffix("NotInplace(" plain ")"); \
    RUN_TESTp(b64EncodeTest, plain, sizeof(plain)-1, expected, out); \
    /* reuse buffer for inplace encoding */ \
    memcpy(out, plain, sizeof(plain)-1); \
    greatest_set_test_suffix("Inplace(" plain ")"); \
    RUN_TESTp(b64EncodeTest, out, sizeof(plain)-1, expected, out); \
} while(0)


SUITE(b64StrEncodingSuite) {
    BASE64_ENC_TEST("foo", "Zm9v");
    // RFC 4648 examples
    BASE64_ENC_TEST("", "");
    BASE64_ENC_TEST("f", "Zg==");
    BASE64_ENC_TEST("fo", "Zm8=");
    BASE64_ENC_TEST("foo", "Zm9v");
    BASE64_ENC_TEST("foob", "Zm9vYg==");
    BASE64_ENC_TEST("fooba", "Zm9vYmE=");
    BASE64_ENC_TEST("foobar", "Zm9vYmFy");
    // Wikipedia examples
    BASE64_ENC_TEST("sure.", "c3VyZS4=");
    BASE64_ENC_TEST("sure", "c3VyZQ==");
    BASE64_ENC_TEST("sur", "c3Vy");
    BASE64_ENC_TEST("su", "c3U=");
    BASE64_ENC_TEST("leasure.", "bGVhc3VyZS4=");
    BASE64_ENC_TEST("easure.", "ZWFzdXJlLg==");
    BASE64_ENC_TEST("asure.", "YXN1cmUu");
    BASE64_ENC_TEST("sure.", "c3VyZS4=");
}

SUITE(b64BinEncodingSuite) {
    // RFC 3548 examples
    BASE64_ENC_TEST("\x14\xfb\x9c\x03\xd9\x7e", "FPucA9l+");
    BASE64_ENC_TEST("\x14\xfb\x9c\x03\xd9", "FPucA9k");
    BASE64_ENC_TEST("\x14\xfb\x9c\x03", "FPucAw");
    // My own test cases with nulls
    BASE64_ENC_TEST("\0", "AA==");
    BASE64_ENC_TEST("\0a", "AGE=");
    BASE64_ENC_TEST("a\0b\0", "YQBiAA==");
    BASE64_ENC_TEST("\0\0\0\0", "AAAAAA==");
}

TEST encBufSizeMacroShouldReportRightSize(void) {
    unsigned sizelookup[] = {1,5,5,5,9,9,9,13,13,13,17,17,17,21};
    for(unsigned i = 0; i<sizeof(sizelookup)/sizeof(sizelookup[0]); ++i) {
         ASSERT_EQ(sizelookup[i], SHLAG_B64_ENCSIZE(i));
    }
    PASS();
}


/* padding len macro */
/* padding write */

int main(int argc, char **argv) {
    GREATEST_MAIN_BEGIN();      /* command-line options, initialization. */

    RUN_SUITE(b64StrEncodingSuite);
    /* RUN_SUITE(b64BinEncodingSuite); */
    RUN_TEST(encBufSizeMacroShouldReportRightSize);

    GREATEST_MAIN_END();        /* display results */
}
