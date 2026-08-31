#include "softgl.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <map>
#include <vector>

namespace {

struct Texture {
    int width = 0;
    int height = 0;
    std::vector<uint32_t> rgba;   // r,g,b,a in memory order
};

struct Vertex {
    double x = 0, y = 0;          // world space
    double s = 0, t = 0;          // texture rectangle coordinates, in texels
};

struct Matrix2D {
    // the engine only ever translates, so a full 4x4 would be dead weight
    double tx = 0, ty = 0;
};

struct State {
    // target surface
    uint8_t* pixels = nullptr;
    int width = 0, height = 0, pitch = 0;
    int shift_r = 0, shift_g = 8, shift_b = 16, shift_a = 24;

    // viewport and projection
    int vp_x = 0, vp_y = 0, vp_w = 0, vp_h = 0;
    double left = 0, right = 1, bottom = 0, top = 1;

    // matrices
    GLenum matrix_mode = GL_MODELVIEW;
    Matrix2D modelview;
    std::vector<Matrix2D> modelview_stack;
    std::vector<Matrix2D> projection_stack;

    // colours
    double cr = 1, cg = 1, cb = 1, ca = 1;
    double clear_r = 0, clear_g = 0, clear_b = 0, clear_a = 1;

    // toggles
    bool blend = false;
    bool texturing = false;

    // textures
    std::map<GLuint, Texture> textures;
    GLuint bound = 0;
    GLuint next_id = 1;

