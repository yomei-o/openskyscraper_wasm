// The slice of fixed-function OpenGL that OpenSkyscraper actually uses,
// implemented in software.
//
// The game draws exactly one kind of thing: axis-aligned, texture-rectangle
// quads with a modulating colour and alpha blending, under an
// `glOrtho(0, w, 0, h, -1, 1)` projection — plus a handful of lines for the
// clock hands and selection frames.  That is a small enough target to rasterise
// on the CPU, which is the point: this build has no WebGL to fall back on, and
// emscripten's own LEGACY_GL_EMULATION describes itself as "incomplete".
//
// Only the twenty-five entry points the engine calls are here.  Anything else
// is deliberately absent so a new call site fails to link rather than silently
// drawing nothing.
#pragma once

#include <cstddef>
#include <cstdint>

// -- types -------------------------------------------------------------------
typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef void GLvoid;
typedef signed char GLbyte;
typedef short GLshort;
typedef int GLint;
typedef int GLsizei;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned int GLuint;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;

// -- constants ---------------------------------------------------------------
#define GL_FALSE 0
#define GL_TRUE 1

#define GL_POINTS 0x0000
#define GL_LINES 0x0001
#define GL_LINE_LOOP 0x0002
#define GL_LINE_STRIP 0x0003
#define GL_TRIANGLES 0x0004
#define GL_QUADS 0x0007

#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_DEPTH_BUFFER_BIT 0x00000100

#define GL_MODELVIEW 0x1700
#define GL_PROJECTION 0x1701

#define GL_BLEND 0x0BE2
#define GL_SRC_ALPHA 0x0302
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_ONE 1
#define GL_ZERO 0

#define GL_TEXTURE_2D 0x0DE1
#define GL_TEXTURE_RECTANGLE_ARB 0x84F5
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_CLAMP_TO_EDGE 0x812F

#define GL_UNPACK_ALIGNMENT 0x0CF5
#define GL_UNSIGNED_BYTE 0x1401
#define GL_RGB 0x1907
#define GL_RGBA 0x1908
#define GL_BGRA 0x80E1

// -- the subset --------------------------------------------------------------
void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
void glClearColor(GLclampf r, GLclampf g, GLclampf b, GLclampf a);
void glClear(GLbitfield mask);

void glMatrixMode(GLenum mode);
void glLoadIdentity(void);
void glPushMatrix(void);
void glPopMatrix(void);
void glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top,
             GLdouble nearVal, GLdouble farVal);
void glTranslated(GLdouble x, GLdouble y, GLdouble z);
void glTranslatef(GLfloat x, GLfloat y, GLfloat z);

void glEnable(GLenum cap);
void glDisable(GLenum cap);
void glBlendFunc(GLenum sfactor, GLenum dfactor);
void glPixelStorei(GLenum pname, GLint param);

void glGenTextures(GLsizei n, GLuint* textures);
void glDeleteTextures(GLsizei n, const GLuint* textures);
void glBindTexture(GLenum target, GLuint texture);
void glTexParameteri(GLenum target, GLenum pname, GLint param);
void glTexImage2D(GLenum target, GLint level, GLint internalFormat,
                  GLsizei width, GLsizei height, GLint border, GLenum format,
                  GLenum type, const GLvoid* pixels);

void glBegin(GLenum mode);
void glEnd(void);
void glVertex2d(GLdouble x, GLdouble y);
void glVertex2f(GLfloat x, GLfloat y);
void glTexCoord2d(GLdouble s, GLdouble t);
void glTexCoord2f(GLfloat s, GLfloat t);
void glColor3f(GLfloat r, GLfloat g, GLfloat b);
void glColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
void glColor4dv(const GLdouble* v);

// -- the bit that is not OpenGL ----------------------------------------------
namespace softgl {

// Point the rasteriser at the surface to draw into.  `pixels` is 32 bits per
// pixel; `shift_*` say where each channel sits, which SDL reports per surface
// rather than fixing.
void set_target(void* pixels, int width, int height, int pitch_bytes,
                int shift_r, int shift_g, int shift_b, int shift_a);

// True once set_target has been given somewhere to draw.
bool has_target();

}  // namespace softgl
