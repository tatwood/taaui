/**
 * @brief     ui simulation header
 * @author    Thomas Atwood (tatwood.net)
 * @date      2011
 * @copyright unlicense / public domain
 ****************************************************************************/
#ifndef taa_UI_H_
#define taa_UI_H_

#include <taa/gl.h>
#include <taa/keyboard.h>
#include <taa/mouse.h>
#include <taa/vec2.h>

//****************************************************************************
// macros

#ifndef taa_UI_LINKAGE
#define taa_UI_LINKAGE
#endif

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
    taa_UI_CONTAINER_BEGIN,
    taa_UI_CONTAINER_END,
    taa_UI_LABEL,
    taa_UI_NUMBERBOX,
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
    /**
     * @brief automatically fit control width to the size of its contents
     */
    taa_UI_WIDTH_AUTO    = -1,
    /**
     * @brief determine control width using fixed size default style width
     */
    taa_UI_WIDTH_DEFAULT = -2,
    /**
     * @brief control should size itself to fill the width of its parent
     */
    taa_UI_WIDTH_FILL    = -3,
};

enum
{
    /**
     * @brief automatically fit control height to the size of its contents
     */
    taa_UI_HEIGHT_AUTO    = -1,
    /**
     * @brief determine control height using fixed size default style height
     */
    taa_UI_HEIGHT_DEFAULT = -2,
    /**
     * @brief control should size itself to fill the height of its parent
     */
    taa_UI_HEIGHT_FILL    = -3,
};

enum
{
    taa_UI_FLAG_NONE      = 0,
    taa_UI_FLAG_CLICKED   = 1 << 0, // 1
    taa_UI_FLAG_DISABLED  = 1 << 1, // 2
    taa_UI_FLAG_FOCUS     = 1 << 2, // 4
    taa_UI_FLAG_HOVER     = 1 << 3, // 8
    taa_UI_FLAG_PRESSED   = 1 << 4, // 16
};

//****************************************************************************
// typedefs

typedef enum taa_ui_datatype_e taa_ui_datatype;
typedef enum taa_ui_halign_e taa_ui_halign;
typedef enum taa_ui_type_e taa_ui_type;
typedef enum taa_ui_valign_e taa_ui_valign;

typedef unsigned int taa_ui_styleid;

/**
 * @details handles are returned from control functions so that the control
 * may be referenced later in the frame. For example, scroll controls require
 * a handle to the target container. Handles are only valid for the lifetime
 * of the frame; once taa_ui_end has been called, all existing handles are
 * invalidated.
 */
typedef uintptr_t taa_ui_handle;

typedef struct taa_ui_vertex_s taa_ui_vertex;

typedef struct taa_ui_font_char_s taa_ui_font_char;
typedef struct taa_ui_font_s taa_ui_font;

typedef struct taa_ui_rect_s taa_ui_rect;
typedef struct taa_ui_style_s taa_ui_style;
typedef struct taa_ui_iddata_s taa_ui_iddata;
typedef struct taa_ui_scrolldata_s taa_ui_scrolldata;
typedef struct taa_ui_textdata_s taa_ui_textdata;
typedef struct taa_ui_control_s taa_ui_control;
typedef struct taa_ui_controllist_s  taa_ui_controllist;

typedef struct taa_ui_s taa_ui;

//****************************************************************************
// structs

struct taa_ui_font_char_s
{
    int32_t width;
    taa_vec2 uv0;
    taa_vec2 uv1;
};

struct taa_ui_vertex_s
{
    taa_vec2 pos;
    taa_vec2 uv;
    uint32_t color;
};

struct taa_ui_font_s
{
    int32_t maxcharwidth;
    int32_t charheight;
    int32_t texwidth;
    int32_t texheight;
    taa_ui_font_char characters[256];
    taa_texture2d texture;
};

/**
 */
struct taa_ui_rect_s
{
    int32_t x;
    int32_t y;
    int32_t w;
    int32_t h;
};

struct taa_ui_style_s
{
    const taa_ui_font* font;
    taa_ui_halign halign;
    taa_ui_valign valign;
    int32_t lborder;
    int32_t tborder;
    int32_t rborder;
    int32_t bborder;
    int32_t lpadding;
    int32_t tpadding;
    int32_t rpadding;
    int32_t bpadding;
    int32_t defaultw;
    int32_t defaulth;
};

struct taa_ui_iddata_s
{
    taa_ui_datatype type;
    uint32_t id;
};

struct taa_ui_scrolldata_s
{
    taa_ui_datatype type;
    int32_t range;
    int32_t value;
    taa_ui_styleid sliderstyleid;
    taa_ui_rect sliderrect;
    taa_ui_rect sliderpane;
};

struct taa_ui_textdata_s
{
    taa_ui_datatype type;
    const char* text;
    uint32_t textlength;
    uint32_t textcapacity;
};

struct taa_ui_control_s
{
    taa_ui_type type;
    taa_ui_styleid styleid;
    uint32_t flags;
    /**
     * @details unclipped control rectangle in screen space
     */
    taa_ui_rect rect;
    /**
     * @details clipped control rectangle in screen space
     */
    taa_ui_rect cliprect;
    union
    {
        taa_ui_datatype type;
        taa_ui_iddata id;
        taa_ui_scrolldata scroll;
        taa_ui_textdata text;
    } data;
};

struct taa_ui_controllist_s
{
    const taa_ui_control* controls;
    uint32_t numcontrols;
    int32_t viewwidth;
    int32_t viewheight;
    int32_t caret;
    uint32_t selectstart;
    uint32_t selectlength;
};

