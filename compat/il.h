// The slice of DevIL that OpenSkyscraper actually uses.
//
// DevIL has no Emscripten port, and swapping in stb_image is not enough: the
// SimTower loader reads and *rewrites* the palette of an 8-bit image to make the
// day, overcast, rain and night variants of the sky, then clones the image each
// time.  So palettes have to survive loading rather than being expanded away,
// which rules out every small decoder that hands back RGB.
//
// This keeps DevIL's model — a bound "current image", an id per image — because
// that is what the thirty call sites expect; nothing in the engine changes.
//
// Palette entries are four bytes in BMP order: B, G, R, A.  The engine relies
// on that: applyReplacementPalette() writes v[3-n] from an AARRGGBB source.
#pragma once

#include <cstddef>

typedef unsigned int ILuint;
typedef int ILint;
typedef unsigned int ILenum;
typedef unsigned char ILubyte;
typedef unsigned char ILboolean;
typedef size_t ILsizei;

#define IL_FALSE 0
#define IL_TRUE 1

// formats
#define IL_COLOUR_INDEX 0x1900
#define IL_COLOR_INDEX 0x1900
#define IL_RGB 0x1907
#define IL_RGBA 0x1908
#define IL_BGR 0x80E0
#define IL_BGRA 0x80E1
#define IL_LUMINANCE 0x1909

// types
#define IL_UNSIGNED_BYTE 0x1401

// image kinds
#define IL_BMP 0x0420
#define IL_PNG 0x042A
#define IL_TYPE_UNKNOWN 0x0000

// ilGetInteger keys
#define IL_IMAGE_WIDTH 0x0DE4
#define IL_IMAGE_HEIGHT 0x0DE5
#define IL_IMAGE_DEPTH 0x0DE6
#define IL_IMAGE_BYTES_PER_PIXEL 0x0DE8
#define IL_IMAGE_BPP 0x0DE8
#define IL_IMAGE_FORMAT 0x0DEA
#define IL_IMAGE_TYPE 0x0DEB
#define IL_IMAGE_SIZE_OF_DATA 0x0DEB + 1
#define IL_PALETTE_TYPE 0x0DEC
#define IL_PALETTE_NUM_COLS 0x0DED

// ilEnable / ilOriginFunc
#define IL_ORIGIN_SET 0x0600
#define IL_ORIGIN_LOWER_LEFT 0x0601
#define IL_ORIGIN_UPPER_LEFT 0x0602
#define IL_FILE_OVERWRITE 0x0620

#ifdef __cplusplus
extern "C" {
#endif

void ilInit(void);
ILboolean ilEnable(ILenum mode);
ILboolean ilDisable(ILenum mode);
void ilOriginFunc(ILenum mode);

ILuint ilGenImage(void);
void ilGenImages(ILsizei num, ILuint* images);
void ilBindImage(ILuint image);
void ilDeleteImage(ILuint image);
void ilDeleteImages(ILsizei num, const ILuint* images);
ILuint ilCloneCurImage(void);

ILboolean ilLoadL(ILenum type, const void* data, ILuint length);
ILboolean ilLoadImage(const char* filename);
ILboolean ilSaveImage(const char* filename);

ILint ilGetInteger(ILenum mode);
ILubyte* ilGetData(void);
ILubyte* ilGetPalette(void);

ILboolean ilConvertImage(ILenum destFormat, ILenum destType);
ILboolean ilTexImage(ILuint width, ILuint height, ILuint depth, ILubyte bpp,
                     ILenum format, ILenum type, void* data);
ILboolean ilBlit(ILuint source, ILint destX, ILint destY, ILint destZ,
                 ILuint srcX, ILuint srcY, ILuint srcZ, ILuint width,
                 ILuint height, ILuint depth);
ILboolean ilCopyPixels(ILuint xOff, ILuint yOff, ILuint zOff, ILuint width,
                       ILuint height, ILuint depth, ILenum format, ILenum type,
                       void* data);
ILboolean ilSetPixels(ILint xOff, ILint yOff, ILint zOff, ILuint width,
                      ILuint height, ILuint depth, ILenum format, ILenum type,
                      void* data);

#ifdef __cplusplus
}
#endif
