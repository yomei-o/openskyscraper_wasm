#!/bin/sh
# Check the DevIL replacement against real SimTower data.
#
# The palette path is the risky part of this port: the loader reads and rewrites
# an 8-bit image's palette to make the day/night/rain skies, so the decoder has
# to keep palettes rather than expand them.  This pulls real bitmaps out of a
# SIMTOWER.EXE, decodes them through compat/il.cpp, and compares the result
# against Pillow byte for byte.
#
# Usage:  ./test/run_tests.sh /path/to/SIMTOWER.EXE
set -e
ROOT=$(cd "$(dirname "$0")/.." && pwd)
EXE=${1:-$ROOT/SIMTOWER.EXE}
OUT=$ROOT/build/bmp_out
CXX=${CXX:-g++}

[ -f "$EXE" ] || { echo "no SIMTOWER.EXE at $EXE"; exit 2; }

mkdir -p "$OUT"
python "$ROOT/tools/extract_bmp.py" "$EXE" "$OUT" "${COUNT:-6}"

"$CXX" -std=c++14 -O2 -I"$ROOT/compat" -o "$OUT/il_test" \
    "$ROOT/test/il_test.cpp" "$ROOT/compat/il.cpp"

for bmp in "$OUT"/*.bmp; do
    "$OUT/il_test" "$bmp" "${bmp%.bmp}.ppm"
done

python - "$OUT" <<'PY'
import glob, os, sys
from PIL import Image
out = sys.argv[1]
bad = 0
for bmp in sorted(glob.glob(os.path.join(out, '*.bmp'))):
    ppm = bmp[:-4] + '.ppm'
    ref, mine = Image.open(bmp).convert('RGB'), Image.open(ppm).convert('RGB')
    if ref.size != mine.size:
        print('FAIL %s size %s vs %s' % (bmp, ref.size, mine.size)); bad += 1; continue
    diff = sum(1 for a, b in zip(ref.tobytes(), mine.tobytes()) if a != b)
    print('%-14s %-11s %d differing bytes' % (os.path.basename(bmp), str(ref.size), diff))
    bad += 1 if diff else 0
print('FAILED' if bad else 'all images match Pillow exactly')
raise SystemExit(1 if bad else 0)
PY
