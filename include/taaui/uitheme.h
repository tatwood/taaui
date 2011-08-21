/**
 * @brief     default ui theme header
 * @author    Thomas Atwood (tatwood.net)
 * @date      2011
 * @copyright unlicense / public domain
 ****************************************************************************/
#ifndef TAAUI_UITHEME_H_
#define TAAUI_UITHEME_H_

#include "uidrawlist.h"
#include <taa/math.h>

//****************************************************************************
// typedefs

typedef struct taa_uitheme_visualrect_s taa_uitheme_visualrect;
typedef struct taa_uitheme_visuals_s taa_uitheme_visuals;
typedef struct taa_uitheme_s taa_uitheme;

//****************************************************************************
// structs

struct taa_uitheme_visualrect_s
{
    taa_vec2 uvlt;
    taa_vec2 uvrb;
    taa_texture2d texture;
};

struct taa_uitheme_visuals_s
{
    uint32_t fgcolor;
    uint32_t bgcolor;
    taa_uitheme_visualrect background;
    taa_uitheme_visualrect lborder;
    taa_uitheme_visualrect rborder;
    taa_uitheme_visualrect tborder;
    taa_uitheme_visualrect bborder;
    taa_uitheme_visualrect ltcorner;
    taa_uitheme_visualrect rtcorner;
    taa_uitheme_visualrect lbcorner;
    taa_uitheme_visualrect rbcorner;
};

//****************************************************************************
// functions

taa_EXTERN_C void taa_uitheme_calcvisualrect(
    taa_texture2d texture,
    uint32_t texturew,
    uint32_t textureh,
    int32_t lx,
    int32_t ty,
    int32_t rx,
    int32_t by,
    taa_uitheme_visualrect* rectout);

taa_EXTERN_C void taa_uitheme_create(
    uint32_t numstyles,
    uint32_t numvisuals,
    taa_uitheme** themeout);

taa_EXTERN_C void taa_uitheme_destroy(
    taa_uitheme* theme);

taa_EXTERN_C void taa_uitheme_drawrect(
    const taa_ui_style* style,
    const taa_uitheme_visuals* visuals,
    const taa_ui_rect* rect,
    const taa_ui_rect* clip,
    taa_uidrawlist_builder* dlb);

taa_EXTERN_C void taa_uitheme_drawrectcontrol(
    const taa_uitheme* theme,
    const taa_ui_controllist* ctrls,
    const taa_ui_control* ctrl,
    taa_uidrawlist_builder* dlb);

taa_EXTERN_C void taa_uitheme_drawscrollcontrol(
    const taa_uitheme* theme,
    const taa_ui_controllist* ctrls,
    const taa_ui_control* ctrl,
    taa_uidrawlist_builder* dlb);

taa_EXTERN_C void taa_uitheme_drawtextcontrol(
    const taa_uitheme* theme,
    const taa_ui_controllist* ctrls,
    const taa_ui_control* ctrl,
    taa_uidrawlist_builder* dlb);

taa_EXTERN_C const taa_uitheme_visuals* taa_uitheme_findvisuals(
    const taa_uitheme* theme,
    uint32_t styleid,
    uint32_t flags);

taa_EXTERN_C const taa_ui_stylesheet* taa_uitheme_getstylesheet(
    const taa_uitheme* theme);

taa_EXTERN_C void taa_uitheme_setstyle(
    taa_uitheme* theme,
    uint32_t styleid,
    const taa_ui_style* srcstyle);

taa_EXTERN_C void taa_uitheme_setvisuals(
    taa_uitheme* theme,
    uint32_t styleid,
    uint32_t flags,
    const taa_uitheme_visuals* srcvisuals);

#endif // TAAUI_UITHEME_H_