    // immediate mode
    bool in_begin = false;
    GLenum mode = 0;
    double cur_s = 0, cur_t = 0;
    std::vector<Vertex> verts;
};

State g;

inline uint32_t to_byte(double v) {
    int i = static_cast<int>(v * 255.0 + 0.5);
    return static_cast<uint32_t>(i < 0 ? 0 : (i > 255 ? 255 : i));
}

inline uint32_t pack(double r, double g_, double b, double a) {
    const auto to8 = [](double v) -> uint32_t {
        int i = static_cast<int>(v * 255.0 + 0.5);
        if (i < 0) i = 0;
        if (i > 255) i = 255;
        return static_cast<uint32_t>(i);
    };
    return (to8(r) << g.shift_r) | (to8(g_) << g.shift_g) |
           (to8(b) << g.shift_b) | (to8(a) << g.shift_a);
}

inline uint32_t* row(int y) {
    return reinterpret_cast<uint32_t*>(g.pixels + static_cast<size_t>(y) * g.pitch);
}

// world -> screen.  glOrtho puts the origin bottom left with y up; a
// framebuffer counts down from the top, so y is flipped exactly once, here.
inline void to_screen(double wx, double wy, double* sx, double* sy) {
    const double x = wx + g.modelview.tx;
    const double y = wy + g.modelview.ty;
    const double nx = (x - g.left) / (g.right - g.left);
    const double ny = (y - g.bottom) / (g.top - g.bottom);
    *sx = g.vp_x + nx * g.vp_w;
    *sy = g.vp_y + (1.0 - ny) * g.vp_h;
}

// Integer source colour, already in the target's channel order.
inline uint32_t pack8(uint32_t r, uint32_t g_, uint32_t b) {
    return (r << g.shift_r) | (g_ << g.shift_g) | (b << g.shift_b) |
           (0xffu << g.shift_a);
}

// src over dst with an 8-bit alpha.  The +128/+8 rounding is the usual trick
// for dividing by 255 with shifts.
inline uint32_t blend8(uint32_t dst, uint32_t sr, uint32_t sg, uint32_t sb,
                       uint32_t a) {
    const uint32_t ia = 255u - a;
    const uint32_t dr = (dst >> g.shift_r) & 0xff;
    const uint32_t dg = (dst >> g.shift_g) & 0xff;
    const uint32_t db = (dst >> g.shift_b) & 0xff;
    const uint32_t r = (sr * a + dr * ia + 127u) / 255u;
    const uint32_t gg = (sg * a + dg * ia + 127u) / 255u;
    const uint32_t b = (sb * a + db * ia + 127u) / 255u;
    return pack8(r, gg, b);
}

inline void blend_pixel(int x, int y, double r, double gg, double b, double a) {
    if (x < 0 || y < 0 || x >= g.width || y >= g.height) return;
    if (a <= 0.0) return;
    const auto to8 = [](double v) -> uint32_t {
        int i = static_cast<int>(v * 255.0 + 0.5);
        return static_cast<uint32_t>(i < 0 ? 0 : (i > 255 ? 255 : i));
    };
    uint32_t* p = row(y) + x;
    if (!g.blend || a >= 1.0) {
        *p = pack8(to8(r), to8(gg), to8(b));
        return;
    }
    *p = blend8(*p, to8(r), to8(gg), to8(b), to8(a));
}

const Texture* bound_texture() {
    if (!g.texturing || g.bound == 0) return nullptr;
    auto it = g.textures.find(g.bound);
    if (it == g.textures.end() || it->second.rgba.empty()) return nullptr;
    return &it->second;
}

// One axis-aligned quad.  v0 and v2 are opposite corners, which is enough to
// derive the texture mapping in both directions and copes with a flipped
// texture rect without a special case.
void draw_quad(const Vertex& v0, const Vertex& v2) {
    double x0, y0, x2, y2;
    to_screen(v0.x, v0.y, &x0, &y0);
    to_screen(v2.x, v2.y, &x2, &y2);

    double sx0 = std::min(x0, x2), sx1 = std::max(x0, x2);
    double sy0 = std::min(y0, y2), sy1 = std::max(y0, y2);

    int px0 = static_cast<int>(std::floor(sx0 + 0.5));
    int px1 = static_cast<int>(std::floor(sx1 + 0.5));
    int py0 = static_cast<int>(std::floor(sy0 + 0.5));
    int py1 = static_cast<int>(std::floor(sy1 + 0.5));
    if (px1 <= px0) px1 = px0 + 1;      // never drop a sub-pixel sliver
    if (py1 <= py0) py1 = py0 + 1;

    const int cx0 = std::max(px0, 0), cx1 = std::min(px1, g.width);
    const int cy0 = std::max(py0, 0), cy1 = std::min(py1, g.height);
    if (cx0 >= cx1 || cy0 >= cy1) return;

    const Texture* tex = bound_texture();
    if (!tex) {
        const uint32_t sr = to_byte(g.cr), sg = to_byte(g.cg);
        const uint32_t sb = to_byte(g.cb), a = to_byte(g.ca);
        if (!a) return;
        const uint32_t solid = pack8(sr, sg, sb);
        const bool opaque = (!g.blend || a == 255u);
        for (int y = cy0; y < cy1; ++y) {
            uint32_t* dst = row(y) + cx0;
            if (opaque) {
                for (int x = cx0; x < cx1; ++x) *dst++ = solid;
            } else {
                for (int x = cx0; x < cx1; ++x, ++dst)
                    *dst = blend8(*dst, sr, sg, sb, a);
            }
        }
        return;
    }

    // texel coordinates run with x0->s0 and y0->t0; the screen y flip means the
    // t axis runs opposite to the pixel rows.  16.16 fixed point, because this
    // loop runs a million times a frame and doubles here are what make the tab
    // stop answering the browser.
    const double dsd = (v2.s - v0.s) / (px1 - px0);
    const double dtd = (v2.t - v0.t) / (py1 - py0);
    const int32_t ds = static_cast<int32_t>(dsd * 65536.0);
    const int32_t dt = static_cast<int32_t>(dtd * 65536.0);
    const bool y_flipped = (y0 > y2);

    const uint32_t mr = to_byte(g.cr), mg = to_byte(g.cg), mb = to_byte(g.cb);
    const uint32_t ma = to_byte(g.ca);
    const bool modulated = (mr != 255 || mg != 255 || mb != 255 || ma != 255);

    for (int y = cy0; y < cy1; ++y) {
        const int fy = y_flipped ? (py1 - 1 - y) : (y - py0);
        int32_t t = static_cast<int32_t>(v0.t * 65536.0) +
                    static_cast<int32_t>((fy + 0.5) * dtd * 65536.0);
        int ty = t >> 16;
        if (ty < 0) ty = 0;
        if (ty >= tex->height) ty = tex->height - 1;
        const uint32_t* trow = &tex->rgba[static_cast<size_t>(ty) * tex->width];

        int32_t s = static_cast<int32_t>(v0.s * 65536.0) +
                    static_cast<int32_t>((cx0 - px0 + 0.5) * dsd * 65536.0);
        uint32_t* dst = row(y) + cx0;

        for (int x = cx0; x < cx1; ++x, s += ds, ++dst) {
            int tx = s >> 16;
            if (tx < 0) tx = 0;
            else if (tx >= tex->width) tx = tex->width - 1;

            const uint32_t texel = trow[tx];
            uint32_t a = texel >> 24;
            if (!a) continue;
            uint32_t sr = texel & 0xff;
            uint32_t sg = (texel >> 8) & 0xff;
            uint32_t sb = (texel >> 16) & 0xff;
            if (modulated) {
                sr = sr * mr / 255u;
                sg = sg * mg / 255u;
                sb = sb * mb / 255u;
                a = a * ma / 255u;
                if (!a) continue;
            }
            if (!g.blend || a == 255u) *dst = pack8(sr, sg, sb);
            else *dst = blend8(*dst, sr, sg, sb, a);
        }
    }
}

void draw_line(const Vertex& a, const Vertex& b) {
    double ax, ay, bx, by;
    to_screen(a.x, a.y, &ax, &ay);
    to_screen(b.x, b.y, &bx, &by);

    const double dx = bx - ax, dy = by - ay;
    const int steps = static_cast<int>(std::max(std::fabs(dx), std::fabs(dy))) + 1;
    for (int i = 0; i <= steps; ++i) {
        const double f = static_cast<double>(i) / steps;
        blend_pixel(static_cast<int>(std::lround(ax + dx * f)),
                    static_cast<int>(std::lround(ay + dy * f)),
                    g.cr, g.cg, g.cb, g.ca);
    }
}

}  // namespace

