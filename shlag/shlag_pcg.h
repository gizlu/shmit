/* 32bit PCG psuedorandom number generator + fast algorithm for adjusting
 * generated numbers to specified range, wrapped as single file lib
 *
 * Do this: #define SHLAG_PCG_IMPL
 * before you include this file in *one* C or C++ file to create the implementation.
 * You can also optionally enable assertions by defining SHLAG_PCG_DEBUG
 * Other compile time options: SHLAG_PCG_DEF, SHLAG_PCG_ASSERT, SHLAG_PCG_EXAMPLE
 *
 * Credits:
 * - PCG (Melissa O'Neill, MIT or Apache 2.0 licenses): https://github.com/imneme/pcg-c
 * - fast method for adjusting numbers to specified bound without bias (Daniel Lemire,
 *   Public Domain): https://lemire.me/blog/2016/06/30/fast-random-shuffling/
 *   Link includes benchmark and comparison with other methods such as modulo
 *
 * I just combined someone else's work into convenient form and wrote some docs.
 * Changes that I introduced, are very trivial. You could consider them to
 * be just patch on MIT/Apache licensed PCG. At end of file you could find license text (MIT)
 * While I tried to make it unbiased I warrant nothing
 *
 * Example: `examples/pcg_simple.c`
 *
 * TODO (MAYBE): float number generation, advancing prng by n steps without generating
 * numbers (it may be useful in multithreaded programs)
 */

#ifndef SHLAG_PCG_H
#define SHLAG_PCG_H
#include <stdint.h>

// Prepend public function definitions with whatever you want. You can use it
// for example to make functions static. By default, it does nothing
#ifndef SHLAG_PCG_DEF
 #define SHLAG_PCG_DEF
#endif

#ifdef __cplusplus
 extern "C" {
#endif

// rng struct. Internals (other than struct size) are private
typedef struct shlag_pcg32{
    uint64_t state; // RNG state.  All values are possible.
    uint64_t inc;   // Controls which RNG sequence (stream) is selected.
                    // Must *always* be odd. srand() takes care that it is
} shlag_pcg32;

// Seed the rng. For this generator, there are 2^63 possible sequences of pseudo-
// random numbers. Each sequence is entirely distinct and has a period of 2^64.
//
// - @initstate specifies where you are in that period, you can pass any 64-bit value.
// - @initseq selects which stream you will use, you can pass any 64-bit value,
// although only the low 63 bits are significant.  
SHLAG_PCG_DEF void shlag_pcg32_srand(shlag_pcg32* rng, uint64_t initstate, uint64_t initseq);

// gen random u32
SHLAG_PCG_DEF uint32_t shlag_pcg32_rand(shlag_pcg32* rng); 
// gen random u32 in range <0, @end). If @end==0, return 0.
SHLAG_PCG_DEF uint32_t shlag_pcg32_randrange0(shlag_pcg32* rng, uint32_t end);
// gen random u32 in range <@begin, @end). If @begin==@end, return that number
SHLAG_PCG_DEF uint32_t shlag_pcg32_randrange(shlag_pcg32* rng, uint32_t begin, uint32_t end);

#ifdef __cplusplus
 }
#endif

#endif /* SHLAG_PCG_H */


#ifdef SHLAG_PCG_IMPL

SHLAG_PCG_DEF void shlag_pcg32_srand(shlag_pcg32* rng, uint64_t initstate, uint64_t initseq)
{
    rng->state = 0U;
    rng->inc = (initseq << 1u) | 1u;
    shlag_pcg32_rand(rng);
    rng->state += initstate;
    shlag_pcg32_rand(rng);
}

SHLAG_PCG_DEF uint32_t shlag_pcg32_rand(shlag_pcg32* rng) {
    uint64_t oldstate = rng->state;
    rng->state = oldstate * 6364136223846793005ULL + rng->inc;
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

SHLAG_PCG_DEF uint32_t shlag_pcg32_randrange0(shlag_pcg32* rng, uint32_t end) {
    uint64_t random32bit, multiresult;
    uint32_t leftover;
    uint32_t threshold;
    random32bit = shlag_pcg32_rand(rng);
    multiresult = random32bit * end;
    leftover = (uint32_t) multiresult;
    if(leftover < end ) {
        threshold = -end % end ;
        while (leftover < threshold) {
            random32bit = shlag_pcg32_rand(rng);
            multiresult = random32bit * end;
            leftover = (uint32_t) multiresult;
        }
    }
    return multiresult >> 32; // <0, end)
}

SHLAG_PCG_DEF uint32_t shlag_pcg32_randrange(shlag_pcg32* rng, uint32_t begin, uint32_t end)
{
    const uint32_t rangesize = end-begin;
    return begin + shlag_pcg32_randrange0(rng, rangesize);
}

#endif // SHLAG_PCG_IMPL

/*
Copyright (c) 2014-2017 Melissa O'Neill and PCG Project contributors

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
