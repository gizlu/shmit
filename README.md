# shmit
shit that don't deserve separate repos

## scripts
| File           | Description |
|----------------|-------------|
| [**scripts/argv_to_c.sh**](scripts/argv_to_c.sh) | dead simple shell script for printing argv as c array. Useful for debugging and calling exec\*() by hand. Inspired by [ImgToC.c](https://github.com/DanielGibson/Snippets/blob/master/ImgToC.c). |
| [**scripts/screenshot.sh**](scripts/screenshot.sh) | dead simple shell script for doing screenshots of selected area/current window/all displays and saving them in clipoard and disk. Depends on maim and xclip |

## shlag - single or two file libs libs that don't deserve separate repos[^1]
| File           | Description |
|----------------|-------------|
|[**shitest.h**](shlag/shitest.h) | Minimalistic single header testing library written due to my discontent with fullblown frameworks like gtest. TODO: stabilize API/ABI, maybe write script converting output to TAP format |
|[**shlag_btt**](shlag/shlag_btt.h) | Binary to text encodings such as base64. It supports in-place enc/dec, so you don't have to allocate separate buffer. Currently only base64 is implemented (TODO: other formats like b32, examples and stabilizing API/ABI - it is not stable yet). |
|[**shlag_pcg**](shlag/shlag_pcg.h) | 32 bit [pcg pseudorandom generator](https://www.pcg-random.org/) wrapped in single header C/C++ library along [fast, unbiased algorithm](https://lemire.me/blog/2016/06/30/fast-random-shuffling/) for reducting numbers to specified range. I just combined [lemire](https://github.com/lemire) and [imneme](https://github.com/imneme) work into convenient form and wrote some docs. Changes that I introduced, are very trivial. MIT licensed. API and ABI should be stable |


There are examples in `shlag/examples/` and tests in `shlag/tests/`

<details>
<summary>To single header or not to single header?</summary>

originaly shlag were "single header libs that don't deserve separate repos" (name was derived from that), 
but after writing few libs I came to conslusion, that single header model is just needlessly awkward:
- There is almost no difference for user, whether there is one or two files
- It is harder to write code this way, e.g. you have to namespace private stuff, you have to ensure that C compiles as C++, etc. 
- It is confusing for anyone who doesn't know this model

New shlag libs are probably going to have separate header and implementation files
</details>

### building tests and examples
In root project dir run:
```
meson setup build/
meson compile -C build/
```
executables will be outputed into build/ dir

For quick check you can use `meson test -C build` which will run some tests
and examples. NOTE: output is way less verbose than when running them by hand
