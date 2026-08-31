// Decode real SimTower bitmaps through the DevIL replacement and dump PPMs,
// so the result can be diffed against a known-good decoder.
#include <cstdio>
#include <cstdlib>
#include <vector>
#include "il.h"

int main(int argc, char** argv) {
    if (argc < 3) { std::fprintf(stderr, "usage: %s in.bmp out.ppm\n", argv[0]); return 2; }
    std::FILE* f = std::fopen(argv[1], "rb");
    if (!f) { std::fprintf(stderr, "cannot open %s\n", argv[1]); return 1; }
    std::fseek(f, 0, SEEK_END); long n = std::ftell(f); std::fseek(f, 0, SEEK_SET);
    std::vector<unsigned char> buf(n);
    if (std::fread(buf.data(), 1, n, f) != (size_t)n) { std::fclose(f); return 1; }
    std::fclose(f);

    ilInit();
    ILuint img = ilGenImage();
    ilBindImage(img);
    if (!ilLoadL(IL_BMP, buf.data(), (ILuint)buf.size())) {
        std::fprintf(stderr, "ilLoadL failed\n"); return 1;
    }
    const int w = ilGetInteger(IL_IMAGE_WIDTH);
    const int h = ilGetInteger(IL_IMAGE_HEIGHT);
    std::printf("%dx%d format=0x%x bpp=%d palette=%s\n", w, h,
                ilGetInteger(IL_IMAGE_FORMAT), ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL),
                ilGetPalette() ? "yes" : "no");
    if (!ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE)) { std::fprintf(stderr, "convert failed\n"); return 1; }

    const unsigned char* d = ilGetData();
    std::FILE* o = std::fopen(argv[2], "wb");
    std::fprintf(o, "P6\n%d %d\n255\n", w, h);
    // rows are bottom-up, as BMP stores them; flip for a top-down PPM
    for (int y = h - 1; y >= 0; --y) std::fwrite(d + (size_t)y * w * 3, 1, (size_t)w * 3, o);
    std::fclose(o);
    return 0;
}
