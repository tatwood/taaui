/**
 * @brief     ui simulation header
 * @author    Thomas Atwood (tatwood.net)
 * @date      2011
 * @copyright unlicense / public domain
 ****************************************************************************/
#ifndef TAAUI_UI_H_
#define TAAUI_UI_H_

#include "font.h"

//****************************************************************************
// typedefs

typedef uint32_t taa_ui_styleid;

typedef enum taa_ui_datatype_e taa_ui_datatype;
typedef enum taa_ui_halign_e taa_ui_halign;
typedef enum taa_ui_type_e taa_ui_type;
typedef enum taa_ui_valign_e taa_ui_valign;

typedef struct taa_ui_rect_s taa_ui_rect;
typedef struct taa_ui_style_s taa_ui_style;
typedef struct taa_ui_stylesheet_s taa_ui_stylesheet;
typedef struct taa_ui_iddata_s taa_ui_iddata;
typedef struct taa_ui_scrolldata_s taa_ui_scrolldata;
typedef struct taa_ui_textdata_s taa_ui_textdata;
typedef union taa_ui_data_u taa_ui_data;
typedef struct taa_ui_control_s taa_ui_control;
typedef struct taa_ui_controllist_s  taa_ui_controllist;
typedef struct taa_ui_s taa_ui;

//****************************************************************************
// enums

enum taa_ui_datatype_e
{
    taa_UI_DATA_ID,
    taa_UI_DATA_SCROLL,
    taa_UI_DATA_TEXT
};

enum taa_ui_halign_e
{
    taa_UI_HALIGN_CENTER,
    taa_UI_HALIGN_LEFT,
    taa_UI_HALIGN_RIGHT
};

enum taa_ui_type_e
{
    taa_UI_BUTTON,
    taa_UI_CONTAINER,
    taa_UI_LABEL,
    taa_UI_TEXTBOX,
    taa_UI_VSCROLLBAR,
};

enum taa_ui_valign_e
{
    taa_UI_VALIGN_CENTER,
    taa_UI_VALIGN_TOP,
    taa_UI_VALIGN_BOTTOM
};

enum
{
    taa_UI_WIDTH_DEFAULT = 0,
    taa_UI_WIDTH_FILL = -1,
    taa_UI_HEIGHT_DEFAULT = 0,
    taa_UI_HEIGHT_FILL = -1
};

enum
{
    taa_UI_FLAG_NONE     = 0,
    taa_UI_FLAG_DISABLED = 1 << 0, // 1
    taa_UI_FLAG_HOVER    = 1 << 1, // 2
    taa_UI_FLAG_FOCUS    = 1 << 2, // 4
    taa_UI_FLAG_PRESSED  = 1 << 3, // 8
    taa_UI_FLAG_CLICKED  = 1 << 4, // 16
};

//****************************************************************************
// structs

struct taa_ui_rect_s
{
    int32_t x;
    int32_t y;
    int32_t w;
    int32_t h;
};

struct taa_ui_style_s
{
    const taa_font* font;
    taa_ui_halign halign;
    taa_ui_valign valign;
    int32_t lmargin;
    int32_t rmargin;
    int32_t tmargin;
    int32_t bmargin;
    int32_t lborder;
    int32_t rborder;
    int32_t tborder;
    int32_t bborder;
    int32_t lpadding;
    int32_t rpadding;
    int32_t tpadding;
    int32_t bpadding;
    int32_t defaultw;
    int32_t defaulth;
};

struct taa_ui_stylesheet_s
{
    taa_ui_style* styles;
    uint32_t numstyles;
};

struct taa_ui_iddata_s
{
    taa_ui_datatype type;
    uint32_t id;
};

struct taa_ui_scrolldata_s
{
    taa_ui_datatype type;
    taa_ui_styleid barstyleid;
    taa_ui_rect barrect;
};

struct taa_ui_textdata_s
{
    taa_ui_datatype type;
    const char* text;
    uint32_t textlength;
};

union taa_ui_data_u
{
    taa_ui_datatype type;
    taa_ui_iddata id;
    taa_ui_scrolldata scroll;
    taa_ui_textdata text;
};

