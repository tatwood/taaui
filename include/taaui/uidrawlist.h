/**
 * @brief     ui render buffer and command list generation header
 * @author    Thomas Atwood (tatwood.net)
 * @date      2011
 * @copyright unlicense / public domain
 ****************************************************************************/
#ifndef TAAUI_UIDRAWLIST_H_
#define TAAUI_UIDRAWLIST_H_

#include "ui.h"

typedef taa_font_vertex taa_uidrawlist_vertex;
typedef struct taa_uidrawlist_cmd_s taa_uidrawlist_cmd;
typedef struct taa_uidrawlist_s taa_uidrawlist;
typedef struct taa_uidrawlist_builder_s taa_uidrawlist_builder;

struct taa_uidrawlist_cmd_s
{
    taa_mat44 transform;
    taa_texture2d texture;
    uint32_t vboffset;
    uint32_t numvertices;
};

struct taa_uidrawlist_s
{
    taa_vertexbuffer vb;
    taa_uidrawlist_cmd* cmds;
    uint32_t numcmds;
};

//****************************************************************************

taa_EXTERN_C void taa_uidrawlist_addrect(
    taa_uidrawlist_builder* dlb,
    taa_texture2d texture,
    uint32_t color,
    int32_t x,
    int32_t y,
    int32_t w,
    int32_t h,
    const taa_ui_rect* cliprect,
    const taa_vec2* uvlt,
    const taa_vec2* uvrb);

taa_EXTERN_C void taa_uidrawlist_addtext(
    taa_uidrawlist_builder* dlb,
    const taa_font* font,
    uint32_t color,
    const char* txt,
    uint32_t txtlen,
    int32_t x,
    int32_t y,
    int32_t w,
    int32_t h,
    int32_t scrollx,
    int32_t scrolly,
    taa_ui_halign halign,
    taa_ui_valign valign,
    const taa_ui_rect* cliprect);

taa_EXTERN_C void taa_uidrawlist_begin(
    taa_uidrawlist_builder* dlb);

taa_EXTERN_C void taa_uidrawlist_createbuilder(
    uint32_t verticessize,
    uint32_t maxcmds,
    taa_uidrawlist_builder** dlbout);

taa_EXTERN_C void taa_uidrawlist_destroybuilder(
    taa_uidrawlist_builder* dlb);

taa_EXTERN_C void taa_uidrawlist_end(
    taa_uidrawlist_builder* dlb,
    taa_uidrawlist* dlout);

taa_EXTERN_C void taa_uidrawlist_poptransform(
    taa_uidrawlist_builder* dlb);

taa_EXTERN_C void taa_uidrawlist_pushtransform(
    taa_uidrawlist_builder* dlb,
    const taa_mat44* transform);

taa_EXTERN_C void taa_uidrawlist_render(
    uint32_t vieww,
    uint32_t viewh,
    const taa_uidrawlist* dl);

#endif // TAAUI_UIRENDER_H_
