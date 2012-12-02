#ifndef UITHEME_H_
#define UITHEME_H_

#include <taa/uidrawlist.h>
#include <taa/uivisual.h>

//****************************************************************************
// typedefs

typedef struct uitheme_s uitheme;

//****************************************************************************
// enums

enum
{
    UITHEME_BUTTON,
    UITHEME_LABEL,
    UITHEME_LABEL_CENTER,
    UITHEME_LISTROW0,
    UITHEME_LISTROW1,
    UITHEME_SCROLLBAR,
    UITHEME_SCROLLPANE,
    UITHEME_TEXTBOX,
    UITHEME_WINDOW,
    UITHEME_NUM_STYLES
};

enum
{
    UITHEME_SPACING = 5
};

struct uitheme_s
{
    taa_ui_font font;
    taa_texture2d texture;
    taa_ui_style stylesheet[UITHEME_NUM_STYLES];
    taa_ui_visual_map* visuals;
};

//****************************************************************************
// functions

void uitheme_create(
    uitheme* theme_out);

void uitheme_destroy(
    uitheme* theme);

void uitheme_draw(
    const uitheme* theme,
    const taa_ui_controllist* ctrls,
    taa_ui_drawlist* drawlist);

#endif // UITHEME_H_
