# shmit
shit that don't deserve separate repos

## scripts
| File           | Description |
|----------------|-------------|
| [**scripts/argv_to_c.sh**](scripts/argv_to_c.sh) | dead simple shell script for printing argv as c array. Useful for debugging and calling exec*() by hand. Inspired by [ImgToC.c](https://github.com/DanielGibson/Snippets/blob/master/ImgToC.c). |

## shlag - single header libs that don't deserve separate repos
| File           | Description |
|----------------|-------------|
|[**shlag/shlag_pcg.h**](shlag/shlag_pcg.h) | 32 bit [pcg](https://www.pcg-random.org/) pseudorandom generator wrapped in single header C/C++ library along fast, unbiased algorithm for reducting numbers to specified range. MIT licensed. |

### shlag examples
Simple examples are often included within lib header, and are enabled with
macro. You can test them by building files such as [**shlag/examples/pcg_simple.c**](shlag/examples/pcg_simple.c)
