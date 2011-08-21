/**
 * @brief     default ui theme implementation
 * @author    Thomas Atwood (tatwood.net)
 * @date      2011
 * @copyright unlicense / public domain
 ****************************************************************************/
#include <taaui/uitheme.h>
#include <assert.h>

//****************************************************************************
// internal macros

#define taa_UITHEME_U(x_, w_) (((x_)+0.5f)/(w_))
#define taa_UITHEME_V(y_, h_) (1.0f-(((y_)+0.5f)/(h_)))

//****************************************************************************
// internal typedefs

typedef struct taa_uitheme_visualskey_s taa_uitheme_visualskey;

//****************************************************************************
// internal structs

struct taa_uitheme_visualskey_s
{
    uint32_t styleid;
    uint32_t flags;
};

struct taa_uitheme_s
{
    taa_ui_stylesheet stylesheet;
    uint32_t* stylemap;
    taa_uitheme_visualskey* visualskeys;
    taa_uitheme_visuals* visuals;
    uint32_t numvisuals;
    uint32_t visualscapacity;
};

//****************************************************************************
// internal functions

//****************************************************************************
static uint32_t taa_uitheme_search(
    const taa_uitheme_visualskey* keys,
    uint32_t numkeys,
    uint32_t styleid,
    uint32_t flags)
{
    uint32_t lo = 0;
    uint32_t hi = numkeys;
    while(lo < hi)
    {
        uint32_t i = lo + ((hi-lo) >> 1);
        const taa_uitheme_visualskey* k = keys + i;
        if(k->styleid < styleid)
        {
            lo = i + 1;
        }
        else if(k->styleid == styleid && k->flags < flags)
        {
            lo = i + 1;
        }
        else
        {
            hi = i;
        }
    }
    return lo;
}

//****************************************************************************
void taa_uitheme_calcvisualrect(
    taa_texture2d texture,
    uint32_t texturew,
    uint32_t textureh,
    int32_t lx,
    int32_t ty,
    int32_t rx,
    int32_t by,
    taa_uitheme_visualrect* rectout)
{
    rectout->texture = texture;
    rectout->uvlt.x = taa_UITHEME_U(lx, texturew);
    rectout->uvlt.y = taa_UITHEME_V(ty, textureh);
    rectout->uvrb.x = taa_UITHEME_U(rx, texturew);
    rectout->uvrb.y = taa_UITHEME_V(by, textureh);
}

//****************************************************************************
void taa_uitheme_create(
    uint32_t numstyles,
    uint32_t numvisuals,
    taa_uitheme** themeout)
{
    void* p;
    taa_uitheme* theme;
    taa_ui_style* styles;
    uint32_t* map;
    taa_uitheme_visualskey* keys;
    taa_uitheme_visuals* visuals;
    // determine size of the buffer by calculating pointers relative to NULL
    p = NULL;
    theme = (taa_uitheme*) taa_ALIGN8(p);
    p = theme + 1;
    styles = (taa_ui_style*) taa_ALIGN8(p);
    p = styles + numstyles;
    map = (uint32_t*) taa_ALIGN8(p);
    p = map + numstyles;
    keys = (taa_uitheme_visualskey*) taa_ALIGN8(p);
    p = keys + numvisuals;
    visuals = (taa_uitheme_visuals*) taa_ALIGN8(p);
    p = visuals + numvisuals;
    // alloc the buffer
    p = calloc(1, (uintptr_t) p);
    // offset the pointers
    theme = (taa_uitheme*) (((uintptr_t) theme)+((uintptr_t) p));
    styles = (taa_ui_style*) (((uintptr_t) styles)+((uintptr_t) p));
    map = (uint32_t*) (((uintptr_t) map)+((uintptr_t) p));
    keys = (taa_uitheme_visualskey*) (((uintptr_t) keys)+((uintptr_t) p));
    visuals = (taa_uitheme_visuals*) (((uintptr_t) visuals)+((uintptr_t) p));
    // fill out theme struct
    theme->stylesheet.styles = styles;
    theme->stylesheet.numstyles = numstyles;
    theme->stylemap = map;
    theme->visualskeys = keys;
    theme->visuals = visuals;
    theme->visualscapacity = numvisuals;
    // initialize keys
    keys->styleid = -1;
    // set out param
    *themeout = theme;
}

//****************************************************************************
void taa_uitheme_destroy(
    taa_uitheme* theme)
{
    free(theme);
}

