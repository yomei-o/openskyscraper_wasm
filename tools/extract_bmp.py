"""Pull bitmap resources out of the real SIMTOWER.EXE and wrap them as BMP files.

The game does exactly this at runtime: NE resources of type 0x8002 hold a
BITMAPINFOHEADER with no file header, so a 14-byte one is prepended with the
data offset fixed at 0x436 (14 + 40 + 256*4).  Writing them out here lets the
DevIL replacement be checked against real input rather than a synthetic file.
"""
import struct
import sys
import os

path = sys.argv[1] if len(sys.argv) > 1 else 'C:/prog/claude/SIMTOWER.EXE'
outdir = sys.argv[2] if len(sys.argv) > 2 else 'bmp_out'
want = int(sys.argv[3]) if len(sys.argv) > 3 else 6

d = open(path, 'rb').read()
ne = struct.unpack_from('<I', d, 0x3c)[0]
assert d[ne:ne + 2] == b'NE', 'not a 16-bit NE executable'

rt = ne + struct.unpack_from('<H', d, ne + 0x24)[0]
shift = struct.unpack_from('<H', d, ne + 0x32)[0]

os.makedirs(outdir, exist_ok=True)
pos = rt + 2
written = 0
while written < want:
    type_id = struct.unpack_from('<H', d, pos)[0]
    if type_id == 0:
        break
    count = struct.unpack_from('<H', d, pos + 2)[0]
    entry = pos + 8
    for _ in range(count):
        offset = struct.unpack_from('<H', d, entry)[0] << shift
        length = struct.unpack_from('<H', d, entry + 2)[0] << shift
        res_id = struct.unpack_from('<H', d, entry + 6)[0] & 0x7fff
        if type_id == 0x8002 and written < want:
            body = d[offset:offset + length]
            header = struct.pack('<HIII', 0x4D42, 0, 0, 0x436)
            name = os.path.join(outdir, 'res_%04X.bmp' % res_id)
            open(name, 'wb').write(header + body)
            print('%s  %d bytes' % (name, len(header) + len(body)))
            written += 1
        entry += 12
    pos = entry
