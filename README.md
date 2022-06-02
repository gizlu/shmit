# shmit
shit that don't deserve separate repos

## scripts
| File           | Description |
|----------------|-------------|
| [**scripts/argv_to_c.sh**](scripts/argv_to_c.sh) | dead simple shell script for printing argv as c array. Useful for debugging and calling exec\*() by hand. Inspired by [ImgToC.c](https://github.com/DanielGibson/Snippets/blob/master/ImgToC.c). |
| [**scripts/screenshot.sh**](scripts/screenshot.sh) | dead simple shell script for doing screenshots of selected area/current window/all displays and saving them in clipoard and disk. Depends on maim and xclip |

## shlag - single header libs that don't deserve separate repos
| File           | Description |
|----------------|-------------|
|[**shlag/shitest.h**](shlag/shitest.h) | Minimalistic single header testing library written due to my discontent with fullblown frameworks like gtest. TODO: stabilize API/ABI, maybe write script converting output to TAP format |
|[**shlag/shlag_btt.h**](shlag/shlag_btt.h) | Single header lib implementation of binary to text encodings such as base64. It supports in-place encoding, so you don't have to allocate separate buffer. Currently only base64 encoder is implemented (TODO: b64 decoder, other formats like b32, examples and stabilizing API/ABI - it is not stable yet). |
|[**shlag/shlag_pcg.h**](shlag/shlag_pcg.h) | 32 bit [pcg pseudorandom generator](https://www.pcg-random.org/) wrapped in single header C/C++ library along [fast, unbiased algorithm](https://lemire.me/blog/2016/06/30/fast-random-shuffling/) for reducting numbers to specified range. I just combined [lemire](https://github.com/lemire) and [imneme](https://github.com/imneme) work into convenient form and wrote some docs. Changes that I introduced, are very trivial. MIT licensed. API and ABI should be stable |

### building tests and examples
In root project dir run:
```
meson setup build/
meson compile -C build/
```
executables will be outputed into build/ dir

For quick check you can use `meson test -C build` which will run some tests
and examples. NOTE: output is way less verbose than when running them by hand
