# RESUME

## Done

* The whole engine compiles and links for Emscripten. `docs/` holds a working
  bundle: 447 KB of wasm, 146 KB of glue, no WebGL involved.
* `compat/softgl.*` implements the twenty-five fixed-function GL entry points
  the engine uses, in software: axis-aligned textured quads with colour
  modulation and alpha blending, plus lines for the clock hands and selection
  frames, under `glOrtho(0, w, 0, h, -1, 1)`.
* `compat/il.*` replaces DevIL. Verified against real SimTower bitmaps: six
  images, 1.6 MB of pixels, zero bytes differing from Pillow's decode
  (`./test/run_tests.sh`).
* `compat/sigc++` and `compat/mspack` cover the other two dependencies, so a
  checkout builds with nothing but emsdk.
* Page shell asks for the user's own `SIMTOWER.EXE` (or `.EX_`, which it
  detects by the KWAJ magic) and writes it into the virtual filesystem before
  calling `main`.

## Not yet verified

**Nobody has watched this run.** It builds, links, and the resource decoding is
proven against real data, but the machine this was built on has no WebGL *and*
no way to open a browser, so the first actual frame is still unseen. Expect the
first session to be about:

1. **Does the software rasteriser draw the right thing?** The quad path assumes
   every `GL_QUADS` batch is an axis-aligned rectangle whose vertices 0 and 2
   are opposite corners. That holds for every call site read so far, but a
   wrongly-oriented sprite or an inverted texture rect would show up
   immediately.
2. **Texture coordinates are in texels, not 0..1** — the engine used
   `GL_TEXTURE_RECTANGLE_ARB`. `softgl` follows that; if sprites come out
   sampling a single pixel, that assumption broke somewhere.
3. **SDL 1.2 surface format.** `softgl::set_target` takes the channel shifts
   from the surface rather than assuming RGBA; if colours come out swapped,
   that is the place to look.
4. **Audio.** OpenAL is linked but untested; the engine may want a device the
   browser only grants after a user gesture.

## Next, roughly in order

1. Open it, load a `SIMTOWER.EXE`, and fix whatever the first frame reveals.
2. Performance: the rasteriser is a straightforward scanline blitter with a
   `double` per channel. If a full tower is slow, the inner loop converting to
   fixed-point integers is the obvious first move.
3. `master`'s game logic is further along than `master-sdl`'s. Porting features
   across, rather than the whole SFML/libRocket branch, may be worth more than
   fighting those two dependencies.

## Working notes

* **Do not name a header `emscripten.h`.** `engine/source/engine/platform/` is on
  the include path, so a file with that name there shadows the SDK's own and
  `emscripten_set_main_loop` goes undeclared with no hint as to why. It is
  `emscripten_platform.h` for that reason.
* The bitmap resources are 8-bit paletted BMPs whose file header the game
  reconstructs with the data offset hard-coded to `0x436` — 14 + 40 + 256*4.
  `tools/extract_bmp.py` does the same thing so tests see identical input.
* Palette entries are B, G, R, A in that order, because BMP stores them that way
  and `applyReplacementPalette()` writes `v[3-n]` from an AARRGGBB source. Get
  this backwards and the sky comes out orange.
