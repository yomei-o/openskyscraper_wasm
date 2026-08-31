# openskyscraper_wasm

**OpenSkyscraper** — the open reimplementation of Maxis' *SimTower* — ported to
WebAssembly, with **no WebGL**: the fixed-function OpenGL it draws through is
rasterised in software.

### [▶ Live demo](https://yomei-o.github.io/openskyscraper_wasm/) — bring your own `SIMTOWER.EXE`

## What you need to run it

One file: **`SIMTOWER.EXE` from the 16-bit Windows 3.1 release** of SimTower, or
the `SIMTOWER.EX_` off the install disc. Nothing else, and nothing is uploaded —
the page reads the file in your browser and writes it into the WebAssembly
filesystem.

The game's graphics, sounds and palettes live inside that executable's NE
resource table; upstream OpenSkyscraper reads them out at startup and this port
keeps that. It is not shipped here, and it is not free content.

A Mac copy will not work, and neither will a 32-bit repackaging: the loader
parses the *New Executable* format specifically. If your disc has
`SIMTOWER.EX_`, hand that over instead — it is KWAJ-compressed and gets
decompressed in the browser.

## What the port actually changes

Upstream builds against SDL 1.2, OpenGL, OpenAL, DevIL and libsigc++. Three of
those have no Emscripten story, so `compat/` replaces them:

| upstream | here | why |
|---|---|---|
| OpenGL (fixed function) | `compat/softgl.*` — software rasteriser | **no WebGL on the target machines.** Emscripten's own `LEGACY_GL_EMULATION` calls itself "incomplete", and the whole renderer is 434 lines drawing axis-aligned textured quads, so rasterising them is both safer and simpler |
| DevIL | `compat/il.*` — BMP decoder that keeps palettes | no Emscripten port, and stb_image is not enough: the loader **rewrites an 8-bit image's palette** to derive the day, overcast, rain and night skies, then clones it |
| libsigc++ | `compat/sigc++/sigc++.h` | the entire dependency was one `sigc::signal<void>` that is emitted and never connected to |
| libmspack | vendored in `compat/mspack/` | for `SIMTOWER.EX_`; LGPL-2.1, five source files of it |
| SDL 1.2, OpenAL | emscripten's own | `-sUSE_SDL=1`, `-lopenal` |

Two engine changes were needed beyond that, both small:

* **The run loop.** `Application::run()` blocked in a `while` loop; the browser
  owns the frame clock, so the body moved into `runLoopIteration()` and the
  Emscripten build drives it from `emscripten_set_main_loop`. Desktop builds
  take the same path they always did.
* **The video mode.** `SDL_OPENGL` becomes a plain `SDL_SWSURFACE`, and
  `softgl` is pointed at the surface's pixels. `swapBuffers` is `SDL_Flip`.

Everything else in `engine/` is upstream's `master-sdl` branch, untouched.

### Why `master-sdl` and not `master`

Upstream's `master` is the 2013–2019 "restart", and it is the more developed
game — but it is built on **SFML 2** (no official Emscripten support) and
**libRocket** (abandoned), and those reach 49 and 19 of its 112 files
respectively. Porting it means rewriting half the codebase. `master-sdl` needs
SDL 1.2, OpenGL, OpenAL and DevIL, and emscripten supplies the first three
outright.

## Build

```sh
EMSDK=/path/to/emsdk ./build_wasm.sh
python -m http.server -d docs 8000     # then visit http://localhost:8000
```

**Nothing is downloaded.** Every dependency that is not part of emsdk is in this
repository. On Windows the compiler runs at BelowNormal priority so the machine
stays usable.

## Tests

The palette path is where this port could quietly go wrong, so it is checked
against real data:

```sh
./test/run_tests.sh /path/to/SIMTOWER.EXE
```

`tools/extract_bmp.py` pulls bitmap resources straight out of the executable's
NE resource table and wraps them as BMP files the way the game does at runtime
(a 14-byte header with the data offset fixed at `0x436`). Each is then decoded
through `compat/il.cpp` and compared against Pillow, byte for byte.

```
res_0080.bmp   (236, 198)  0 differing bytes
res_0100.bmp   (640, 480)  0 differing bytes
res_0101.bmp   (334, 270)  0 differing bytes
res_012C.bmp   (256, 128)  0 differing bytes
res_012D.bmp   (256, 128)  0 differing bytes
res_012E.bmp   (256, 128)  0 differing bytes
all images match Pillow exactly
```

## State of the game

Be realistic about what this is. OpenSkyscraper's own README calls itself
experimental, [osgameclones](https://osgameclones.com/simtower/) lists the
project as *halted* and *unplayable*, and `master-sdl`'s last commit is from
2011. This port makes it build and run in a browser without WebGL; it does not
make it a finished game.

## Licensing

* **OpenSkyscraper is GPL-2.0**, and so is this repository's own code.
* **libmspack** in `compat/mspack/` is LGPL-2.1, © Stuart Caie.
* **SimTower is not included and is not free.** © 1994 Maxis / OPeNBooK,
  created by Yoot Saito. You supply your own copy; nothing here distributes it
  or helps you obtain it.

## Credit

* [OpenSkyscraper](https://github.com/fabianschuiki/OpenSkyscraper) by Fabian
  Schuiki and contributors — the engine and the SimTower resource loader.
* [libmspack](https://github.com/kyz/libmspack) by Stuart Caie — KWAJ
  decompression.
* *SimTower: The Vertical Empire*, by Yoot Saito, published by Maxis in 1994.
