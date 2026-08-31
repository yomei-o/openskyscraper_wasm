cmake_minimum_required(VERSION 3.13)
project(openskyscraper_wasm CXX C)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT EMSCRIPTEN)
    message(FATAL_ERROR "This build targets Emscripten; configure with emcmake.")
endif()

# The engine, vendored from OpenSkyscraper's master-sdl branch.
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS engine/source/*.cpp)

# The two things the browser cannot provide: fixed-function OpenGL (there is no
# WebGL here) and DevIL (no Emscripten port, and the loader needs palettes).
set(COMPAT_SOURCES
    compat/softgl.cpp
    compat/il.cpp
)

# libmspack, for the KWAJ-compressed SIMTOWER.EX_ that ships on the install CD.
# Only the KWAJ decompressor and what it needs; LGPL-2.1, see compat/mspack.
set(MSPACK_SOURCES
    compat/mspack/kwajd.c
    compat/mspack/system.c
    compat/mspack/lzssd.c
    compat/mspack/mszipd.c
    compat/mspack/crc32.c
)

add_executable(openskyscraper ${ENGINE_SOURCES} ${COMPAT_SOURCES} ${MSPACK_SOURCES})

target_include_directories(openskyscraper PRIVATE
    compat                      # so <IL/il.h>, <sigc++/...> and "softgl.h" resolve here
    compat/mspack               # <mspack.h>
    engine/source
    engine/source/engine/platform
)

# The engine is 2011 C++; its warnings are not this port's business.
target_compile_options(openskyscraper PRIVATE
    -sUSE_SDL=1
    -Wno-everything
)

target_link_options(openskyscraper PRIVATE
    -sUSE_SDL=1
    -sALLOW_MEMORY_GROWTH=1
    -sINITIAL_MEMORY=134217728
    -sEXPORTED_RUNTIME_METHODS=[callMain,ccall,cwrap,FS]
    -sEXPORTED_FUNCTIONS=[_main]
    -sFORCE_FILESYSTEM=1
    -sENVIRONMENT=web
    -lidbfs.js
    --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/web/shell.html
)

set_target_properties(openskyscraper PROPERTIES
    OUTPUT_NAME openskyscraper
    SUFFIX ".html"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs
)
