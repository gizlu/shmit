# shmit
shit that don't deserve separate repos

## scripts
| File           | Description |
|----------------|-------------|
| [**scripts/argvtoc**](scripts/argvtoc) | shellscript for printing argv as c array. Useful for debugging and calling exec\*() by hand. **Public domain**|
| [**scripts/screenshot**](scripts/screenshot) | shellscript for doing screenshots on x11 and saving them in clipoard and disk. **Public domain**|
| [**scripts/marktable**](scripts/marktable) | shellscript for converting tabular data into markdown syntax. **Public domain** |
| [**scripts/gmintbl**](scripts/gmintbl) | shellscript for minifing google benchmark output. **Public domain** |

## shlag - single/two file libs libs that don't deserve separate repos[^1]
| File           | Description |
|----------------|-------------|
|[**shitest.h**](shlag/shitest.h) | Minimal unittesting lib. Written due to my discontent with fullblown frameworks like gtest. **UNSTABLE** |
|[**shlag_btt**](shlag/shlag_btt.h) |  Binary to text encodings such as base64 with support for inplace enc/dec. **UNSTABLE** |
|[**shlag_pcg**](shlag/shlag_pcg.h) | 32 bit [pcg prng](https://www.pcg-random.org/) wrapped in single header lib along [fast, unbiased algo](https://lemire.me/blog/2016/06/30/fast-random-shuffling/) for randrange(). **STABLE, MIT Licensed** |

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
