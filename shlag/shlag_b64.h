/* two file implementation of BinaryToText encodings such as base32 and base64
 *
 * encoder is able to encode inplace (you can overwrite input by output - there is
 * no need for another buffer). I haven't yet seen C lib able to do that.
 * Inspired by: https://github.com/dotnet/corefxlab/pull/834/files
 *
 * used namespaces: shlag_b64, SHLAG_B64
 *
 * In *one* of C or C++ file, you have to define SHLAG_B64_IMPL before including
 * shlag_btt.h. See `shlag/tests/shlag_btt.c` 
 */

#ifndef SHLAG_B64_H
#define SHLAG_B64_H

#include <stdint.h>

// Prepend public function definitions with whatever you want. You can use it
// for example to make functions static. By default it does nothing
#ifndef SHLAG_B64_DEF
 #define SHLAG_B64_DEF
#endif

#ifdef __cplusplus
 extern "C" {
#endif

// calc buffer size needed for encoding n bytes as base64 with padding (including null terminator)
#define SHLAG_B64_ENCSIZE(n) ((int64_t)(n) + 2)/3 * 4  + 1

// encode @in buffer, of specified size into @out buffer.
// @out and @in may point to same buffer - output will just overwrite input
// size of @out shall be >= SHLAG_B64_ENCSIZE(inSize). It is assumed that
// SHLAG_B64_ENCSIZE(inSize) fits in int64_t
SHLAG_B64_DEF void shlag_b64enc(const uint8_t* in, int64_t inSize, char* out);

// cal size of @out buffer required for decoding @in of length @len
// Note: it may return slightly more than strictly needed (when padding used)
#define SHLAG_B64_DECSIZE(len) (((int64_t)(len)*3)/4)

// decode @in buffer, of specified length into @out buffer.
// @out and @in may point to same buffer - output will just overwrite input
// On success returns count of written bytes. On fail returns negative number
SHLAG_B64_DEF int64_t shlag_b64dec(const char* in, int64_t inLen, uint8_t* out);

#ifdef __cplusplus
 }
#endif
#endif // SHLAG_B64_H

// private stuff

#ifdef SHLAG_B64_IMPL
// lookup table for converting six-bit binary into base64 character
static const char* shlag_b64chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// encode normal block of 3 bytes (result is 4 bytes)
static inline void shlag_b64enc_triple(const uint8_t* in, char* out)
{
    // yes, it is kinda hard to read as we encode from backwards (to make inplace enc possible)
    out[3] = shlag_b64chars[in[2] & 0x3F]; // just 6LSB from 3rd byte
    out[2] = shlag_b64chars[((in[1] & 0xF) << 2) | (in[2] >> 6)]; // 4LSB from 2nd, 2MSB from 3rd
    out[1] = shlag_b64chars[((in[0] & 0x3) << 4) | (in[1] >> 4)]; // 2LSB from 1st, 4MSB from 2nd
    out[0] = shlag_b64chars[in[0] >> 2]; // just 6MSB from 1st
}

// encode 1 or 2 byte "leftover" block that goes after packs of threes and pad it with `=` to 4 bytes
static inline void shlag_b64enc_leftover(const uint8_t* in, char* out, uint8_t leftover)
{
    // yes, it is kinda hard to read as we encode from backwards (to make inplace enc possible)
    if(leftover == 2) {
        out[3] = '='; // padding
        out[2] = shlag_b64chars[(in[1] & 0xF) << 2]; // 4 LSB from 2nd leftover byte
        out[1] = shlag_b64chars[(in[0] & 0x3) << 4 | (in[1] >> 4)]; // 2LSB from 1st, then 4MSB from 2nd
        out[0] = shlag_b64chars[in[0] >> 2]; // 6MSB from 1st byte
    } else { /* leftover == 1 */
        out[3] = '='; // padding
        out[2] = '=';
        out[1] = shlag_b64chars[(in[0] & 0x3) << 4]; // 2LSB from leftover byte
        out[0] = shlag_b64chars[in[0] >> 2]; // 6MSB from leftover byte
    }
}

void shlag_b64enc(const uint8_t* in, int64_t inSize, char* out)
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
        shlag_b64enc_triple(in + inSize, out + outLen);
    }
}

#define SHLAG_B64_MAX_VALID 63 // 0b00111111
#define SHLAG_B64_BAD 64    // 0b01000000
#define SHLAG_B64_PAD 128   // 0b10000000
// BAD and PAD are bitflags used for input validation

// Lookup table for converting base64 character into 6-bit binary
// Invalid chars are inited with 64. '=' padding is represented as 128
// generated with script: https://gist.github.com/gizlu/14c5d930241244b8045f3043f8883d93
static const uint8_t shlag_b64bits[256] = {64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,62,64,64,64,63,52,53,54,55,56,57,58,59,60,61,64,64,64,128,64,64,64,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,64,64,64,64,64,64,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64};

// decode normal block of 4 bytes (result is 3 bytes)
// Returns "status" - byte that you can check for BAD_CH and PAD_CH bitflags
static inline uint8_t shlag_b64dec_four(const uint8_t* in, uint8_t* out)
{
    // compiler seem to output less retarded asm with these tmp vars
    uint8_t a = shlag_b64bits[in[0]], b = shlag_b64bits[in[1]], c = shlag_b64bits[in[2]], d = shlag_b64bits[in[3]];
    out[0] = (a << 2) | (b >> 4);
    out[1] = (b << 4) | (c >> 2);
    out[2] = (c << 6) | (d);
    return a | b | c | d;
}

// decode last block of specified size (2,3 or 4). It doesn't return status
static inline void shlag_b64dec_last(const uint8_t* in, uint8_t* out, uint8_t blocksize)
{
    // if(blocksize >= 2) {  // always true
        out[0] = (shlag_b64bits[in[0]] << 2) | (shlag_b64bits[in[1]] >> 4);
    // }
    if(blocksize >= 3) {
        out[1] = (shlag_b64bits[in[1]] << 4) | (shlag_b64bits[in[2]] >> 2);
    }
    if(blocksize == 4) {
        out[2] = (shlag_b64bits[in[2]] << 6) | (shlag_b64bits[in[3]]);
    }
}
int64_t shlag_b64dec(const char* in, int64_t inLen, uint8_t* out)
{
    if(inLen == 0) return 0;
    uint8_t status = 0;
    int64_t i = 0, j = 0;

    while(i + 4 < inLen) {
        status |= shlag_b64dec_four((uint8_t*)in + i, out + j);
        i += 4; j += 3;
    }
    if(status & SHLAG_B64_PAD) return -1; // padding can't occur outside last block

    // last block is decoded differently (cause its size varies, and it may use padding)
    int64_t blocksize = 0;
    for(; i+blocksize < inLen; ++blocksize) {
        status |= shlag_b64bits[(uint8_t)in[i+blocksize]];
        if(status & SHLAG_B64_PAD) break;
    }
    if(status & SHLAG_B64_BAD) return -1;
    if(blocksize == 1) return -1; // one byte leftover is impossible in valid b64
    for(int64_t k = i + blocksize + 1; k < inLen; ++k) {
        if(in[k] != '=') return -1; /* only padding is legal after last block */
    }

    shlag_b64dec_last((uint8_t*)in + i, out + j, blocksize);
    return j + blocksize - 1;
}
#endif // SHLAG_B64_IMPL
