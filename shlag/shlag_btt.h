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

// calc buffer size needed for encoding n bytes as base64 with padding (including null terminator)
#define SHLAG_B64_ENCSIZE(n) ((int64_t)(n) + 2)/3 * 4  + 1

// encode @in buffer, of specified size into @out buffer.
// @out and @in may point to same buffer - output will just overwrite input
// size of @out shall be <= SHLAG_B64_ENCSIZE(inSize). It is assumed that
// SHLAG_B64_ENCSIZE(inSize) fits in int64_t
SHLAG_BTT_DEF void shlag_b64enc(const uint8_t* in, int64_t inSize, char* out);

// cal size of @out buffer required for decoding @in of lenght @len
// Note: it may return slightly more than strictly needed (when padding used)
#define SHLAG_B64_DECSIZE(len) (((int64_t)(len)*3)/4)

// decode @in buffer, of specified lenght into @out buffer.
// @out and @in may point to same buffer - output will just overwrite input
// On success returns count of written bytes. On fail returns negative number
SHLAG_BTT_DEF int64_t shlag_b64dec(const char* in, int64_t inLen, uint8_t* out);

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

#define SHLAG_B64_MAX_VALID 63 // 0b00111111
#define SHLAG_B64_BAD_CH 64    // 0b01000000
#define SHLAG_B64_PAD_CH 128   // 0b10000000
// INVALID_CH and PAD_CH are bitflags used for input validation

// Lookup table for converting base64 character into 6-bit binary
// Invalid chars are inited with 64. '=' padding is represented as 128
// generated with script: https://gist.github.com/gizlu/14c5d930241244b8045f3043f8883d93
static const uint8_t shlag_b64dec_lookup[256] = {64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,62,64,64,64,63,52,53,54,55,56,57,58,59,60,61,64,64,64,128,64,64,64,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,64,64,64,64,64,64,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64};

// validate n bytes of input.
// Set SHLAG_B64_PAD_CH bitflag on @status if `=` occured
// Set SHLAG_B64_INALID_CH bitflag on @status if invalid char occured
// other bits are arbitrary overwritten
static inline void shlag_b64dec_validate(const uint8_t* in, int64_t n, uint8_t* status)
{
    for(int i = 0; i<n; ++i) {
        *status |= shlag_b64dec_lookup[in[i]];
    }
}
// decode normal block of 4 bytes (result is 3 bytes)
static inline void shlag_b64dec_quartet(const uint8_t* in, uint8_t* out)
{
    out[0] = (shlag_b64dec_lookup[in[0]] << 2) | (shlag_b64dec_lookup[in[1]] >> 4);
    out[1] = (shlag_b64dec_lookup[in[1]] << 4) | (shlag_b64dec_lookup[in[2]] >> 2);
    out[2] = (shlag_b64dec_lookup[in[2]] << 6) | (shlag_b64dec_lookup[in[3]]);
}
// decode last block of specified size (2,3 or 4)
static inline void shlag_b64dec_lastblock(const uint8_t* in, uint8_t* out, uint8_t blocksize)
{
    // if(blocksize >= 2) {  // always true
        out[0] = (shlag_b64dec_lookup[in[0]] << 2) | (shlag_b64dec_lookup[in[1]] >> 4);
    // }
    if(blocksize >= 3) {
        out[1] = (shlag_b64dec_lookup[in[1]] << 4) | (shlag_b64dec_lookup[in[2]] >> 2);
    }
    if(blocksize == 4) {
        out[2] = (shlag_b64dec_lookup[in[2]] << 6) | (shlag_b64dec_lookup[in[3]]);
    }
}
SHLAG_BTT_DEF int64_t shlag_b64dec(const char* in, int64_t inLen, uint8_t* out)
{
    if(inLen == 0) return 0;
    uint8_t status = SHLAG_B64_MAX_VALID;
    int64_t i = 0, j = 0;
    while(i + 4 < inLen) {
        shlag_b64dec_validate((uint8_t*)in + i, 4, &status);
        shlag_b64dec_quartet((uint8_t*)in + i, out + j);
        i += 4; j += 3;
    }
    if(status & SHLAG_B64_PAD_CH) return -1; // padding can't occur outside last block

    // last block is decoded differently (cause its size varies, and it may use padding)
    int64_t blocksize = 0;
    while((i + blocksize) < inLen && in[i + blocksize] != '=') {
        ++blocksize;
    }
    // validation
    if(blocksize == 1) return -1; // one byte leftover is impossible in valid b64
    shlag_b64dec_validate((uint8_t*)in + i, blocksize, &status);
    if(status & SHLAG_B64_BAD_CH) return -1;
    for(int k = i + blocksize + 1; k < inLen; ++k) {
        if(in[k] != '=') return -1; // after last block, only padding is allowed
    }

    shlag_b64dec_lastblock((uint8_t*)in + i, out + j, blocksize);
    return j + blocksize - 1;
}
#endif // SHLAG_BTT_IMPL
