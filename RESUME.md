# RESUME

Playable at <https://yomei-o.github.io/openskyscraper_wasm/>. Bring your own
`SIMTOWER.EXE` (or `SIMTOWER.EX_`, which the page detects by its KWAJ magic and
decompresses).

## Build

```sh
cmake --build build --parallel 3     # writes docs/openskyscraper.html
cp docs/openskyscraper.html docs/index.html
```

Emscripten only; `compat/` replaces SDL's OpenGL, DevIL, libsigc++ and libmspack
so a checkout needs nothing but emsdk. `-sSINGLE_FILE=1`, so a stale page can
never pair a cached `.js` with a fresh `.wasm`.

## Two kinds of problem, kept apart

Almost everything reported as broken turned out to be one of these, and telling
them apart first saved a lot of guessing.

**Upstream never implemented it.** OpenSkyscraper stopped around 2010 with the
simulation working and the layer above it unwired. `engine/TODO.md` says so in
the author's own words — the tools window, the funds and population values and
the date display are all on that list. Every fix below in this class was giving
an existing feature a caller, not writing a feature.

**The WebAssembly port wired it wrong.** Emscripten-specific, and mine:

* `SDL_InitSubSystem` is literally `(flags) => 0` under Emscripten. `SDL_Init`
  is what fills `SDL.DOMEventToSDLEvent`, so without it every event arrived with
  type 0 and nothing in the responder chain could match it.
* Drawing at a fixed 1280x768 and scaling the canvas destroyed the bitmap text
  and skewed every click, because pointer coordinates go through
  `canvas.width / rect.width`. The engine now sizes its surface from
  `window.innerWidth/innerHeight` and the element matches exactly.
* `keyboardListeningElement: canvasEl` made the keyboard unreachable: a canvas
  cannot take focus without a `tabindex`, and Emscripten's `preventDefault` on
  mousedown stops a click from focusing it even then. Listen on the document.
* `CruiseControl::frameStart` busy-waits through `SDL_Delay`, which Emscripten
  implements as a spin on the main thread — it froze the tab and the reload
  button. Compiled out under `__EMSCRIPTEN__`.
* `SDL_Flip` is a no-op; pixels reach the canvas in `SDL_UnlockSurface`, and
  `SDL_LockSurface` is what allocates `surface->pixels` in the first place.
* Channel shifts are hardcoded 0/8/16/24, not taken from `surface->format`:
  Emscripten's unlock copies the surface 32 bits at a time into an ImageData
  that is then read as R,G,B,A bytes.

## Fixed since the last handoff

Upstream gaps filled: the bulldozer, finger and inspector tools (the classes did
not exist; `Tool`, `setTool`, `getItems(recti)` and `removeItem` all did);
`Button::pressed` art for them; window dragging (`MouseDragEvent` was defined
and never constructed); the funds and population display, which needed a text
renderer because the engine had none and SimTower drew its numbers with GDI;
`TowerEnvironment::updatePopulation`, which was an empty TODO; the message bar,
fed by build failures that were being written to the console and thrown away.

Elevators: `Item::make` forced every one to 110 floors with a hardcoded
`rect.size.y += 109`, after the 4x1 rect had already passed validation — so the
rule that a transport item needs a floor on every level it passes never applied
to elevators at all. Height now comes from a vertical drag, capped at 30 floors
by a new `maxSpan` descriptor field, and an existing shaft can be extended:
only the added stretches are validated, and the item is moved rather than
rebuilt so it keeps its cars and passengers.

Also: an `Audio::update` iterator bug that erased an element and then
decremented the invalidated iterator, which hung the tab a few seconds in every
time the first sound effect finished; and a debug mode, `?debug` on the URL or
`--debug` on the command line, for unlimited funds and the top rating from the
start.

## Open

1. **The elevator floor numbers do not appear, and a diagnostic is deployed
   waiting on one console line.** `ElevatorItem::drawBackground` prints eight
   bounded lines of the form
   `elevator floors: rect [...] dirty [...] visible [...] -> floors 3..7;
   textures bg [352, 36] ls [160, 36] ms [192, 36]`.
   `ls [0, 0]` means the digit texture never finalised, and `13.0 / ls->size.x`
   divides by zero; `floors 5..4` means the per-floor loop never runs. The whole
   chain reads correctly statically — the six floordigit resources are plain
   uncompressed 8bpp BMPs, the palette survives the loader, indexed pixels
   convert with alpha 255 — so the answer is in the run-time values. **Remove the
   logging once it is known.**
2. **Nine item types exist only as enum names**: cinema, parking ramp, parking
   space, recycling centre, metro, cathedral, security, medical centre, condo.
   No descriptor, no class, so `descriptorForItemType` returns null and the
   toolbox skips them — which is why raising the rating adds nothing. This
   matches `engine/TODO.md`'s "Items to be created".
3. **`minRating` is declared and never read anywhere.** Rating does not gate
   construction at all. Implementing the gate is small; the item classes above
   are the real work.
4. **Elevator stop floors are not configurable.** `isFloorActive(int floor)` is
   a virtual hook that `drawFloor` already consults and that returns `true`
   unconditionally. Making it mean something needs a set of floors on the item,
   `route.cpp` to respect it, and a settings window.
5. **Elevator construction is effectively free.** `constructItem` computes cost
   from `additionalFacilityCellsRequired`, which counts floors rather than the
   shaft; upstream's own `//TODO: make this work for transport items` is on the
   line above.

## Kept deliberately

The counter guards in `elevator.cpp`, `hotel.cpp`, `office.cpp` and `car.cpp`
are cheap insurance and stay. An unbounded loop in a browser takes the tab and
the reload button with it, which is a much worse failure than a log line saying
a loop gave up.

## Habits that paid off here

* Measure before theorising. The floor-number problem has had four plausible
  explanations and all four were wrong; the logging will settle it in one round
  trip. Earlier, phase tracing found the audio hang after three failed guesses.
* Proof-read generated data offline. The bitmap font was drawn to a PNG from the
  C table before it ever reached a browser, which is how the nine-pixel `$` and
  `1` problem was caught.
* Write patch scripts to files rather than through shell heredocs. Backslashes
  and quotes get eaten, repeatedly.