//****************************************************************************
void taa_uitheme_drawrect(
    const taa_ui_style* style,
    const taa_uitheme_visuals* visuals,
    const taa_ui_rect* rect,
    const taa_ui_rect* clip,
    taa_uidrawlist_builder* dlb)
{
    if((clip->w > 0) &&
       (clip->h > 0) &&
       (rect->x+rect->w >= clip->x) &&
       (rect->y+rect->h >= clip->y) &&
       (rect->x <= clip->x+clip->w) &&
       (rect->y <= clip->y+clip->h))
    {
        int32_t xl = rect->x;
        int32_t yt = rect->y;
        int32_t xr = xl + rect->w;
        int32_t yb = yt + rect->h;
        int32_t lb = style->lborder;
        int32_t rb = style->rborder;
        int32_t tb = style->tborder;
        int32_t bb = style->bborder;
        int32_t cw = rect->w - lb - rb;
        int32_t ch = rect->h - tb - bb;
        uint32_t c = visuals->bgcolor;
        if(lb > 0 && tb > 0)
        {
            // left top corner
            const taa_vec2* uvlt = &visuals->ltcorner.uvlt;
            const taa_vec2* uvrb = &visuals->ltcorner.uvrb;
            taa_texture2d tx = visuals->ltcorner.texture;
            taa_uidrawlist_addrect(dlb,tx,c,xl   ,yt   ,lb,tb,clip,uvlt,uvrb);
        }
        if(rb > 0 && tb > 0)
        {
            // right top corner
            const taa_vec2* uvlt = &visuals->rtcorner.uvlt;
            const taa_vec2* uvrb = &visuals->rtcorner.uvrb;
            taa_texture2d tx = visuals->rtcorner.texture;
            taa_uidrawlist_addrect(dlb,tx,c,xr-rb,yt   ,rb,tb,clip,uvlt,uvrb);
        }
        if(lb > 0 && bb > 0)
        {
            // left bottom corner
            const taa_vec2* uvlt = &visuals->lbcorner.uvlt;
            const taa_vec2* uvrb = &visuals->lbcorner.uvrb;
            taa_texture2d tx = visuals->lbcorner.texture;
            taa_uidrawlist_addrect(dlb,tx,c,xl   ,yb-bb,lb,bb,clip,uvlt,uvrb);
        }
        if(rb > 0 && bb > 0)
        {
            // right bottom corner
            const taa_vec2* uvlt = &visuals->rbcorner.uvlt;
            const taa_vec2* uvrb = &visuals->rbcorner.uvrb;
            taa_texture2d tx = visuals->rbcorner.texture;
            taa_uidrawlist_addrect(dlb,tx,c,xr-rb,yb-bb,rb,bb,clip,uvlt,uvrb);
        }
        if(lb > 0)
        {
            // left border
            const taa_vec2* uvlt = &visuals->lborder.uvlt;
            const taa_vec2* uvrb = &visuals->lborder.uvrb;
            taa_texture2d tx = visuals->lborder.texture;
            taa_uidrawlist_addrect(dlb,tx,c,xl   ,yt+tb,lb,ch,clip,uvlt,uvrb);
        }
        if(rb > 0)
        {
            // right border
            const taa_vec2* uvlt = &visuals->rborder.uvlt;
            const taa_vec2* uvrb = &visuals->rborder.uvrb;
            taa_texture2d tx = visuals->rborder.texture;
            taa_uidrawlist_addrect(dlb,tx,c,xr-rb,yt+tb,rb,ch,clip,uvlt,uvrb);
        }
        if(tb > 0)
        {
            // top border
            const taa_vec2* uvlt = &visuals->tborder.uvlt;
            const taa_vec2* uvrb = &visuals->tborder.uvrb;
            taa_texture2d tx = visuals->tborder.texture;
            taa_uidrawlist_addrect(dlb,tx,c,xl+lb,yt   ,cw,tb,clip,uvlt,uvrb);
        }
        if(bb > 0)
        {
            // bottom border
            const taa_vec2* uvlt = &visuals->bborder.uvlt;
            const taa_vec2* uvrb = &visuals->bborder.uvrb;
            taa_texture2d tx = visuals->bborder.texture;
            taa_uidrawlist_addrect(dlb,tx,c,xl+lb,yb-bb,cw,bb,clip,uvlt,uvrb);
        }
        if(cw > 0 && ch > 0)
        {
            // background
            const taa_vec2* uvlt = &visuals->background.uvlt;
            const taa_vec2* uvrb = &visuals->background.uvrb;
            taa_texture2d tx = visuals->background.texture;
            taa_uidrawlist_addrect(dlb,tx,c,xl+lb,yt+tb,cw,ch,clip,uvlt,uvrb);
        }
    }
}

