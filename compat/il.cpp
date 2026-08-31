#include "il.h"

#include <cstdio>
#include <cstring>
#include <map>
#include <vector>

namespace {

struct Image {
    ILuint width = 0;
    ILuint height = 0;
    ILenum format = IL_RGB;       // IL_COLOUR_INDEX, IL_RGB or IL_RGBA
    int bpp = 3;                  // bytes per pixel of `data`
    std::vector<ILubyte> data;    // rows bottom-up, as BMP and IL_ORIGIN_LOWER_LEFT agree
    std::vector<ILubyte> palette; // 256 entries, B G R A
};

std::map<ILuint, Image> g_images;
ILuint g_next = 1;
ILuint g_bound = 0;

Image* current() {
    if (!g_bound) return nullptr;
    auto it = g_images.find(g_bound);
    return it == g_images.end() ? nullptr : &it->second;
}

inline ILuint rd16(const ILubyte* p) {
    return static_cast<ILuint>(p[0]) | (static_cast<ILuint>(p[1]) << 8);
}

inline ILuint rd32(const ILubyte* p) {
    return static_cast<ILuint>(p[0]) | (static_cast<ILuint>(p[1]) << 8) |
           (static_cast<ILuint>(p[2]) << 16) | (static_cast<ILuint>(p[3]) << 24);
}

// Windows BMP, the uncompressed 1/4/8/24/32-bit forms SimTower's resources use.
// The palette is kept rather than expanded — that is the whole reason this file
// exists.
bool load_bmp(const ILubyte* buf, size_t len, Image* out) {
    if (len < 54 || buf[0] != 'B' || buf[1] != 'M') return false;
    const ILuint data_offset = rd32(buf + 10);
    const ILuint header_size = rd32(buf + 14);
    if (header_size < 12) return false;

    ILuint width, height, bits, compression = 0, palette_count = 0;
    if (header_size == 12) {                    // BITMAPCOREHEADER
        width = rd16(buf + 18);
        height = rd16(buf + 20);
        bits = rd16(buf + 24);
    } else {                                    // BITMAPINFOHEADER and later
        width = rd32(buf + 18);
        height = rd32(buf + 22);
        bits = rd16(buf + 28);
        compression = rd32(buf + 30);
        palette_count = rd32(buf + 46);
    }
    if (compression != 0) return false;         // no RLE in these resources
    if (width == 0 || height == 0) return false;

    out->width = width;
    out->height = height;

    const size_t src_stride = ((static_cast<size_t>(width) * bits + 31) / 32) * 4;
    if (data_offset + src_stride * height > len) return false;

    if (bits <= 8) {
        if (palette_count == 0) palette_count = 1u << bits;
        if (palette_count > 256) palette_count = 256;
        const size_t pal_off = 14 + header_size;
        const size_t entry = (header_size == 12) ? 3 : 4;
        out->palette.assign(256 * 4, 0);
        for (ILuint i = 0; i < palette_count; ++i) {
            const size_t o = pal_off + i * entry;
            if (o + 2 >= len) break;
            out->palette[i * 4 + 0] = buf[o + 0];   // B
            out->palette[i * 4 + 1] = buf[o + 1];   // G
            out->palette[i * 4 + 2] = buf[o + 2];   // R
            out->palette[i * 4 + 3] = 0xff;         // A
        }
        out->format = IL_COLOUR_INDEX;
        out->bpp = 1;
        out->data.assign(static_cast<size_t>(width) * height, 0);
        for (ILuint y = 0; y < height; ++y) {
            const ILubyte* src = buf + data_offset + src_stride * y;
            ILubyte* dst = &out->data[static_cast<size_t>(y) * width];
            for (ILuint x = 0; x < width; ++x) {
                if (bits == 8) dst[x] = src[x];
                else if (bits == 4) dst[x] = (x & 1) ? (src[x / 2] & 0x0f)
                                                     : (src[x / 2] >> 4);
                else dst[x] = (src[x / 8] >> (7 - (x & 7))) & 1;
            }
        }
        return true;
    }

    const int comps = (bits == 32) ? 4 : 3;
    out->format = (comps == 4) ? IL_RGBA : IL_RGB;
    out->bpp = comps;
    out->data.assign(static_cast<size_t>(width) * height * comps, 0);
    for (ILuint y = 0; y < height; ++y) {
        const ILubyte* src = buf + data_offset + src_stride * y;
        ILubyte* dst = &out->data[static_cast<size_t>(y) * width * comps];
        for (ILuint x = 0; x < width; ++x) {
            dst[x * comps + 0] = src[x * (bits / 8) + 2];   // BMP stores BGR
            dst[x * comps + 1] = src[x * (bits / 8) + 1];
            dst[x * comps + 2] = src[x * (bits / 8) + 0];
            if (comps == 4) dst[x * comps + 3] = src[x * 4 + 3];
        }
    }
    return true;
}

}  // namespace

