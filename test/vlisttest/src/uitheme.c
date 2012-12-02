#include "uitheme.h"
#include "fontarial18n.h"
#include "uitexture.h"
#include <assert.h>
#include <string.h>

// TODO: make endian correct
#define UITHEME_RGBA(r_, g_, b_, a_) \
    ((r_) | ((g_) << 8) | ((b_) << 16) | ((a_) << 24))

enum
{
    UITHEME_BLACK = UITHEME_RGBA(0x00,0x00,0x00,0xff),
    UITHEME_GRAY0 = UITHEME_RGBA(0x2a,0x2a,0x2a,0xff),
    UITHEME_GRAY1 = UITHEME_RGBA(0x3f,0x3f,0x3f,0xff),
    UITHEME_GRAY2 = UITHEME_RGBA(0x55,0x55,0x55,0xff),
    UITHEME_GRAY3 = UITHEME_RGBA(0x7f,0x7f,0x7f,0xff),
    UITHEME_GRAY4 = UITHEME_RGBA(0xaa,0xaa,0xaa,0xff),
    UITHEME_GRAY5 = UITHEME_RGBA(0xd4,0xd4,0xd4,0xff),
    UITHEME_WHITE = UITHEME_RGBA(0xff,0xff,0xff,0xff),
    UITHEME_BLUE = UITHEME_RGBA(0x00,0x55,0xff,0xff),
    UITHEME_GREEN = UITHEME_RGBA(0x00,0xff,0x00,0xff),
    UITHEME_YELLOW = UITHEME_RGBA(0xff,0xff,0x00,0xff),
    UITHEME_TRANSPARENT = UITHEME_RGBA(0x00,0x00,0x00,0x00)
};

enum
{
    UITHEME_BORDER_THIN  = 2,
    UITHEME_BORDER_THICK = 4,
    UITHEME_PADDING_THIN = 2,
    UITHEME_PADDING_THICK= 5
};

//****************************************************************************
static int uitheme_calc_default_height(
    const taa_ui_style* style,
    int contenth)
{
    return
        style->tborder +
        style->tpadding +
        contenth +
        style->bpadding +
        style->bborder;
}

//****************************************************************************
static int uitheme_calc_default_width(
    const taa_ui_style* style,
    int contentw)
{
    return
        style->lborder +
        style->lpadding +
        contentw +
        style->rpadding +
        style->rborder;
}

//****************************************************************************
static void uitheme_draw_control(
    const uitheme* theme,
    const taa_ui_controllist* ctrls,
    const taa_ui_control* ctrl,
    taa_ui_drawlist* drawlist)
{
    const taa_ui_rect* clip = &ctrl->cliprect;
    const taa_ui_rect* rect = &ctrl->rect;
    const taa_ui_style* style;
    const taa_ui_visual* vis;
    taa_ui_styleid styleid;
    uint32_t flags;
    styleid = ctrl->styleid;
    flags = ctrl->flags;
    style = theme->stylesheet + ctrl->styleid;
    vis = taa_ui_find_visual(theme->visuals, styleid, flags);
    taa_ui_draw_visual_background(style, vis, rect, clip,  drawlist);
    taa_ui_draw_visual_border(style, vis, rect, clip,  drawlist);
}

//****************************************************************************
static void uitheme_draw_idcontrol(
    const uitheme* theme,
    const taa_ui_controllist* ctrls,
    const taa_ui_control* ctrl,
    taa_ui_drawlist* drawlist)
{
    const taa_ui_rect* clip = &ctrl->cliprect;
    const taa_ui_rect* rect = &ctrl->rect;
    const taa_ui_style* style;
    const taa_ui_visual* vis;
    taa_ui_styleid styleid;
    uint32_t flags;
    styleid = ctrl->styleid;
    flags = ctrl->flags;
    style = theme->stylesheet + ctrl->styleid;
    vis = taa_ui_find_visual(theme->visuals, styleid, flags);
    if(ctrl->type == taa_UI_CONTAINER_BEGIN)
    {
        // the background is drawn when container begins, so children
        // can draw over it
        taa_ui_draw_visual_background(style, vis, rect, clip,  drawlist);
    }
    else if(ctrl->type == taa_UI_CONTAINER_END)
    {
        // the border is draw when container ends, so it draws over
        // children
        taa_ui_draw_visual_border(style, vis, rect, clip,  drawlist);
    }
}

