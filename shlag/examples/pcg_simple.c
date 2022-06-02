// Example for seeding prng and generating few numbers.
// To build it without buildsystem, run something like:
// cc -I. examples/pcg_simple.c -o bin/pcg_simple
#define SHLAG_PCG_IMPL
#define SHLAG_PCG_DEBUG // optional, for enabling assertions
#define SHLAG_PCG_DEF static // optional, but serves for example
#include "shlag_pcg.h"
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
