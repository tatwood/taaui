/**
 * @brief     bitmap font header
 * @author    Thomas Atwood (tatwood.net)
 * @date      2011
 * @copyright unlicense / public domain
 ****************************************************************************/
#ifndef TAAUI_FONT_H_
#define TAAUI_FONT_H_

#include <taa/math.h>
#include <taa/render.h>

//****************************************************************************
// typedefs

typedef struct taa_font_char_s taa_font_char;
typedef struct taa_font_vertex_s taa_font_vertex;
typedef struct taa_font_s taa_font;

//****************************************************************************
// structs

struct taa_font_char_s
{
    uint32_t width;
    taa_vec2 uv0;
    taa_vec2 uv1;
};

struct taa_font_vertex_s
{
    taa_vec2 pos;
    taa_vec2 uv;
    uint32_t color;
};

struct taa_font_s
{
    uint32_t maxcharwidth;
    uint32_t charheight;
    uint32_t texwidth;
    uint32_t texheight;
    taa_font_char characters[256];
    taa_texture2d texture;
};

taa_EXTERN_C uint32_t taa_font_countvertices(
    const char* text,
    uint32_t txtlen);

taa_EXTERN_C void taa_font_create(
    taa_font* fontout);

taa_EXTERN_C void taa_font_destroy(
    taa_font* font);

/**
 * <p>generates a list of font verts as triangles</p>
 * @return the number of vertices written to the buffer
 */
taa_EXTERN_C uint32_t taa_font_genvertices(
    const taa_font* font,
    const char* txt,
    uint32_t txtlen,
    int32_t x,
    int32_t y,
    uint32_t w,
    uint32_t h,
    int32_t scrollx,
    int32_t scrolly,
    uint32_t color,
    taa_font_vertex* vertsout,
    uint32_t vertcapacity);

taa_EXTERN_C void taa_font_load(
    taa_font* font,
    const void* buf,
    uint32_t size);

taa_EXTERN_C uint32_t taa_font_textwidth(
    const taa_font* font,
    const char* txt,
    uint32_t txtlen);

#endif // TAAUI_FONT_H_