// -- API ---------------------------------------------------------------------
namespace softgl {

void set_target(void* pixels, int width, int height, int pitch_bytes,
                int shift_r, int shift_g, int shift_b, int shift_a) {
    g.pixels = static_cast<uint8_t*>(pixels);
    g.width = width;
    g.height = height;
    g.pitch = pitch_bytes;
    g.shift_r = shift_r;
    g.shift_g = shift_g;
    g.shift_b = shift_b;
    g.shift_a = shift_a;
    if (g.vp_w == 0 || g.vp_h == 0) {
        g.vp_w = width;
        g.vp_h = height;
    }
}

bool has_target() { return g.pixels != nullptr; }

}  // namespace softgl

void glViewport(GLint x, GLint y, GLsizei w, GLsizei h) {
    g.vp_x = x;
    g.vp_y = y;
    g.vp_w = w;
    g.vp_h = h;
}

void glClearColor(GLclampf r, GLclampf gg, GLclampf b, GLclampf a) {
    g.clear_r = r;
    g.clear_g = gg;
    g.clear_b = b;
    g.clear_a = a;
}

void glClear(GLbitfield mask) {
    if (!(mask & GL_COLOR_BUFFER_BIT) || !g.pixels) return;
    const uint32_t c = pack(g.clear_r, g.clear_g, g.clear_b, 1.0);
    for (int y = 0; y < g.height; ++y) {
        uint32_t* p = row(y);
        for (int x = 0; x < g.width; ++x) p[x] = c;
    }
}

void glMatrixMode(GLenum mode) { g.matrix_mode = mode; }

void glLoadIdentity(void) {
    if (g.matrix_mode == GL_MODELVIEW) g.modelview = Matrix2D();
    // the projection is defined by glOrtho, which overwrites it wholesale
}

void glPushMatrix(void) {
    if (g.matrix_mode == GL_MODELVIEW) g.modelview_stack.push_back(g.modelview);
    else g.projection_stack.push_back(Matrix2D());
}

void glPopMatrix(void) {
    if (g.matrix_mode == GL_MODELVIEW) {
        if (!g.modelview_stack.empty()) {
            g.modelview = g.modelview_stack.back();
            g.modelview_stack.pop_back();
        }
    } else if (!g.projection_stack.empty()) {
        g.projection_stack.pop_back();
    }
}

void glOrtho(GLdouble l, GLdouble r, GLdouble b, GLdouble t, GLdouble, GLdouble) {
    g.left = l;
    g.right = r;
    g.bottom = b;
    g.top = t;
}

void glTranslated(GLdouble x, GLdouble y, GLdouble) {
    if (g.matrix_mode == GL_MODELVIEW) {
        g.modelview.tx += x;
        g.modelview.ty += y;
    }
}

void glTranslatef(GLfloat x, GLfloat y, GLfloat z) {
    glTranslated(x, y, z);
}