//****************************************************************************
// font functions

taa_UI_LINKAGE size_t taa_ui_count_font_vertices(
    const char* text,
    size_t txtlen);

taa_UI_LINKAGE void taa_ui_create_font(
    taa_ui_font* font_out);

taa_UI_LINKAGE void taa_ui_destroy_font(
    taa_ui_font* font);

/**
 * @details a list of font verts as triangles
 * @return the number of vertices written to the buffer
 */
taa_UI_LINKAGE size_t taa_ui_gen_font_vertices(
    const taa_ui_font* font,
    const char* txt,
    size_t txtlen,
    int x,
    int y,
    int w,
    int h,
    int scrollx,
    int scrolly,
    uint32_t color,
    taa_ui_vertex* verts_out,
    size_t vertcapacity);

taa_UI_LINKAGE void taa_ui_load_font(
    taa_ui_font* font,
    const void* buf,
    size_t size);

taa_UI_LINKAGE int taa_ui_calc_font_width(
    const taa_ui_font* font,
    const char* txt,
    size_t txtlen);

//****************************************************************************
// ui simulation functions

/**
 * @brief begins a ui simulation frame
 * @param ui the ui context
 * @param vieww width of the viewport for the ui
 * @param viewh height of the viewport for the ui
 * @param kb the current state of the keyboard
 * @param mouse the current state of the mouse
 * @param winevents events generated by the native windowing system at the
 *        current frame
 * @param numevents number of native windowing events
 */
taa_UI_LINKAGE void taa_ui_begin(
    taa_ui* ui,
    int vieww,
    int viewh,
    const taa_keyboard_state* kb,
    const taa_mouse_state* mouse,
    const taa_window_event* winevents,
    int numevents);

taa_UI_LINKAGE taa_ui_handle taa_ui_button(
    taa_ui* ui,
    taa_ui_styleid styleid,
    unsigned int flags,
    const taa_ui_rect* rect,
    const char* txt,
    unsigned int* flags_out);

taa_UI_LINKAGE void taa_ui_create(
    size_t stacksize,
    size_t maxcontrols,
    size_t maxtext,
    taa_ui_style* stylesheet,
    size_t numstyles,
    taa_ui** ui_out);

taa_UI_LINKAGE void taa_ui_destroy(
    taa_ui* ui);

taa_UI_LINKAGE const taa_ui_controllist* taa_ui_end(
    taa_ui* ui);

taa_UI_LINKAGE unsigned int taa_ui_generate_id(
    taa_ui* ui);

taa_UI_LINKAGE taa_ui_handle taa_ui_label(
    taa_ui* ui,
    taa_ui_styleid styleid,
    const taa_ui_rect* rect,
    const char* txt);

/**
 * @brief a single line integer entry field
 */
taa_UI_LINKAGE taa_ui_handle taa_ui_numberbox(
    taa_ui* ui,
    taa_ui_styleid styleid,
    unsigned int flags,
    const taa_ui_rect* rect,
    int min,
    int max,
    int* value,
    unsigned int* flags_out);

taa_UI_LINKAGE void taa_ui_pop_cols(
    taa_ui* ui);

taa_UI_LINKAGE taa_ui_handle taa_ui_pop_container(
    taa_ui* ui,
    int* scrollx_out,
    int* scrolly_out,
    unsigned int* flags_out);

taa_UI_LINKAGE void taa_ui_pop_rect(
    taa_ui* ui);

taa_UI_LINKAGE void taa_ui_pop_rows(
    taa_ui* ui);

taa_UI_LINKAGE void taa_ui_push_cols(
    taa_ui* ui,
    taa_ui_valign valign,
    int spacing,
    const taa_ui_rect* rect);

taa_UI_LINKAGE void taa_ui_push_container(
    taa_ui* ui,
    taa_ui_styleid styleid,
    unsigned int flags,
    const taa_ui_rect* rect,
    int scrollx,
    int scrolly,
    unsigned int id);

taa_UI_LINKAGE void taa_ui_push_rect(
    taa_ui* ui,
    taa_ui_halign halign,
    taa_ui_valign valign,
    const taa_ui_rect* rect);

taa_UI_LINKAGE void taa_ui_push_rows(
    taa_ui* ui,
    taa_ui_halign halign,
    int spacing,
    const taa_ui_rect* rect);

/**
 * @brief a single line text entry field
 */
taa_UI_LINKAGE taa_ui_handle taa_ui_textbox(
    taa_ui* ui,
    taa_ui_styleid styleid,
    unsigned int flags,
    const taa_ui_rect* rect,
    char* txt,
    size_t txtsize,
    unsigned int* flags_out);

/**
 * @brief adds a vertical scroll bar to the ui simulation
 * @param ui the ui context
 * @param panestyleid the style id to apply to the bar background pane
 * @param sliderstyleid the style id to apply to the bar slider
 * @param flags requested flags for the control
 * @param rect scroll pane rectangle, relative to parent
 * @param target handle to the container that will be affected by scroll
 * @param value input/output param representing amount vertical scroll
 * @param flags_out final computed flags for the control
 */
taa_UI_LINKAGE taa_ui_handle taa_ui_vscrollbar(
    taa_ui* ui,
    taa_ui_styleid panestyleid,
    taa_ui_styleid sliderstyleid,
    unsigned int flags,
    const taa_ui_rect* rect,
    taa_ui_handle target,
    int* value,
    unsigned int* flags_out);

#endif // taa_UI_H_

