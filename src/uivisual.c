#include <taa/uivisual.h>
#include <assert.h>
#include <string.h>

//****************************************************************************
// internal typedefs

typedef struct taa_ui_visual_key_s taa_ui_visual_key;

//****************************************************************************
// internal structs

struct taa_ui_visual_key_s
{
    uint32_t styleid;
    uint32_t flags;
};

struct taa_ui_visual_map_s
{
    uint32_t* stylemap;
    taa_ui_visual_key* keys;
    taa_ui_visual* visuals;
    uint32_t numstyles;
    uint32_t numvisuals;
    uint32_t capacity;
};

//****************************************************************************
static uint32_t taa_ui_visual_search(
    const taa_ui_visual_key* keys,
    uint32_t numkeys,
    uint32_t styleid,
    uint32_t flags)
{
    uint32_t lo = 0;
    uint32_t hi = numkeys;
    while(lo < hi)
    {
        uint32_t i = lo + ((hi-lo) >> 1);
        const taa_ui_visual_key* k = keys + i;
        if(k->styleid < styleid)
        {
            lo = i + 1;
        }
        else if(k->styleid == styleid && k->flags != flags)
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
void taa_ui_calc_visual_rect(
    taa_texture2d texture,
    uint32_t texturew,
    uint32_t textureh,
    int32_t lx,
    int32_t ty,
    int32_t rx,
    int32_t by,
    taa_ui_visual_rect* rect_out)
{
    rect_out->texture = texture;
    if(rx >= lx)
    {
        rx += 1;
        rect_out->width = rx - lx;
    }
    else
    {
        lx += 1;
        rect_out->width = lx - rx;
    }
    if(by >= ty)
    {
        by += 1;
        rect_out->height = by - ty;
    }
    else
    {
        ty += 1;
        rect_out->height = ty - by;
    }
    rect_out->uvlt.x = ((float) lx)/texturew;
    rect_out->uvlt.y = 1.0f - ((float) ty)/textureh;
    rect_out->uvrb.x = ((float) rx)/texturew;
    rect_out->uvrb.y = 1.0f - ((float) by)/textureh;
}

//****************************************************************************
void taa_ui_create_visual_map(
    uint32_t numstyles,
    uint32_t numvisuals,
    taa_ui_visual_map** map_out)
{
    void* buf;
    taa_ui_visual_map* map;
    uint32_t* stylemap;
    taa_ui_visual_key* keys;
    taa_ui_visual* visuals;
    // determine pointer offsets within the buffer
    buf = NULL;
    map = (taa_ui_visual_map*) buf;
    buf = map + 1;
    stylemap = (uint32_t*) taa_ALIGN_PTR(buf, 8);
    buf = stylemap + numstyles;
    keys = (taa_ui_visual_key*) taa_ALIGN_PTR(buf, 8);
    buf = keys + numvisuals;
    visuals = (taa_ui_visual*) taa_ALIGN_PTR(buf, 8);
    buf = visuals + numvisuals;
    // alloc the buffer
    buf = malloc((size_t) buf);
    // offset the pointers with buffer address
    map = (taa_ui_visual_map*) (((ptrdiff_t) map)+((ptrdiff_t) buf));
    stylemap = (uint32_t*) (((ptrdiff_t) stylemap)+((ptrdiff_t) buf));
    keys = (taa_ui_visual_key*) (((ptrdiff_t) keys)+((ptrdiff_t) buf));
    visuals = (taa_ui_visual*) (((ptrdiff_t) visuals)+((ptrdiff_t) buf));
    // initialize structs
    map->stylemap = stylemap;
    map->keys = keys;
    map->visuals = visuals;
    map->numstyles = numstyles;
    map->numvisuals = 0;
    map->capacity = numvisuals;
    memset(stylemap, 0, numstyles * sizeof(*stylemap));
    // initialize keys
    keys->styleid = -1;
    // set out param
    *map_out = map;
}

//****************************************************************************
void taa_ui_destroy_visual_map(
    taa_ui_visual_map* map)
{
    free(map);
}

//****************************************************************************
void taa_ui_draw_visual_background(
    const taa_ui_style* style,
    const taa_ui_visual* visual,
    const taa_ui_rect* rect,
    const taa_ui_rect* clip,
    taa_ui_drawlist* drawlist)
{
    uint32_t color = visual->bgcolor;
    // don't draw if background is completely transparent
    if((color & 0xff000000) != 0)
    {
        int32_t xl = rect->x;
        int32_t yt = rect->y;
        int32_t lb = style->lborder;
        int32_t rb = style->rborder;
        int32_t tb = style->tborder;
        int32_t bb = style->bborder;
        int32_t cw = rect->w - lb - rb;
        int32_t ch = rect->h - tb - bb;
        if(cw > 0 && ch > 0)
        {
            // background
            taa_ui_add_drawlist_rect(
                 drawlist,
                visual->background.texture,
                color,
                xl+lb,
                yt+tb,
                cw,
                ch,
                clip,
                &visual->background.uvlt,
                &visual->background.uvrb);
        }
    }
}

//****************************************************************************
void taa_ui_draw_visual_border(
    const taa_ui_style* style,
    const taa_ui_visual* visual,
    const taa_ui_rect* rect,
    const taa_ui_rect* clip,
    taa_ui_drawlist* drawlist)
{
    uint32_t color = visual->bordercolor;
    // don't draw if border is completely transparent
    if((color & 0xff000000) != 0)
    {
        // whereas the background rectangle is drawn using the border settings
        // from the style, the borders are drawn using the pixel rectangles of
        // their uv coordinates. this allows the visual appearance of the
        // borders to overlap the background rectangle, enabling effects such
        // as drop shadows.
        int32_t xl = rect->x;
        int32_t yt = rect->y;
        int32_t xr = xl + rect->w;
        int32_t yb = yt + rect->h;
        int32_t lb = style->lborder;
        int32_t rb = style->rborder;
        int32_t tb = style->tborder;
        int32_t bb = style->bborder;
        int32_t lw = visual->lborder.width;
        int32_t rw = visual->rborder.width;
        int32_t th = visual->tborder.height;
        int32_t bh = visual->bborder.height;
        int32_t cw = rect->w - lw - rw;
        int32_t ch = rect->h - th - bh;
        if(lb > 0 && tb > 0)
        {
            // left top corner
            taa_ui_add_drawlist_rect(
                 drawlist,
                visual->ltcorner.texture,
                color,
                xl,
                yt,
                visual->ltcorner.width,
                visual->ltcorner.height,
                clip,
                &visual->ltcorner.uvlt,
                &visual->ltcorner.uvrb);
        }
        if(rb > 0 && tb > 0)
        {
            // right top corner
            taa_ui_add_drawlist_rect(
                 drawlist,
                visual->rtcorner.texture,
                color,
                xr - rw,
                yt,
                visual->rtcorner.width,
                visual->rtcorner.height,
                clip,
                &visual->rtcorner.uvlt,
                &visual->rtcorner.uvrb);
        }
        if(lb > 0 && bb > 0)
        {
            // left bottom corner
            taa_ui_add_drawlist_rect(
                 drawlist,
                visual->lbcorner.texture,
                color,
                xl,
                yb - bh,
                visual->lbcorner.width,
                visual->lbcorner.height,
                clip,
                &visual->lbcorner.uvlt,
                &visual->lbcorner.uvrb);
        }
        if(rb > 0 && bb > 0)
        {
            // right bottom corner
            taa_ui_add_drawlist_rect(
                 drawlist,
                visual->rbcorner.texture,
                color,
                xr - rw,
                yb - bh,
                visual->rbcorner.width,
                visual->rbcorner.height,
                clip,
                &visual->rbcorner.uvlt,
                &visual->rbcorner.uvrb);
        }
        if(lb > 0)
        {
            // left border
            taa_ui_add_drawlist_rect(
                 drawlist,
                visual->lborder.texture,
                color,
                xl,
                yt + th,
                lw,
                ch,
                clip,
                &visual->lborder.uvlt,
                &visual->lborder.uvrb);
        }
        if(rb > 0)
        {
            // right border
            taa_ui_add_drawlist_rect(
                 drawlist,
                visual->rborder.texture,
                color,
                xr - rw,
                yt + th,
                rw,
                ch,
                clip,
                &visual->rborder.uvlt,
                &visual->rborder.uvrb);
        }
        if(tb > 0)
        {
            // top border
            taa_ui_add_drawlist_rect(
                 drawlist,
                visual->tborder.texture,
                color,
                xl + lw,
                yt,
                cw,
                th,
                clip,
                &visual->tborder.uvlt,
                &visual->tborder.uvrb);
        }
        if(bb > 0)
        {
            // bottom border
            taa_ui_add_drawlist_rect(
                 drawlist,
                visual->bborder.texture,
                color,
                xl + lw,
                yb - bh,
                cw,
                bh,
                clip,
                &visual->bborder.uvlt,
                &visual->bborder.uvrb);
        }
    }
}

//****************************************************************************
void taa_ui_draw_visual_text(
    const taa_ui_style* style,
    const taa_ui_visual* visual,
    const taa_ui_rect* rect,
    const taa_ui_rect* clip,
    const char* text,
    uint32_t textlen,
    int32_t caret,
    uint32_t selectstart,
    uint32_t selectlength,
    taa_ui_drawlist* drawlist)
{
    const taa_ui_font* font = style->font;
    int32_t loffset = style->lborder + style->lpadding;
    int32_t roffset = style->rborder + style->rpadding;
    int32_t toffset = style->tborder + style->tpadding;
    int32_t boffset = style->bborder + style->bpadding;
    int32_t textx = rect->x + loffset;
    int32_t texty = rect->y + toffset;
    int32_t cw = rect->w - loffset - roffset;
    int32_t ch = rect->h - toffset - boffset;
    int32_t scrollx = 0;
    if(cw > 0 && ch > 0)
    {
        if(caret >= 0)
        {
            // if a caret exists
            // draw caret and scroll text so that caret is visible
            int32_t caretx;
            int32_t caretend;
            int32_t caretw;
            caretw = font->characters['|'].width;
            caretx = taa_ui_calc_font_width(font, text, caret);
            caretend = caretx + caretw;
            if(caretend >= cw)
            {
                scrollx = caretend - cw;
            }
            taa_ui_add_drawlist_text(
                 drawlist,
                font,
                visual->fgcolor,
                "|",
                1,
                textx + caretx - scrollx,
                texty,
                caretw,
                ch,
                0,
                0,
                style->halign,
                style->valign,
                clip);
        }
        taa_ui_add_drawlist_text(
             drawlist,
            font,
            visual->fgcolor,
            text,
            textlen,
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

//****************************************************************************
const taa_ui_visual* taa_ui_find_visual(
    taa_ui_visual_map* map,
    uint32_t styleid,
    uint32_t flags)
{
    int mapindex = map->stylemap[styleid];
    const taa_ui_visual_key* kitr = map->keys + mapindex;
    const taa_ui_visual_key* kend = map->keys + map->numvisuals;
    const taa_ui_visual* vitr = map->visuals + mapindex;
    const taa_ui_visual* result = vitr;
    while(kitr < kend && (kitr->styleid == styleid))
    {
        if((kitr->flags & flags) == kitr->flags)
        {
            result = vitr;
        }
        ++kitr;
        ++vitr;
    }
    return result;
}

//****************************************************************************
void taa_ui_insert_visual(
    taa_ui_visual_map* map,
    uint32_t styleid,
    uint32_t flags,
    const taa_ui_visual* srcvisuals)
{
    taa_ui_visual_key* key;
    taa_ui_visual* vis;
    uint32_t index;
    assert(styleid < map->numstyles);
    // find the index in the settings list where this entry should be inserted
    index = taa_ui_visual_search(
        map->keys,
        map->numvisuals,
        styleid,
        flags);
    assert(index < map->capacity);
    key = map->keys + index;
    vis = map->visuals + index;
    if(styleid != key->styleid || flags != key->flags)
    {
        // entry does not already exist
        taa_ui_visual_key* keyitr;
        taa_ui_visual_key* keyend;
        taa_ui_visual* visitr;
        taa_ui_visual* visend;
        uint32_t* mapitr;
        uint32_t* mapend;
        assert(map->numvisuals < map->capacity);
        // shift everything to the right of the index over to make room
        keyitr = map->keys + map->numvisuals;
        keyend = map->keys + index;
        while(keyitr != keyend)
        {
            *keyitr = *(keyitr - 1);
            --keyitr;
        }
        visitr = map->visuals + map->numvisuals;
        visend = map->visuals + index;
        while(visitr != visend)
        {
            *visitr = *(visitr - 1);
            --visitr;
        }
        // adjust the map entry for the current styleid
        mapitr = map->stylemap + styleid;
        if(*mapitr > index || map->keys[*mapitr].styleid != styleid)
        {
            *mapitr = index;
        }
        // move all the map indices for styleids > than the current one over
        mapitr = map->stylemap + styleid + 1;
        mapend = map->stylemap + map->numstyles;
        while(mapitr < mapend)
        {
            *mapitr += 1;
            ++mapitr;
        }
        key->styleid = styleid;
        key->flags = flags;
        ++map->numvisuals;
    }
    *vis = *srcvisuals;
}