void glEnable(GLenum cap) {
    if (cap == GL_BLEND) g.blend = true;
    else if (cap == GL_TEXTURE_RECTANGLE_ARB || cap == GL_TEXTURE_2D)
        g.texturing = true;
}

void glDisable(GLenum cap) {
    if (cap == GL_BLEND) g.blend = false;
    else if (cap == GL_TEXTURE_RECTANGLE_ARB || cap == GL_TEXTURE_2D)
        g.texturing = false;
}

void glBlendFunc(GLenum, GLenum) {
    // the engine only ever asks for src_alpha / one_minus_src_alpha, which is
    // what blend_pixel does
}

void glPixelStorei(GLenum, GLint) {
    // rows are always tightly packed here
}

void glGenTextures(GLsizei n, GLuint* textures) {
    for (GLsizei i = 0; i < n; ++i) {
        const GLuint id = g.next_id++;
        g.textures[id] = Texture();
        textures[i] = id;
    }
}

void glDeleteTextures(GLsizei n, const GLuint* textures) {
    for (GLsizei i = 0; i < n; ++i) {
        if (g.bound == textures[i]) g.bound = 0;
        g.textures.erase(textures[i]);
    }
}

void glBindTexture(GLenum, GLuint texture) { g.bound = texture; }

void glTexParameteri(GLenum, GLenum, GLint) {
    // sampling is always nearest; the engine asks for nothing else
}

void glTexImage2D(GLenum, GLint, GLint, GLsizei width, GLsizei height, GLint,
                  GLenum format, GLenum type, const GLvoid* pixels) {
    if (g.bound == 0) return;
    Texture& tex = g.textures[g.bound];
    tex.width = width;
    tex.height = height;
    tex.rgba.assign(static_cast<size_t>(width) * height, 0);
    if (!pixels || type != GL_UNSIGNED_BYTE) return;

    const uint8_t* src = static_cast<const uint8_t*>(pixels);
    const int comps = (format == GL_RGB) ? 3 : 4;
    for (size_t i = 0; i < tex.rgba.size(); ++i) {
        const uint8_t* p = src + i * comps;
        const uint32_t a = (comps == 4) ? p[3] : 0xffu;
        tex.rgba[i] = static_cast<uint32_t>(p[0]) |
                      (static_cast<uint32_t>(p[1]) << 8) |
                      (static_cast<uint32_t>(p[2]) << 16) | (a << 24);
    }
}

void glBegin(GLenum mode) {
    g.in_begin = true;
    g.mode = mode;
    g.verts.clear();
}

void glEnd(void) {
    g.in_begin = false;
    if (!g.pixels) return;

    if (g.mode == GL_QUADS) {
        for (size_t i = 0; i + 3 < g.verts.size(); i += 4)
            draw_quad(g.verts[i], g.verts[i + 2]);
    } else if (g.mode == GL_LINES) {
        for (size_t i = 0; i + 1 < g.verts.size(); i += 2)
            draw_line(g.verts[i], g.verts[i + 1]);
    } else if (g.mode == GL_LINE_STRIP || g.mode == GL_LINE_LOOP) {
        for (size_t i = 0; i + 1 < g.verts.size(); ++i)
            draw_line(g.verts[i], g.verts[i + 1]);
        if (g.mode == GL_LINE_LOOP && g.verts.size() > 2)
            draw_line(g.verts.back(), g.verts.front());
    }
    g.verts.clear();
}

void glVertex2d(GLdouble x, GLdouble y) {
    Vertex v;
    v.x = x;
    v.y = y;
    v.s = g.cur_s;
    v.t = g.cur_t;
    g.verts.push_back(v);
}

void glVertex2f(GLfloat x, GLfloat y) { glVertex2d(x, y); }

void glTexCoord2d(GLdouble s, GLdouble t) {
    g.cur_s = s;
    g.cur_t = t;
}

void glTexCoord2f(GLfloat s, GLfloat t) { glTexCoord2d(s, t); }

void glColor3f(GLfloat r, GLfloat gg, GLfloat b) {
    g.cr = r;
    g.cg = gg;
    g.cb = b;
    g.ca = 1.0;
}

void glColor4f(GLfloat r, GLfloat gg, GLfloat b, GLfloat a) {
    g.cr = r;
    g.cg = gg;
    g.cb = b;
    g.ca = a;
}

void glColor4dv(const GLdouble* v) {
    g.cr = v[0];
    g.cg = v[1];
    g.cb = v[2];
    g.ca = v[3];
}