//****************************************************************************
void taa_uitheme_drawrectcontrol(
    const taa_uitheme* theme,
    const taa_ui_controllist* ctrls,
    const taa_ui_control* ctrl,
    taa_uidrawlist_builder* dlb)
{
    const taa_ui_rect* clip = &ctrl->cliprect;
    const taa_ui_rect* rect = &ctrl->rect;
    if((clip->w > 0) &&
       (clip->h > 0) &&
       (rect->x+rect->w >= clip->x) &&
       (rect->y+rect->h >= clip->y) &&
       (rect->x <= clip->x+clip->w) &&
       (rect->y <= clip->y+clip->h))
    {
        taa_ui_styleid styleid = ctrl->styleid;
        uint32_t flags = ctrl->flags;
        const taa_ui_style* style;
        const taa_uitheme_visuals* vis;
        style = theme->stylesheet.styles + styleid;
        vis = taa_uitheme_findvisuals(theme, styleid, flags);
        taa_uitheme_drawrect(style, vis, rect, clip, dlb);
    }
}


//****************************************************************************
void taa_uitheme_drawscrollcontrol(
    const taa_uitheme* theme,
    const taa_ui_controllist* ctrls,
    const taa_ui_control* ctrl,
    taa_uidrawlist_builder* dlb)
{
    const taa_ui_rect* clip = &ctrl->cliprect;
    const taa_ui_rect* rect = &ctrl->rect;
    if((clip->w > 0) &&
       (clip->h > 0) &&
       (rect->x+rect->w >= clip->x) &&
       (rect->y+rect->h >= clip->y) &&
       (rect->x <= clip->x+clip->w) &&
       (rect->y <= clip->y+clip->h))
    {
        const taa_ui_rect* barrect = &ctrl->data.scroll.barrect;
        taa_ui_styleid styleid = ctrl->styleid;
        uint32_t flags = ctrl->flags;
        const taa_ui_style* style;
        const taa_uitheme_visuals* vis;
        style = theme->stylesheet.styles + styleid;
        vis = taa_uitheme_findvisuals(theme, styleid, flags);
        taa_uitheme_drawrect(style, vis, rect, clip, dlb);
        if(barrect->w > 0 && barrect->h > 0)
        {
            taa_ui_styleid barstyleid = ctrl->data.scroll.barstyleid;
            const taa_ui_style* barstyle;
            const taa_uitheme_visuals* barvis;
            barstyle = theme->stylesheet.styles + barstyleid;
            barvis =  taa_uitheme_findvisuals(theme, barstyleid, flags);
            taa_uitheme_drawrect(barstyle, barvis, barrect, clip, dlb);
        }
    }
}

//****************************************************************************
void taa_uitheme_drawtextcontrol(
    const taa_uitheme* theme,
    const taa_ui_controllist* ctrls,
    const taa_ui_control* ctrl,
    taa_uidrawlist_builder* dlb)
{
    const taa_ui_rect* clip = &ctrl->cliprect;
    const taa_ui_rect* rect = &ctrl->rect;
    if((clip->w > 0) &&
       (clip->h > 0) &&
       (rect->x+rect->w >= clip->x) &&
       (rect->y+rect->h >= clip->y) &&
       (rect->x <= clip->x+clip->w) &&
       (rect->y <= clip->y+clip->h))
    {
        const char* txt = ctrl->data.text.text;
        const taa_ui_style* style = theme->stylesheet.styles + ctrl->styleid;
        const taa_font* font = style->font;
        const taa_uitheme_visuals* vis;
        int32_t loffset = style->lborder + style->lpadding;
        int32_t roffset = style->rborder + style->rpadding;
        int32_t toffset = style->tborder + style->tpadding;
        int32_t boffset = style->bborder + style->bpadding;
        int32_t textx = rect->x + loffset;
        int32_t texty = rect->y + toffset;
        int32_t cw = rect->w - loffset - roffset;
        int32_t ch = rect->h - toffset - boffset;
        int32_t scrollx = 0;
        assert(ctrl->data.type == taa_UI_DATA_TEXT);
        vis = taa_uitheme_findvisuals(theme,ctrl->styleid, ctrl->flags);
        taa_uitheme_drawrect(style, vis, rect, clip, dlb);
        if(cw > 0 && ch > 0)
        {
            if(ctrl->type == taa_UI_TEXTBOX)
            {
                if((ctrl->flags & taa_UI_FLAG_FOCUS) != 0)
                {
                    // if it's an editable text box that has focus,
                    // draw caret and scroll text so that caret is visible
                    int32_t caretx;
                    int32_t caretend;
                    caretx = taa_font_textwidth(font, txt, ctrls->caret);
                    caretend = caretx + font->characters['|'].width;
                    if(caretend >= cw)
                    {
                        scrollx = cw - caretend;
                    }
                    taa_uidrawlist_addtext(
                        dlb,
                        font,
                        vis->fgcolor,
                        "|",
                        1,
                        textx + caretx,
                        texty,
                        cw,
                        ch,
                        scrollx,
                        0,
                        style->halign,
                        style->valign,
                        clip);
                }
            }
            taa_uidrawlist_addtext(
                dlb,
                font,
                vis->fgcolor,
                txt,
                ctrl->data.text.textlength,
                textx,
                texty,
                cw,
                ch,
                scrollx,
                0,
                style->halign,
                style->valign,
                clip);
        }
    }
}

