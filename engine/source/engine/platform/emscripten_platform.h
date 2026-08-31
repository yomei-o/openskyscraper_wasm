// Emscripten platform header.
//
// OpenAL is provided by emscripten itself (-lopenal).  OpenGL is not: these
// browsers have no WebGL, so the fixed-function subset the engine draws through
// comes from the software rasteriser in compat/softgl.h instead.

//OpenAL
#include <AL/al.h>
#include <AL/alc.h>

//OpenGL, in software
#include "softgl.h"

// SDL 1.2 had this for SDL_PeepEvents masks; emscripten's SDL headers drop it.
#ifndef SDL_EVENTMASK
#define SDL_EVENTMASK(X) (1 << (X))
#endif
