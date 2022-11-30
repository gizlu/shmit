# shmit
stuff that does not deserve to be split into multiple repos

## scripts
| File           | Description |
|----------------|-------------|
| [**scripts/argvtoc**](scripts/argvtoc) | shellscript for printing argv as c array. Useful for debugging and calling exec\*() by hand. **Public domain**|
| [**scripts/screenshot**](scripts/screenshot) | shellscript for doing screenshots on x11 and saving them in clipoard and disk. **Public domain**|
| [**scripts/marktable**](scripts/marktable) | shellscript for converting tabular data into markdown syntax. **Public domain** |
| [**scripts/gmintbl**](scripts/gmintbl) | shellscript for minifing google benchmark output. **Public domain** |

## shlag - single header libs libs that don't deserve separate repos
| File           | Description |
|----------------|-------------|
|[**rundeck.h**](shlag/rundeck.h) | Implematation of Legends of Runeterra's card deck serialization algorithm. **USTABLE, unpolished and untested** |
|[**shitest.h**](shlag/shitest.h) | Minimal unittesting lib. Written due to my discontent with fullblown frameworks like gtest. **UNSTABLE** |
|[**shlag_b64.h**](shlag/shlag_b64.h) | base64 implementation with support for inplace enc/dec. **UNSTABLE** |
|[**shlag_pcg.h**](shlag/shlag_pcg.h) | 32 bit [pcg prng](https://www.pcg-random.org/) wrapped in single header lib along [fast, unbiased algo](https://lemire.me/blog/2016/06/30/fast-random-shuffling/) for randrange(). **STABLE, MIT Licensed** |

There are examples in `shlag/examples/` and tests in `shlag/tests/`

### building tests and examples
In root project dir run:
```
meson setup build/
meson compile -C build/
```
executables will be outputed into build/ dir

For quick check you can use `meson test -C build` which will run some tests
and examples. NOTE: output is way less verbose than when running them by hand