//****************************************************************************
static void uitheme_draw_scrollcontrol(
    const uitheme* theme,
    const taa_ui_controllist* ctrls,
    const taa_ui_control* ctrl,
    taa_ui_drawlist* drawlist)
{
    const taa_ui_rect* clip = &ctrl->cliprect;
    const taa_ui_rect* rect = &ctrl->rect;
    const taa_ui_rect* sldrect = &ctrl->data.scroll.sliderrect;
    const taa_ui_style* style;
    const taa_ui_visual* vis;
    taa_ui_styleid styleid;
    uint32_t flags;
    styleid = ctrl->styleid;
    flags = ctrl->flags;
    style = theme->stylesheet + ctrl->styleid;
    vis = taa_ui_find_visual(theme->visuals, styleid, flags);
    taa_ui_draw_visual_background(style, vis, rect, clip,  drawlist);
    taa_ui_draw_visual_border(style, vis, rect, clip,  drawlist);
    if(sldrect->w > 0 && sldrect->h > 0)
    {
        taa_ui_styleid sldstyleid = ctrl->data.scroll.sliderstyleid;
        const taa_ui_style* sldstyle;
        const taa_ui_visual* sldvis;
        sldstyle = theme->stylesheet + sldstyleid;
        sldvis =  taa_ui_find_visual(theme->visuals, sldstyleid, flags);
        taa_ui_draw_visual_background(sldstyle,sldvis,sldrect,clip,drawlist);
        taa_ui_draw_visual_border(sldstyle, sldvis, sldrect, clip,  drawlist);
    }
}

//****************************************************************************
static void uitheme_draw_textcontrol(
    const uitheme* theme,
    const taa_ui_controllist* ctrls,
    const taa_ui_control* ctrl,
    taa_ui_drawlist* drawlist)
{
    const taa_ui_rect* clip = &ctrl->cliprect;
    const taa_ui_rect* rect = &ctrl->rect;
    const taa_ui_style* style;
    const taa_ui_visual* vis;
    taa_ui_styleid styleid;
    uint32_t flags;
    const char* txt;
    int len;
    int caret;
    int sel;
    int sellen;
    styleid = ctrl->styleid;
    flags = ctrl->flags;
    style = theme->stylesheet + ctrl->styleid;
    vis = taa_ui_find_visual(theme->visuals, styleid, flags);
    txt = ctrl->data.text.text;
    len = ctrl->data.text.textlength;
    caret = -1;
    sel = 0;
    sellen = 0;
    if(ctrl->type == taa_UI_TEXTBOX)
    {
        if((ctrl->flags & taa_UI_FLAG_FOCUS) != 0)
        {
            caret = ctrls->caret;
            sel = ctrls->selectstart;
            sellen = ctrls->selectlength;
        }
    }
    taa_ui_draw_visual_background(style,vis,rect,clip, drawlist);
    taa_ui_draw_visual_text(
        style,
        vis,
        rect,
        clip,
        txt,
        len,
        caret,
        sel,
        sellen,
         drawlist);
    taa_ui_draw_visual_border(style,vis,rect,clip, drawlist);
}

