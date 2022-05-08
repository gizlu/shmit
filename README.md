# shmit
shit that don't deserve separate repos

## shlag - single header libs that don't deserve separate repos
| File           | Description |
|----------------|-------------|
|[**shlag/shlag_pcg.h**](shlag/shlag_pcg.h) | 32 bit [pcg](https://www.pcg-random.org/)
pseudorandom generator wrapped in single header C/C++ library along fast, unbiased algorithm
for reducting numbers to specified range. |

### shlag examples
Simple examples are often included within lib header, and are enabled with
macro. You can test them by building files such as `examples/<short_libname>_simple.c`
(for instance [**shlag/examples/pcg_simple.c**](shlag/examples/pcg_simple.c)
