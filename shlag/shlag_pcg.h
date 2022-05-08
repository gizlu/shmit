/* 32bit PCG psuedorandom number generator + fast algorithm for adjusting
 * generated numbers to specified range, wrapped as single file lib
 *
 * Do this: #define SHLAG_PCG_IMPL
 * before you include this file in *one* C or C++ file to create the implementation.
 * You can also optionaly enable assertions by defining SHLAG_PCG_DEBUG
 * Other compile time options: SHLAG_PCG_DEF, SHLAG_PCG_ASSERT, SHLAG_PCG_EXAMPLE
 *
 * Credits:
 * - PCG (Melissa O'Neill, MIT or Apache 2.0 licenses): https://github.com/imneme/pcg-c
 * - fast method for adjusting numbers to specified bound without bias (Daniel Lemire,
 *   Public Domain): https://lemire.me/blog/2016/06/30/fast-random-shuffling/
 *   Link includes benchmark and comparsion with other methods such as modulo
 *
 * I just combined someone else's work into convenient form and wrote some docs.
 * Changes that I introduced, are very trivial. You could consider them to
 * be just patch on MIT/Apache licensed PCG. At end of file you could find license text (MIT)
 * While I tried to make it unbiased I warrant nothing
 *
 * TODO (MAYBE): float number generation, advancing prng by n steps without generating
 * numbers (it may be useful in multithreaded programs)
 */

#ifndef SHLAG_PCG_H
#define SHLAG_PCG_H
#include <stdint.h>

// Prepend public function definitions with whatever you want. You can use it
// for example to make functions static. By default it does nothing
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
// gen random u32 in range <0, @bound). Right side exclusive. @bound shall not be 0
SHLAG_PCG_DEF uint32_t shlag_pcg32_randbound(shlag_pcg32* rng, uint32_t bound);
// gen random u32 in range <@min, @max>. Both sides inclusive. @max shall be greater than @min
SHLAG_PCG_DEF uint32_t shlag_pcg32_randrange(shlag_pcg32* rng, uint32_t min, uint32_t max);

#ifdef __cplusplus
 }
#endif

#endif /* SHLAG_PCG_H */

// Example for seeding prng and generating few numbers.
// See examples/pcg_simple.c for how to compile it.
#ifdef SHLAG_PCG_EXAMPLE
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
int main(int argc, char** argv)
{
    if(argc != 4 && argc != 6) {
        if(argc == 0) exit(1); // someone is screwing with us
        fprintf(stderr, "Usage: %s count min max\n", argv[0]);
        fprintf(stderr, "Usage: %s count min max initstate initseq\n", argv[0]);
        exit(1);
    }
    shlag_pcg32 rng;
    uint64_t initstate, initseq;
    if(argc == 6) { // use user specified seed
        // error handling for strtoul omitted for brevity. Don't do this in real code
        initstate = strtoul(argv[4], NULL, 10);
        initseq = strtoul(argv[5], NULL, 10);
    } else { // seed with current time
        initstate = (uint64_t)time(NULL);
        initseq = initstate;
        // Yes, seeding with current time makes generated output predictable, but I
        // think it is good enough for things like war card game simulation done for fun
        // If you need it to be less predictable you should probably seed with sth like
        // /dev/random and use different values for initstate and initseq.
        // If you need real unpredictability you probably shouldn't be using this lib
        // (Please don't sue me after making your casino go broke).
    }
    shlag_pcg32_srand(&rng, initstate, initseq);
    fprintf(stderr, "prng initstate: %llu, prng initseq: %llu\n",
            (unsigned long long)initstate, (unsigned long long)initseq); 
    // %llu support >= 64 bits so it is safe to cast from uint64

    uint32_t count = strtoul(argv[1], NULL, 10);
    uint32_t min = strtoul(argv[2], NULL, 10);
    uint32_t max = strtoul(argv[3], NULL, 10);
    for(uint32_t i = 0; i < count; ++i)
    {
        printf("%lu ", (unsigned long)shlag_pcg32_randrange(&rng, min, max));
        // ulong support >= 32 bits so it is safe to cast from uint32
    }
    putchar('\n');
}
#endif // SHLAG_PCG_EXAMPLE

#ifdef SHLAG_PCG_IMPL

// you may define SHLAG_PCG_ASSERT(x) yourself for customization
#if defined(SHLAG_PCG_DEBUG) && !defined(SHLAG_PCG_ASSERT) 
 #include <assert.h>
 #define SHLAG_PCG_ASSERT(x) assert((x))
#elif !defined(SHLAG_PCG_DEBUG)
 #define SHLAG_PCG_ASSERT(x) do {} while(0)
#endif

SHLAG_PCG_DEF void shlag_pcg32_srand(shlag_pcg32* rng, uint64_t initstate, uint64_t initseq)
{
    rng->state = 0U;
    rng->inc = (initseq << 1u) | 1u;
    shlag_pcg32_rand(rng);
    rng->state += initstate;
    shlag_pcg32_rand(rng);
    SHLAG_PCG_ASSERT((rng->inc % 2) == 1); // inc must be odd
}

SHLAG_PCG_DEF uint32_t shlag_pcg32_rand(shlag_pcg32* rng) {
    uint64_t oldstate = rng->state;
    rng->state = oldstate * 6364136223846793005ULL + rng->inc;
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

SHLAG_PCG_DEF uint32_t shlag_pcg32_randbound(shlag_pcg32* rng, uint32_t bound) {
    SHLAG_PCG_ASSERT(bound > 0);
    uint64_t random32bit, multiresult;
    uint32_t leftover;
    uint32_t threshold;
    random32bit = shlag_pcg32_rand(rng);
    multiresult = random32bit * bound;
    leftover = (uint32_t) multiresult;
    if(leftover < bound ) {
        threshold = -bound % bound ;
        while (leftover < threshold) {
            random32bit = shlag_pcg32_rand(rng);
            multiresult = random32bit * bound;
            leftover = (uint32_t) multiresult;
        }
    }
    return multiresult >> 32; // <0, bound)
}

SHLAG_PCG_DEF uint32_t shlag_pcg32_randrange(shlag_pcg32* rng, uint32_t min, uint32_t max) 
{
    SHLAG_PCG_ASSERT(max > min);
    const uint32_t rangesize = max-min+1;
    return min + shlag_pcg32_randbound(rng, rangesize);
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