//****************************************************************************
const taa_uitheme_visuals* taa_uitheme_findvisuals(
    const taa_uitheme* theme,
    uint32_t styleid,
    uint32_t flags)
{
    uint32_t i = theme->stylemap[styleid];
    const taa_uitheme_visualskey* k = theme->visualskeys + i;
    const taa_uitheme_visualskey* kend = theme->visualskeys+theme->numvisuals;
    const taa_uitheme_visuals* v = theme->visuals + i;
    const taa_uitheme_visuals* rv = theme->visuals;
    while(k < kend && (k->styleid == styleid))
    {
        if((k->flags & flags) == k->flags)
        {
            rv = v;
        }
        ++k;
        ++v;
    }
    return rv;
}

//****************************************************************************
const taa_ui_stylesheet* taa_uitheme_getstylesheet(
    const taa_uitheme* theme)
{
    return &theme->stylesheet;
}

//****************************************************************************
void taa_uitheme_setstyle(
    taa_uitheme* theme,
    uint32_t styleid,
    const taa_ui_style* srcstyle)
{
    assert(styleid < theme->stylesheet.numstyles);
    theme->stylesheet.styles[styleid] = *srcstyle;
}

//****************************************************************************
void taa_uitheme_setvisuals(
    taa_uitheme* theme,
    uint32_t styleid,
    uint32_t flags,
    const taa_uitheme_visuals* srcvisuals)
{
    taa_uitheme_visualskey* key;
    taa_uitheme_visuals* vis;
    uint32_t index;
    assert(styleid < theme->stylesheet.numstyles);
    // find the index in the settings list where this entry should be inserted
    index = taa_uitheme_search(
        theme->visualskeys,
        theme->numvisuals,
        styleid,
        flags);
    assert(index < theme->visualscapacity);
    key = theme->visualskeys + index;
    vis = theme->visuals + index;
    if(styleid != key->styleid || flags != key->flags)
    {
        // entry does not already exist
        taa_uitheme_visualskey* keyitr;
        taa_uitheme_visualskey* keyend;
        taa_uitheme_visuals* visitr;
        taa_uitheme_visuals* visend;
        uint32_t* mapitr;
        uint32_t* mapend;
        assert(theme->numvisuals < theme->visualscapacity);
        // shift everything to the right of the index over to make room
        keyitr = theme->visualskeys + theme->numvisuals;
        keyend = theme->visualskeys + index;
        while(keyitr != keyend)
        {
            *keyitr = *(keyitr - 1);
            --keyitr;
        }
        visitr = theme->visuals + theme->numvisuals;
        visend = theme->visuals + index;
        while(visitr != visend)
        {
            *visitr = *(visitr - 1);
            --visitr;
        }
        // adjust the map entry for the current styleid
        mapitr = theme->stylemap + styleid;
        if(*mapitr > index || theme->visualskeys[*mapitr].styleid != styleid)
        {
            *mapitr = index;
        }
        // move all the map indices for styleids > than the current one over
        mapitr = theme->stylemap + styleid + 1;
        mapend = theme->stylemap + theme->stylesheet.numstyles;
        while(mapitr < mapend)
        {
            *mapitr += 1;
            ++mapitr;
        }
        key->styleid = styleid;
        key->flags = flags;
        ++theme->numvisuals;
    }
    *vis = *srcvisuals;
}