//****************************************************************************
static void uitheme_init_texture(
    taa_texture2d texture)
{
    unsigned char offset;
    unsigned char imagetype;
    short width;
    short height;
    unsigned char bits;
    // the ui_texture data should be a 16x16 bgra tga blob
    memcpy(&offset, g_uitexture + 0, sizeof(offset));
    memcpy(&imagetype, g_uitexture + 2, sizeof(imagetype));
    memcpy(&width, g_uitexture + 12, sizeof(width));
    memcpy(&height, g_uitexture + 14, sizeof(height));
    memcpy(&bits, g_uitexture + 16, sizeof(bits));
    assert(imagetype == 2); // rgb
    assert(width == 16);
    assert(height == 16);
    assert(bits == 32);
    offset += 18; // add 18 byte tga header to pixel offset
    taa_texture2d_bind(texture);
    taa_texture2d_setparameter(taa_TEXPARAM_MAX_LEVEL, 0);
    taa_texture2d_setparameter(taa_TEXPARAM_MAG_FILTER,taa_TEXFILTER_NEAREST);
    taa_texture2d_setparameter(taa_TEXPARAM_MIN_FILTER,taa_TEXFILTER_NEAREST);
    taa_texture2d_setparameter(taa_TEXPARAM_WRAP_S,taa_TEXWRAP_CLAMP);
    taa_texture2d_setparameter(taa_TEXPARAM_WRAP_T,taa_TEXWRAP_CLAMP);
    taa_texture2d_image(
        0,
        taa_TEXFORMAT_BGRA8,
        width,
        height,
        g_uitexture + offset);
}

//****************************************************************************
static void uitheme_init_stylesheet(
    taa_ui_style* styles,
    taa_ui_font* font)
{
    taa_ui_style dflt;
    taa_ui_style* style;
    // set default style
    dflt.font = font;
    dflt.halign = taa_UI_HALIGN_LEFT;
    dflt.valign = taa_UI_VALIGN_TOP;
    dflt.lborder = UITHEME_BORDER_THIN;
    dflt.tborder = UITHEME_BORDER_THIN;
    dflt.rborder = UITHEME_BORDER_THIN;
    dflt.bborder = UITHEME_BORDER_THIN;
    dflt.lpadding = UITHEME_PADDING_THIN;
    dflt.tpadding = UITHEME_PADDING_THIN;
    dflt.rpadding = UITHEME_PADDING_THIN;
    dflt.bpadding = UITHEME_PADDING_THIN;
    dflt.defaultw = uitheme_calc_default_width(&dflt, 0);
    dflt.defaulth = uitheme_calc_default_height(&dflt, font->charheight);
    // button style
    style = styles + UITHEME_BUTTON;
    *style = dflt;
    style->halign = taa_UI_VALIGN_CENTER;
    style->valign = taa_UI_VALIGN_CENTER;
    style->defaultw = uitheme_calc_default_width(style,font->maxcharwidth*6);
    style->defaulth = uitheme_calc_default_height(style, font->charheight);
    // label style
    style = styles + UITHEME_LABEL;
    *style = dflt;
    style->lborder = 0;
    style->tborder = 0;
    style->rborder = 0;
    style->bborder = 0;
    style->defaultw = uitheme_calc_default_width(style,font->maxcharwidth*16);
    style->defaulth = uitheme_calc_default_height(style, font->charheight);
    // centered label style
    style = styles + UITHEME_LABEL_CENTER;
    *style = styles[UITHEME_LABEL];
    style->halign = taa_UI_HALIGN_CENTER;
    style->valign = taa_UI_VALIGN_CENTER;
    // listrow style
    style = styles + UITHEME_LISTROW0;
    *style = dflt;
    style->lborder = 0;
    style->tborder = 0;
    style->rborder = 0;
    style->bborder = 0;
    style->lpadding = 0;
    style->tpadding = 0;
    style->rpadding = 0;
    style->bpadding = 0;
    style->defaultw = uitheme_calc_default_width(style, 0);
    style->defaulth = uitheme_calc_default_height(style, 0);
    style = styles + UITHEME_LISTROW1;
    *style = styles[UITHEME_LISTROW0];
    // scrollbar style
    style = styles + UITHEME_SCROLLBAR;
    *style = dflt;
    // scrollpane style
    style = styles + UITHEME_SCROLLPANE;
    *style = dflt;
    style->lpadding = 0;
    style->tpadding = 0;
    style->rpadding = 0;
    style->bpadding = 0;
    style->defaultw = uitheme_calc_default_width(style, 18);
    style->defaulth = uitheme_calc_default_height(style, font->charheight);
    // textbox style
    style = styles + UITHEME_TEXTBOX;
    *style = dflt;
    style->defaultw = uitheme_calc_default_width(style,font->maxcharwidth*32);
    style->defaulth = uitheme_calc_default_height(style, font->charheight);
    // window style
    style = styles + UITHEME_WINDOW;
    *style = dflt;
    style->lborder = UITHEME_BORDER_THICK;
    style->tborder = UITHEME_BORDER_THICK;
    style->rborder = UITHEME_BORDER_THICK;
    style->bborder = UITHEME_BORDER_THICK;
    style->lpadding = UITHEME_PADDING_THICK;
    style->tpadding = UITHEME_PADDING_THICK;
    style->rpadding = UITHEME_PADDING_THICK;
    style->bpadding = UITHEME_PADDING_THICK;
    style->defaultw = uitheme_calc_default_width(style,512);
    style->defaulth = uitheme_calc_default_height(style,512);
}