struct taa_ui_control_s
{
    taa_ui_type type;
    taa_ui_styleid styleid;
    uint32_t flags;
    /// in screen space
    taa_ui_rect rect;
    taa_ui_rect cliprect;
    taa_ui_data data;
};

struct taa_ui_controllist_s
{
    const taa_ui_control* controls;
    uint32_t numcontrols;
    int32_t viewwidth;
    int32_t viewheight;
    uint32_t caret;
    uint32_t selectstart;
    uint32_t selectlength;
    uint32_t framecounter;
};

//****************************************************************************
// functions

taa_EXTERN_C void taa_ui_begin(
    taa_ui* ui,
    const taa_window_state* winstate);

taa_EXTERN_C void taa_ui_button(
    taa_ui* ui,
    taa_ui_styleid styleid,
    const taa_ui_rect* rect,
    const char* txt,
    uint32_t* flags);

taa_EXTERN_C void taa_ui_col_begin(
    taa_ui* ui,
    int32_t w,
    taa_ui_halign halign,
    taa_ui_valign valign);

taa_EXTERN_C void taa_ui_col_end(
    taa_ui* ui);

taa_EXTERN_C void taa_ui_container_begin(
    taa_ui* ui,
    taa_ui_styleid styleid,
    const taa_ui_rect* rect,
    int32_t scrollx,
    int32_t scrolly,
    uint32_t id);

taa_EXTERN_C void taa_ui_container_end(
    taa_ui* ui,
    int32_t* childwout,
    int32_t* childhout,
    int32_t* scrollxout,
    int32_t* scrollyout,
    uint32_t* flags);

taa_EXTERN_C void taa_ui_create(
    uint32_t stacksize,
    uint32_t maxcontrols,
    uint32_t maxtext,
    taa_ui** uiout);

taa_EXTERN_C void taa_ui_destroy(
    taa_ui* ui);

taa_EXTERN_C const taa_ui_controllist* taa_ui_end(
    taa_ui* ui);

taa_EXTERN_C int32_t taa_ui_fillheight(
    taa_ui* ui,
    taa_ui_styleid styleid);

taa_EXTERN_C int32_t taa_ui_fillwidth(
    taa_ui* ui,
    taa_ui_styleid styleid);

taa_EXTERN_C uint32_t taa_ui_generateid(
    taa_ui* ui);

taa_EXTERN_C void taa_ui_label(
    taa_ui* ui,
    taa_ui_styleid styleid,
    const taa_ui_rect* rect,
    const char* txt);

taa_EXTERN_C void taa_ui_row_begin(
    taa_ui* ui,
    int32_t h,
    taa_ui_halign halign,
    taa_ui_valign valign);

taa_EXTERN_C void taa_ui_row_end(
    taa_ui* ui);

taa_EXTERN_C void taa_ui_setfocus(
    taa_ui* ui,
    int32_t x,
    int32_t y);

taa_EXTERN_C void taa_ui_setstylesheet(
    taa_ui* ui,
    const taa_ui_stylesheet* styles);

taa_EXTERN_C void taa_ui_sizecols(
    taa_ui* ui,
    const taa_ui_styleid* styleids,
    int32_t* widthsin,
    uint32_t numcols,
    int32_t* widthsout);

taa_EXTERN_C void taa_ui_sizerows(
    taa_ui* ui,
    const taa_ui_styleid* styleids,
    int32_t* heightsin,
    uint32_t numrows,
    int32_t* heightsout);

taa_EXTERN_C void taa_ui_table_begin(
    taa_ui* ui,
    const taa_ui_rect* rect);

taa_EXTERN_C void taa_ui_table_end(
    taa_ui* ui);

taa_EXTERN_C void taa_ui_textbox(
    taa_ui* ui,
    taa_ui_styleid styleid,
    const taa_ui_rect* rect,
    char* txt,
    uint32_t txtsize,
    uint32_t* flags);

taa_EXTERN_C void taa_ui_vscrollbar(
    taa_ui* ui,
    taa_ui_styleid panestyleid,
    taa_ui_styleid barstyleid,
    const taa_ui_rect* rect,
    int32_t datasize,
    int32_t* value,
    uint32_t* flagsout);

#endif // TAAUI_UI_H_