extern "C" {

void ilInit(void) {}
ILboolean ilEnable(ILenum) { return IL_TRUE; }
ILboolean ilDisable(ILenum) { return IL_TRUE; }
void ilOriginFunc(ILenum) {}

ILuint ilGenImage(void) {
    const ILuint id = g_next++;
    g_images[id] = Image();
    return id;
}

void ilGenImages(ILsizei num, ILuint* images) {
    for (ILsizei i = 0; i < num; ++i) images[i] = ilGenImage();
}

void ilBindImage(ILuint image) { g_bound = image; }

void ilDeleteImage(ILuint image) {
    if (g_bound == image) g_bound = 0;
    g_images.erase(image);
}

void ilDeleteImages(ILsizei num, const ILuint* images) {
    for (ILsizei i = 0; i < num; ++i) ilDeleteImage(images[i]);
}

ILuint ilCloneCurImage(void) {
    const Image* src = current();
    if (!src) return 0;
    const ILuint id = g_next++;
    g_images[id] = *src;          // data and palette both, by value
    return id;
}

ILboolean ilLoadL(ILenum type, const void* data, ILuint length) {
    Image* img = current();
    if (!img || !data) return IL_FALSE;
    if (type != IL_BMP && type != IL_TYPE_UNKNOWN) return IL_FALSE;
    Image loaded;
    if (!load_bmp(static_cast<const ILubyte*>(data), length, &loaded))
        return IL_FALSE;
    *img = loaded;
    return IL_TRUE;
}

ILboolean ilLoadImage(const char* filename) {
    Image* img = current();
    if (!img || !filename) return IL_FALSE;
    std::FILE* f = std::fopen(filename, "rb");
    if (!f) return IL_FALSE;
    std::fseek(f, 0, SEEK_END);
    const long n = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    std::vector<ILubyte> buf(n > 0 ? n : 0);
    if (!buf.empty() && std::fread(buf.data(), 1, buf.size(), f) != buf.size()) {
        std::fclose(f);
        return IL_FALSE;
    }
    std::fclose(f);
    Image loaded;
    if (!load_bmp(buf.data(), buf.size(), &loaded)) return IL_FALSE;
    *img = loaded;
    return IL_TRUE;
}

ILboolean ilSaveImage(const char*) {
    return IL_TRUE;               // only used for debug dumps
}

ILint ilGetInteger(ILenum mode) {
    const Image* img = current();
    if (!img) return 0;
    switch (mode) {
        case IL_IMAGE_WIDTH: return static_cast<ILint>(img->width);
        case IL_IMAGE_HEIGHT: return static_cast<ILint>(img->height);
        case IL_IMAGE_DEPTH: return 1;
        case IL_IMAGE_BYTES_PER_PIXEL: return img->bpp;
        case IL_IMAGE_FORMAT: return static_cast<ILint>(img->format);
        case IL_IMAGE_TYPE: return IL_UNSIGNED_BYTE;
        case IL_IMAGE_SIZE_OF_DATA: return static_cast<ILint>(img->data.size());
        case IL_PALETTE_NUM_COLS: return img->palette.empty() ? 0 : 256;
        default: return 0;
    }
}

ILubyte* ilGetData(void) {
    Image* img = current();
    return (img && !img->data.empty()) ? img->data.data() : nullptr;
}

ILubyte* ilGetPalette(void) {
    Image* img = current();
    return (img && !img->palette.empty()) ? img->palette.data() : nullptr;
}

ILboolean ilConvertImage(ILenum destFormat, ILenum destType) {
    Image* img = current();
    if (!img || destType != IL_UNSIGNED_BYTE) return IL_FALSE;
    if (img->format == destFormat) return IL_TRUE;
    const int comps = (destFormat == IL_RGBA) ? 4
                    : (destFormat == IL_RGB) ? 3 : 0;
    if (comps == 0) return IL_FALSE;

    const size_t count = static_cast<size_t>(img->width) * img->height;
    std::vector<ILubyte> out(count * comps, 0);

    if (img->format == IL_COLOUR_INDEX) {
        if (img->palette.size() < 256 * 4) return IL_FALSE;
        for (size_t i = 0; i < count; ++i) {
            const ILubyte* e = &img->palette[img->data[i] * 4];
            out[i * comps + 0] = e[2];      // palette is B G R A
            out[i * comps + 1] = e[1];
            out[i * comps + 2] = e[0];
            if (comps == 4) out[i * comps + 3] = e[3];
        }
    } else {
        const int src_comps = img->bpp;
        for (size_t i = 0; i < count; ++i) {
            out[i * comps + 0] = img->data[i * src_comps + 0];
            out[i * comps + 1] = img->data[i * src_comps + 1];
            out[i * comps + 2] = img->data[i * src_comps + 2];
            if (comps == 4)
                out[i * comps + 3] =
                    (src_comps == 4) ? img->data[i * src_comps + 3] : 0xff;
        }
    }

    img->data.swap(out);
    img->format = destFormat;
    img->bpp = comps;
    img->palette.clear();
    return IL_TRUE;
}

ILboolean ilTexImage(ILuint width, ILuint height, ILuint, ILubyte bpp,
                     ILenum format, ILenum, void* data) {
    Image* img = current();
    if (!img) return IL_FALSE;
    img->width = width;
    img->height = height;
    img->format = format;
    img->bpp = bpp ? bpp : 3;
    img->palette.clear();
    const size_t n = static_cast<size_t>(width) * height * img->bpp;
    img->data.assign(n, 0);
    if (data) std::memcpy(img->data.data(), data, n);
    return IL_TRUE;
}

ILboolean ilBlit(ILuint source, ILint destX, ILint destY, ILint,
                 ILuint srcX, ILuint srcY, ILuint, ILuint width, ILuint height,
                 ILuint) {
    Image* dst = current();
    auto it = g_images.find(source);
    if (!dst || it == g_images.end()) return IL_FALSE;
    const Image& src = it->second;
    if (src.bpp != dst->bpp) return IL_FALSE;

    for (ILuint y = 0; y < height; ++y) {
        const long sy = static_cast<long>(srcY) + y;
        const long dy = static_cast<long>(destY) + y;
        if (sy < 0 || dy < 0 || sy >= (long)src.height || dy >= (long)dst->height)
            continue;
        for (ILuint x = 0; x < width; ++x) {
            const long sx = static_cast<long>(srcX) + x;
            const long dx = static_cast<long>(destX) + x;
            if (sx < 0 || dx < 0 || sx >= (long)src.width || dx >= (long)dst->width)
                continue;
            std::memcpy(&dst->data[((size_t)dy * dst->width + dx) * dst->bpp],
                        &src.data[((size_t)sy * src.width + sx) * src.bpp],
                        dst->bpp);
        }
    }
    // a blit into a fresh paletted target should inherit the source palette
    if (dst->palette.empty() && !src.palette.empty()) dst->palette = src.palette;
    return IL_TRUE;
}

ILboolean ilCopyPixels(ILuint xOff, ILuint yOff, ILuint, ILuint width,
                       ILuint height, ILuint, ILenum, ILenum, void* data) {
    const Image* img = current();
    if (!img || !data) return IL_FALSE;
    ILubyte* out = static_cast<ILubyte*>(data);
    for (ILuint y = 0; y < height; ++y) {
        for (ILuint x = 0; x < width; ++x) {
            const ILuint sx = xOff + x, sy = yOff + y;
            ILubyte* d = out + ((size_t)y * width + x) * img->bpp;
            if (sx >= img->width || sy >= img->height) {
                std::memset(d, 0, img->bpp);
                continue;
            }
            std::memcpy(d, &img->data[((size_t)sy * img->width + sx) * img->bpp],
                        img->bpp);
        }
    }
    return IL_TRUE;
}

ILboolean ilSetPixels(ILint xOff, ILint yOff, ILint, ILuint width,
                      ILuint height, ILuint, ILenum, ILenum, void* data) {
    Image* img = current();
    if (!img || !data) return IL_FALSE;
    const ILubyte* src = static_cast<const ILubyte*>(data);
    for (ILuint y = 0; y < height; ++y) {
        for (ILuint x = 0; x < width; ++x) {
            const long dx = xOff + (long)x, dy = yOff + (long)y;
            if (dx < 0 || dy < 0 || dx >= (long)img->width ||
                dy >= (long)img->height)
                continue;
            std::memcpy(&img->data[((size_t)dy * img->width + dx) * img->bpp],
                        src + ((size_t)y * width + x) * img->bpp, img->bpp);
        }
    }
    return IL_TRUE;
}

}  // extern "C"