//****************************************************************************
static void uitheme_init_visuals(
    taa_ui_visual_map* visuals,
    taa_texture2d texture)
{
    taa_ui_visual vis;
    taa_ui_visual noborder;
    taa_ui_visual dropborder;
    taa_ui_visual flatborder;
    taa_ui_visual thickborder;
    taa_ui_visual_rect rect;
    uint32_t flags;
    // create default visual for controls with_out a border
    noborder.fgcolor     = UITHEME_GRAY5;
    noborder.bgcolor     = UITHEME_GRAY3;
    noborder.bordercolor = UITHEME_TRANSPARENT;
    taa_ui_calc_visual_rect(texture,16,16, 5, 5, 5, 5,&rect);
    noborder.background = rect;
    noborder.lborder = rect;
    noborder.rborder = rect;
    noborder.tborder = rect;
    noborder.bborder = rect;
    noborder.ltcorner = rect;
    noborder.rtcorner = rect;
    noborder.lbcorner = rect;
    noborder.rbcorner = rect;
    // create default visual for controls with a drop border
    dropborder.fgcolor     = UITHEME_GRAY5;
    dropborder.bgcolor     = UITHEME_GRAY2;
    dropborder.bordercolor = UITHEME_GRAY3;
    taa_ui_calc_visual_rect(texture,16,16, 5, 5, 5, 5,&dropborder.background);
    taa_ui_calc_visual_rect(texture,16,16, 8, 6,13, 6,&dropborder.lborder);
    taa_ui_calc_visual_rect(texture,16,16,13, 6, 8, 6,&dropborder.rborder);
    taa_ui_calc_visual_rect(texture,16,16,14, 0,14, 5,&dropborder.tborder);
    taa_ui_calc_visual_rect(texture,16,16,14, 5,14, 0,&dropborder.bborder);
    taa_ui_calc_visual_rect(texture,16,16, 8, 0,13, 5,&dropborder.ltcorner);
    taa_ui_calc_visual_rect(texture,16,16,13, 0, 8, 5,&dropborder.rtcorner);
    taa_ui_calc_visual_rect(texture,16,16, 8, 5,13, 0,&dropborder.lbcorner);
    taa_ui_calc_visual_rect(texture,16,16,13, 5, 8, 0,&dropborder.rbcorner);
    // create default visual for controls with a flat border
    flatborder.fgcolor     = UITHEME_GRAY5;
    flatborder.bgcolor     = UITHEME_GRAY3;
    flatborder.bordercolor = UITHEME_GRAY1;
    taa_ui_calc_visual_rect(texture,16,16, 5, 5, 5, 5,&flatborder.background);
    taa_ui_calc_visual_rect(texture,16,16, 0, 4, 3, 4,&flatborder.lborder);
    taa_ui_calc_visual_rect(texture,16,16, 3, 4, 0, 4,&flatborder.rborder);
    taa_ui_calc_visual_rect(texture,16,16, 4, 0, 4, 3,&flatborder.tborder);
    taa_ui_calc_visual_rect(texture,16,16, 4, 3, 4, 0,&flatborder.bborder);
    taa_ui_calc_visual_rect(texture,16,16, 0, 0, 3, 3,&flatborder.ltcorner);
    taa_ui_calc_visual_rect(texture,16,16, 3, 0, 0, 3,&flatborder.rtcorner);
    taa_ui_calc_visual_rect(texture,16,16, 0, 3, 3, 0,&flatborder.lbcorner);
    taa_ui_calc_visual_rect(texture,16,16, 3, 3, 0, 0,&flatborder.rbcorner);
    // create default visual for controls with a thick border
    thickborder.fgcolor     = UITHEME_GRAY5;
    thickborder.bgcolor     = UITHEME_GRAY3;
    thickborder.bordercolor = UITHEME_GRAY0;
    taa_ui_calc_visual_rect(texture,16,16, 5, 5, 5, 5,&thickborder.background);
    taa_ui_calc_visual_rect(texture,16,16, 0,14, 5,14,&thickborder.lborder);
    taa_ui_calc_visual_rect(texture,16,16, 5,14, 0,14,&thickborder.rborder);
    taa_ui_calc_visual_rect(texture,16,16, 6, 8, 6,13,&thickborder.tborder);
    taa_ui_calc_visual_rect(texture,16,16, 6,13, 6, 8,&thickborder.bborder);
    taa_ui_calc_visual_rect(texture,16,16, 0, 8, 5,13,&thickborder.ltcorner);
    taa_ui_calc_visual_rect(texture,16,16, 5, 8, 0,13,&thickborder.rtcorner);
    taa_ui_calc_visual_rect(texture,16,16, 0,13, 5, 8,&thickborder.lbcorner);
    taa_ui_calc_visual_rect(texture,16,16, 5,13, 0, 8,&thickborder.rbcorner);

    // button style
    vis = flatborder;
    // default
    flags = 0;
    taa_ui_insert_visual(visuals,UITHEME_BUTTON,flags,&vis);
    // hover
    flags = taa_UI_FLAG_HOVER;
    vis.fgcolor = UITHEME_WHITE;
    vis.bordercolor = UITHEME_GRAY5;
    taa_ui_insert_visual(visuals,UITHEME_BUTTON,flags,&vis);
    // focus
    flags = taa_UI_FLAG_FOCUS;
    vis.fgcolor = UITHEME_WHITE;
    vis.bordercolor = UITHEME_BLUE;
    taa_ui_insert_visual(visuals,UITHEME_BUTTON,flags,&vis);

    // label style
    vis = noborder;
    // default
    flags = 0;
    vis.bgcolor = UITHEME_TRANSPARENT;
    taa_ui_insert_visual(visuals,UITHEME_LABEL,flags,&vis);

    // centered label style
    vis = noborder;
    // default
    flags = 0;
    vis.bgcolor = UITHEME_TRANSPARENT;
    taa_ui_insert_visual(visuals,UITHEME_LABEL_CENTER,flags,&vis);

    // listrow0 style
    vis = noborder;
    // default
    flags = 0;
    vis.bgcolor = UITHEME_GRAY1;
    taa_ui_insert_visual(visuals,UITHEME_LISTROW0,flags,&vis);
    // focus
    flags = taa_UI_FLAG_FOCUS;
    vis.fgcolor = UITHEME_WHITE;
    vis.bgcolor = UITHEME_BLUE;
    taa_ui_insert_visual(visuals,UITHEME_LISTROW0,flags,&vis);

    // listrow1 style
    vis = noborder;
    // default
    flags = 0;
    vis.bgcolor = UITHEME_GRAY2;
    taa_ui_insert_visual(visuals,UITHEME_LISTROW1,flags,&vis);
    // focus
    flags = taa_UI_FLAG_FOCUS;
    vis.fgcolor = UITHEME_WHITE;
    vis.bgcolor = UITHEME_BLUE;
    taa_ui_insert_visual(visuals,UITHEME_LISTROW1,flags,&vis);

    // scrollbar style
    vis = flatborder;
    // default
    flags = 0;
    taa_ui_insert_visual(visuals,UITHEME_SCROLLBAR,flags,&vis);
    // hover
    flags = taa_UI_FLAG_HOVER;
    vis.bordercolor = UITHEME_GRAY5;
    taa_ui_insert_visual(visuals,UITHEME_SCROLLBAR,flags,&vis);
    // focus
    flags = taa_UI_FLAG_FOCUS;
    vis.bordercolor = UITHEME_BLUE;
    taa_ui_insert_visual(visuals,UITHEME_SCROLLBAR,flags,&vis);

    // scrollpane style
    vis = dropborder;
    // default
    flags = 0;
    taa_ui_insert_visual(visuals,UITHEME_SCROLLPANE,flags,&vis);

    // textbox style
    vis = dropborder;
    // default
    flags = 0;
    taa_ui_insert_visual(visuals,UITHEME_TEXTBOX,flags,&vis);
    // hover
    flags = taa_UI_FLAG_HOVER;
    vis.fgcolor = UITHEME_WHITE;
    vis.bordercolor = UITHEME_GRAY5;
    taa_ui_insert_visual(visuals,UITHEME_TEXTBOX,flags,&vis);
    // focus
    flags = taa_UI_FLAG_FOCUS;
    vis.fgcolor = UITHEME_WHITE;
    vis.bgcolor = UITHEME_GRAY0;
    vis.bordercolor = UITHEME_BLUE;
    taa_ui_insert_visual(visuals,UITHEME_TEXTBOX,flags,&vis);

    // window style
    vis = thickborder;
    // default
    flags = 0;
    taa_ui_insert_visual(visuals,UITHEME_WINDOW,flags,&vis);
}

