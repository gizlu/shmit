/* two file implementation of BinaryToText encodings such as base32 and base64
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

#ifdef __cplusplus
 extern "C" {
#endif

// calc buffer size needed for encoding n bytes as base64 with padding (including null terminator)
#define SHLAG_B64_ENCSIZE(n) ((int64_t)(n) + 2)/3 * 4  + 1

// encode @in buffer, of specified size into @out buffer.
// @out and @in may point to same buffer - output will just overwrite input
// size of @out shall be >= SHLAG_B64_ENCSIZE(inSize). It is assumed that
// SHLAG_B64_ENCSIZE(inSize) fits in int64_t
void shlag_b64enc(const uint8_t* in, int64_t inSize, char* out);

// cal size of @out buffer required for decoding @in of lenght @len
// Note: it may return slightly more than strictly needed (when padding used)
#define SHLAG_B64_DECSIZE(len) (((int64_t)(len)*3)/4)

// decode @in buffer, of specified lenght into @out buffer.
// @out and @in may point to same buffer - output will just overwrite input
// On success returns count of written bytes. On fail returns negative number
int64_t shlag_b64dec(const char* in, int64_t inLen, uint8_t* out);

#ifdef __cplusplus
 }
#endif
#endif // SHLAG_BTT_H
