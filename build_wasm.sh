#!/bin/sh
# Build OpenSkyscraper for WebAssembly into docs/.
#
# Nothing needs downloading: SDL 1.2 and OpenAL come from emscripten itself,
# and the three libraries the engine wanted that have no Emscripten port are in
# compat/ - libmspack vendored, DevIL and libsigc++ replaced.
#
# Usage:  EMSDK=/path/to/emsdk ./build_wasm.sh
set -e
ROOT=$(cd "$(dirname "$0")" && pwd)
JOBS=${JOBS:-3}

EMSDK=${EMSDK:-/c/prog/emsdk/emsdk}
EMDIR="$EMSDK/upstream/emscripten"
[ -f "$EMDIR/emcc" ] || { echo "emcc not found under $EMSDK"; exit 1; }
export EMSDK EM_CONFIG="${EM_CONFIG:-$EMSDK/.emscripten}"
export PATH="$EMDIR:$PATH"

emcmake cmake -S "$ROOT" -B "$ROOT/build" -G Ninja -DCMAKE_BUILD_TYPE=Release

case "$(uname -s)" in
    MINGW*|MSYS*|CYGWIN*)
        # keep the machine usable; children inherit the priority class
        powershell -NoProfile -Command "
            \$p = Start-Process -FilePath 'cmake' \
                -ArgumentList '--build','$ROOT/build','--parallel','$JOBS' \
                -PassThru -NoNewWindow
            try { \$p.PriorityClass = 'BelowNormal' } catch { }
            \$p.WaitForExit()
            exit \$p.ExitCode"
        ;;
    *)
        nice -n 10 cmake --build "$ROOT/build" --parallel "$JOBS"
        ;;
esac

# SINGLE_FILE puts the wasm, the glue and the embedded resources all inside
# the one HTML, so there is no second file that could come from another
# build - and a page served from cache is simply the previous build, whole.
cp "$ROOT/docs/openskyscraper.html" "$ROOT/docs/index.html"
ls -la "$ROOT/docs"
echo "done - serve docs/ over http and pick your own SIMTOWER.EXE"