//****************************************************************************
void uitheme_create(
    uitheme* theme_out)
{
    uitheme* theme = theme_out;
    taa_texture2d texture;
    taa_ui_font* font;
    taa_ui_style* styles;
    taa_ui_visual_map* visuals;
    // create the texture
    taa_texture2d_create(&texture);
    uitheme_init_texture(texture);
    theme->texture = texture;
    // create the font
    font = &theme->font;
    taa_ui_create_font(font);
    taa_ui_load_font(font, g_fontarial18n, sizeof(g_fontarial18n));
    // create the stylesheet
    styles = theme->stylesheet;
    uitheme_init_stylesheet(styles, font);
    // create the visual map
    taa_ui_create_visual_map(
        UITHEME_NUM_STYLES,
        UITHEME_NUM_STYLES*4,
        &visuals);
    uitheme_init_visuals(visuals, texture);
    theme->visuals = visuals;
}

//****************************************************************************
void uitheme_destroy(
    uitheme* theme)
{
    taa_ui_destroy_visual_map(theme->visuals);
    taa_ui_destroy_font(&theme->font);
    taa_texture2d_destroy(theme->texture);
}

//****************************************************************************
void uitheme_draw(
    const uitheme* theme,
    const taa_ui_controllist* ctrls,
    taa_ui_drawlist* drawlist)
{
    const taa_ui_control* citr = ctrls->controls;
    const taa_ui_control* cend = citr + ctrls->numcontrols;
    while(citr != cend)
    {
        const taa_ui_rect* rect = &citr->rect;
        const taa_ui_rect* clip = &citr->cliprect;
        if((clip->w > 0) &&
           (clip->h > 0) &&
           (rect->x+rect->w >= clip->x) &&
           (rect->y+rect->h >= clip->y) &&
           (rect->x <= clip->x+clip->w) &&
           (rect->y <= clip->y+clip->h))
        {
            switch(citr->data.type)
            {
            case taa_UI_DATA_ID:
                uitheme_draw_idcontrol(theme, ctrls, citr,  drawlist);
                break;
            case taa_UI_DATA_SCROLL:
                uitheme_draw_scrollcontrol(theme, ctrls, citr,  drawlist);
                break;
            case taa_UI_DATA_TEXT:
                uitheme_draw_textcontrol(theme, ctrls, citr,  drawlist);
                break;
            default:
                uitheme_draw_control(theme, ctrls, citr,  drawlist);
                break;
            }
        }
        ++citr;
    }
}
