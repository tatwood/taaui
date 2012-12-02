/**
 * @brief     render command processing header
 * @author    Thomas Atwood (tatwood.net)
 * @date      2011
 * @copyright unlicense / public domain
 ****************************************************************************/
#ifndef taa_UIRENDER_H_
#define taa_UIRENDER_H_

#include "uidrawlist.h"

typedef struct taa_ui_render_data_s taa_ui_render_data;

taa_UI_LINKAGE void taa_ui_create_render_data(
    taa_ui_render_data** rnd);

taa_UI_LINKAGE void taa_ui_destroy_render_data(
    taa_ui_render_data* rnd);

taa_UI_LINKAGE void taa_ui_render(
    taa_ui_render_data* rnd,
    int vieww,
    int viewh,
    taa_vertexbuffer vb,
    const taa_ui_drawlist_cmd* cmds,
    size_t numcmds);

#endif // taa_UIRENDER_H_
