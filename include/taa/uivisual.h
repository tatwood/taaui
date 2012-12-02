/**
 * @brief     ui visual style management header
 * @author    Thomas Atwood (tatwood.net)
 * @date      2011
 * @copyright unlicense / public domain
 ****************************************************************************/
#ifndef taa_UIVISUAL_H_
#define taa_UIVISUAL_H_

#include "uidrawlist.h"
#include <taa/mat44.h>
#include <taa/vec2.h>

//****************************************************************************
// typedefs

typedef struct taa_ui_visual_rect_s taa_ui_visual_rect;
typedef struct taa_ui_visual_s taa_ui_visual;
typedef struct taa_ui_visual_map_s taa_ui_visual_map;

//****************************************************************************
// structs

struct taa_ui_visual_rect_s
{
    taa_vec2 uvlt;
    taa_vec2 uvrb;
    taa_texture2d texture;
    uint32_t width;
    uint32_t height;
};

/**
 * @details there is a one-to-many relation of ui styles to visuals. each
 *          style may select from multiple visuals depending on the active
 *          control flags
 */
struct taa_ui_visual_s
{
    /// color to multiply against control foreground (ex: text/image)
    uint32_t fgcolor;
    /// color to multiply against control background texture
    uint32_t bgcolor;
    /// color to multiply against control border texture
    uint32_t bordercolor;
    /// texture and uv rectangle for control background
    taa_ui_visual_rect background;
    /// texture and uv rectangle for control left border
    taa_ui_visual_rect lborder;
    /// texture and uv rectangle for control right border
    taa_ui_visual_rect rborder;
    /// texture and uv rectangle for control top border
    taa_ui_visual_rect tborder;
    /// texture and uv rectangle for control bottom border
    taa_ui_visual_rect bborder;
    /// texture and uv rectangle for control left-top corner
    taa_ui_visual_rect ltcorner;
    /// texture and uv rectangle for control right-top corner
    taa_ui_visual_rect rtcorner;
    /// texture and uv rectangle for control left-bottom corner
    taa_ui_visual_rect lbcorner;
    /// texture and uv rectangle for control right-bottom corner
    taa_ui_visual_rect rbcorner;
};

taa_UI_LINKAGE void taa_ui_calc_visual_rect(
    taa_texture2d texture,
    uint32_t texturew,
    uint32_t textureh,
    int32_t lx,
    int32_t ty,
    int32_t rx,
    int32_t by,
    taa_ui_visual_rect* rect_out);

taa_UI_LINKAGE void taa_ui_create_visual_map(
    uint32_t numstyles,
    uint32_t numvisuals,
    taa_ui_visual_map** map_out);

taa_UI_LINKAGE void taa_ui_destroy_visual_map(
    taa_ui_visual_map* map);

taa_UI_LINKAGE void taa_ui_draw_visual_background(
    const taa_ui_style* style,
    const taa_ui_visual* visual,
    const taa_ui_rect* rect,
    const taa_ui_rect* clip,
    taa_ui_drawlist* drawlist);

taa_UI_LINKAGE void taa_ui_draw_visual_border(
    const taa_ui_style* style,
    const taa_ui_visual* visual,
    const taa_ui_rect* rect,
    const taa_ui_rect* clip,
    taa_ui_drawlist* drawlist);

taa_UI_LINKAGE void taa_ui_draw_visual_text(
    const taa_ui_style* style,
    const taa_ui_visual* visual,
    const taa_ui_rect* rect,
    const taa_ui_rect* clip,
    const char* text,
    uint32_t textlen,
    int32_t caret,
    uint32_t selectstart,
    uint32_t selectlength,
    taa_ui_drawlist* drawlist);

taa_UI_LINKAGE const taa_ui_visual* taa_ui_find_visual(
    taa_ui_visual_map* map,
    uint32_t styleid,
    uint32_t flags);

/**
 * @details given a key composed of a ui style id and control state flags,
 *          this function inserts a visual settings record into the map.
 * @param map handle to the visual map
 * @param styleid index of the ui style to receive the visual settings
 * @param flags ui control flags mask used to receive the visual settings
 * @param srcvisuals visual settings to copy into the map value
 */
taa_UI_LINKAGE void taa_ui_insert_visual(
    taa_ui_visual_map* map,
    uint32_t styleid,
    uint32_t flags,
    const taa_ui_visual* srcvisuals);

#endif // taa_UIVISUAL_H_
