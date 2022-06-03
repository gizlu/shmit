/* single file implementation of BinaryToText encodings such as base32 and base64
 *
 * Define SHLAG_BTT_IMPL before you include this file in *one* C or C++ file to create the implementation.
*
 * encoder is able to encode inplace (you can overwrite input by output - there is
 * no need for another buffer). I haven't yet seen C lib able to do that.
 * Inspired by: https://github.com/dotnet/corefxlab/pull/834/files
 *
 * used namespaces: shlag_b64, SHLAG_B64, SHLAG_BTT
 */
#ifndef SHLAG_BTT_H
#define SHLAG_BTT_H

#include <stdint.h>

// Prepend public function definitions with whatever you want. You can use it
// for example to make functions static. By default it does nothing
#ifndef SHLAG_BTT_DEF
 #define SHLAG_BTT_DEF
#endif

#ifdef __cplusplus
 extern "C" {
#endif

// Just arbitrary choosen limit of encoded input size. *enc() and *ENCSIZE() assume that @inSize <= BTT_ENCSIZE_LIMIT
// Very likely it could be increased but I don't warrant it because:
// - I'm not sure, and unable to test it (ackshualy I have one retarded hack in mind - mmap on highly compressed fs)
// - 1TB is more than enough. You gotta be insane to use base64 or base32 for such a big shit
// I reserve right to change it and not call it API break (just in case having 2138TB RAM becomes the standard or something)
#define SHLAG_BTT_ENCSIZE_LIMIT (int64_t)1024 * 1024 * 1024 * 1024

// calc buffer size needed for encoding n bytes as base64 with padding (including null terminator)
#define SHLAG_B64_ENCSIZE(n) ((int64_t)(n) + 2)/3 * 4  + 1

// encode @in buffer, of specified size into @out buffer.
// @out and @in may point to same buffer - output will just overwrite input
// size of @out shall be <= SHLAG_B64_ENCSIZE(inSize)
SHLAG_BTT_DEF void shlag_b64enc(const uint8_t* in, int64_t inSize, char* out);

#ifdef __cplusplus
 }
#endif
#endif // SHLAG_BTT_H

// Implementation. Mainly private things
#ifdef SHLAG_BTT_IMPL

// lookup table for converting six-bit binary into base64 character
static const char* shlag_b64enc_lookup = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// encode normal block of 3 bytes (result is 4 bytes)
static inline void shlag_b64enc_triplet(const uint8_t* in, char* out)
{
    // yes, it is kinda hard to read as we encode from backwards (to make inplace enc possible)
    out[3] = shlag_b64enc_lookup[in[2] & 0x3F]; // just 6LSB from 3rd byte
    out[2] = shlag_b64enc_lookup[((in[1] & 0xF) << 2) | (in[2] >> 6)]; // 4LSB from 2nd, 2MSB from 3rd
    out[1] = shlag_b64enc_lookup[((in[0] & 0x3) << 4) | (in[1] >> 4)]; // 2LSB from 1st, 4MSB from 2nd
    out[0] = shlag_b64enc_lookup[in[0] >> 2]; // just 6MSB from 1st
}

// encode 1 or 2 byte "leftover" block that goes after packs of threes and pad it with `=` to 4 bytes
static inline void shlag_b64enc_leftover(const uint8_t* in, char* out, uint8_t leftover)
{
    // yes, it is kinda hard to read as we encode from backwards (to make inplace enc possible)
    if(leftover == 2) {
        out[3] = '='; // padding
        out[2] = shlag_b64enc_lookup[(in[1] & 0xF) << 2]; // 4 LSB from 2nd lefover byte
        out[1] = shlag_b64enc_lookup[(in[0] & 0x3) << 4 | (in[1] >> 4)]; // 2LSB from 1st, then 4MSB from 2nd
        out[0] = shlag_b64enc_lookup[in[0] >> 2]; // 6MSB from 1st byte
    } else { /* leftover == 1 */
        out[3] = '='; // padding
        out[2] = '=';
        out[1] = shlag_b64enc_lookup[(in[0] & 0x3) << 4]; // 2LSB from leftover byte
        out[0] = shlag_b64enc_lookup[in[0] >> 2]; // 6MSB from leftover byte
    }
}

SHLAG_BTT_DEF void shlag_b64enc(const uint8_t* in, int64_t inSize, char* out)
{
    // we encode in backwards order to avoid overwriting not yet encoded data (to make inplace enc possible)
    int64_t outLen = SHLAG_B64_ENCSIZE(inSize) - 1;
    out[outLen] = '\0';
    const uint8_t leftover = inSize % 3; // how many bytes after packs of 3
    if(leftover) {
        outLen -= 4; inSize -= leftover;
        shlag_b64enc_leftover(in + inSize, out + outLen, leftover);
    }
    while(inSize > 0) {
        outLen -= 4;
        inSize -= 3;
        shlag_b64enc_triplet(in + inSize, out + outLen);
    }
}

// lookup table for converting base64 character into 6-bit binary
// Wrong characters return 0
static const uint8_t shlag_b64dec_lookup[256] = { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 62, 63, 62, 62, 63, 52, 53, 54, 55,
56, 57, 58, 59, 60, 61,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  6,
7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,  0,
0,  0,  0, 63,  0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51 };

// decode normal block of 4 bytes (result is 3 bytes)
static inline void shlag_b64dec_quartet(const uint8_t* in, uint8_t* out)
{
    out[0] = (shlag_b64dec_lookup[in[0]] << 2) | (shlag_b64dec_lookup[in[1]] >> 4);
    out[1] = (shlag_b64dec_lookup[in[1]] << 4) | (shlag_b64dec_lookup[in[2]] >> 2);
    out[2] = (shlag_b64dec_lookup[in[2]] << 6) | (shlag_b64dec_lookup[in[3]]);
}


static inline void shlag_b64dec_leftover(const uint8_t* in, uint8_t* out, uint8_t leftover)
{
    // if leftover exists it is always 2 or 3 char long
    if(leftover >= 2) {
        out[0] = (shlag_b64dec_lookup[in[0]] << 2) | (shlag_b64dec_lookup[in[1]] >> 4);
    }
    if(leftover == 3) {
        out[1] = (shlag_b64dec_lookup[in[1]] << 4) | (shlag_b64dec_lookup[in[2]] >> 2);
    }
}
SHLAG_BTT_DEF void shlag_b64dec(const char* in, int64_t inLen, uint8_t* out)
{
    int64_t i = 0, j = 0;
    while(i + 4 <= inLen) {
        shlag_b64dec_quartet((uint8_t*)in + i, out + j);
        i += 4; j += 3;
    }
    // in case output is unpadded
    int64_t leftover = inLen - i;
    shlag_b64dec_leftover((uint8_t*)in + i, out + j, leftover);
    // TODO (maybe): add some input validation
}
#endif // SHLAG_BTT_IMPL
