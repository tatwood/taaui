/**
 * @brief     ui vertex buffer and command list generation header
 * @details   The primary purpose of this file is to allow the batching of ui
 *            draw calls into as few commands as possible. A renderer could
 *            be implemented based directly on the ui control list, but that
 *            would result in a large number of small draw calls.
 * @author    Thomas Atwood (tatwood.net)
 * @date      2011
 * @copyright unlicense / public domain
 ****************************************************************************/
#ifndef taa_UIDRAWLIST_H_
#define taa_UIDRAWLIST_H_

#include "ui.h"

typedef struct taa_ui_drawlist_cmd_s taa_ui_drawlist_cmd;
typedef struct taa_ui_drawlist_s taa_ui_drawlist;

struct taa_ui_drawlist_cmd_s
{
    taa_mat44 transform;
    taa_texture2d texture;
    uint32_t vboffset;
    uint32_t numvertices;
};

//****************************************************************************

taa_UI_LINKAGE void taa_ui_add_drawlist_rect(
    taa_ui_drawlist* drawlist,
    taa_texture2d texture,
    uint32_t color,
    int x,
    int y,
    int w,
    int h,
    const taa_ui_rect* cliprect,
    const taa_vec2* uvlt,
    const taa_vec2* uvrb);

taa_UI_LINKAGE void taa_ui_add_drawlist_text(
    taa_ui_drawlist* drawlist,
    const taa_ui_font* font,
    uint32_t color,
    const char* txt,
    size_t txtlen,
    int x,
    int y,
    int w,
    int h,
    int scrollx,
    int scrolly,
    taa_ui_halign halign,
    taa_ui_valign valign,
    const taa_ui_rect* cliprect);

taa_UI_LINKAGE void taa_ui_begin_drawlist(
    taa_ui_drawlist* drawlist,
    taa_ui_drawlist_cmd* cmds,
    size_t maxcmds,
    taa_ui_vertex* verts,
    size_t maxverts);

taa_UI_LINKAGE void taa_ui_create_drawlist(
    taa_ui_drawlist** drawlist_out);

taa_UI_LINKAGE void taa_ui_destroy_drawlist(
    taa_ui_drawlist* drawlist);

taa_UI_LINKAGE void taa_ui_end_drawlist(
    taa_ui_drawlist* drawlist,
    size_t* numcmds_out,
    size_t* numverts_out);

taa_UI_LINKAGE void taa_ui_drawlist_pop_transform(
    taa_ui_drawlist* drawlist);

taa_UI_LINKAGE void taa_ui_drawlist_push_transform(
    taa_ui_drawlist* drawlist,
    const taa_mat44* transform);

#endif // taa_UIRENDER_H_
