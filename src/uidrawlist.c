/**
 * @brief     ui vertex buffer and command list generation implementation
 * @author    Thomas Atwood (tatwood.net)
 * @date      2011
 * @copyright unlicense / public domain
 ****************************************************************************/
#include <taa/uidrawlist.h>
#include <taa/mat44.h>
#include <taa/scalar.h>

enum
{
    taa_UIDRAWLIST_TRANSFORMSTACK_SIZE = 16
};

struct taa_ui_drawlist_s
{
    taa_mat44 transformstack[taa_UIDRAWLIST_TRANSFORMSTACK_SIZE];
    taa_ui_vertex* verts;
    taa_ui_drawlist_cmd* cmds;
    uint32_t stackdepth;
    size_t vertindex;
    size_t maxverts;
    size_t cmdindex;
    size_t maxcmds;
};

//****************************************************************************
void taa_ui_add_drawlist_rect(
    taa_ui_drawlist* drawlist,
    taa_texture2d texture,
    uint32_t color,
    int x,
    int y,
    int w,
    int h,
    const taa_ui_rect* cliprect,
    const taa_vec2* uvlt,
    const taa_vec2* uvrb)
{
    taa_ui_vertex* v;
    taa_ui_drawlist_cmd* cmd;
    v = drawlist->verts + drawlist->vertindex;
    cmd = drawlist->cmds + drawlist->cmdindex;
    if(drawlist->vertindex + 6 <= drawlist->maxverts)
    {
        int xr = x + w;
        int yb = y + h;
        if (x  < (cliprect->x + ((int) cliprect->w)) &&
            xr > cliprect->x &&
            y  < (cliprect->y + ((int) cliprect->h)) &&
            yb > cliprect->y)
        {
            // if rectangle is visible, add it to the draw call list
            if (memcmp(&cmd->texture, &texture, sizeof(texture)) != 0)
            {
                // if the texture is different than the texture from the
                // previous draw call, a new command must be generated
                size_t vboffset = drawlist->vertindex * sizeof(*v);
                if(cmd->numvertices == 0)
                {
                    cmd->texture = texture;
                    cmd->vboffset = vboffset;
                    cmd->numvertices = 0;
                }
                else if(drawlist->cmdindex+1 < drawlist->maxcmds)
                {
                    ++drawlist->cmdindex;
                    ++cmd;
                    cmd->texture = texture;
                    cmd->vboffset = vboffset;
                    cmd->numvertices = 0;
                }
                else
                {
                    // TODO: log exceeded command buffer capacity
                    assert(0);
                    cmd = NULL;
                }
            }
            if(cmd != NULL)
            {
                taa_vec2 pos0 = { (float) x, (float) y };
                taa_vec2 pos1 = { (float) (x+w), (float) (y+h) };
                taa_vec2 uv0 = *uvlt;
                taa_vec2 uv1 = *uvrb;
                // clip the rectangle
                if(cliprect->x > x)
                {
                    float s = ((float) (cliprect->x - x))/w;
                    uv0.x = taa_mix(uvlt->x, uvrb->x, s);
                    pos0.x = (float) cliprect->x;
                }
                if(cliprect->y > y)
                {
                    float s = ((float) (cliprect->y - y))/h;
                    uv0.y = taa_mix(uvlt->y, uvrb->y, s);
                    pos0.y = (float) cliprect->y;
                }
                if(((int32_t) (cliprect->x + cliprect->w)) < xr)
                {
                    float s = ((float) (xr - (cliprect->x+cliprect->w)))/w;
                    uv1.x = taa_mix(uvrb->x, uvlt->x, s);
                    pos1.x = (float) (cliprect->x + cliprect->w);
                }
                if(((int32_t) (cliprect->y + cliprect->h)) < yb)
                {
                    float s = ((float) (yb - (cliprect->y+cliprect->h)))/h;
                    uv1.y = taa_mix(uvrb->y, uvlt->y, s);
                    pos1.y = (float) (cliprect->y + cliprect->h);
                }
                // create vertices for two ccw triangles.
                taa_vec2_set(pos0.x, pos0.y, &v->pos);
                taa_vec2_set( uv0.x,  uv0.y, &v->uv);
                v->color = color;
                ++v;
                taa_vec2_set(pos0.x, pos1.y, &v->pos);
                taa_vec2_set( uv0.x,  uv1.y, &v->uv);
                v->color = color;
                ++v;
                taa_vec2_set(pos1.x, pos1.y, &v->pos);
                taa_vec2_set( uv1.x,  uv1.y, &v->uv);
                v->color = color;
                ++v;
                taa_vec2_set(pos1.x, pos1.y, &v->pos);
                taa_vec2_set( uv1.x,  uv1.y, &v->uv);
                v->color = color;
                ++v;
                taa_vec2_set(pos1.x, pos0.y, &v->pos);
                taa_vec2_set( uv1.x,  uv0.y, &v->uv);
                v->color = color;
                ++v;
                taa_vec2_set(pos0.x, pos0.y, &v->pos);
                taa_vec2_set( uv0.x,  uv0.y, &v->uv);
                v->color = color;
                // move the buffer forwards
                cmd->numvertices += 6;
                drawlist->vertindex += 6;
            }
        }
    }
    else
    {
        // TODO: log exceeded vertex buffer capacity
        assert(0);
    }
}

//****************************************************************************
void taa_ui_add_drawlist_text(
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
    const taa_ui_rect* cliprect)
{
    taa_ui_drawlist_cmd* cmd;
    taa_ui_vertex* v;
    cmd = drawlist->cmds + drawlist->cmdindex;
    v = drawlist->verts + drawlist->vertindex;
    // get a draw call command pointer
    if (memcmp(&cmd->texture, &font->texture, sizeof(font->texture)) != 0)
    {
        // if the font texture is different than the texture from the previous
        // draw call, a new command must be generated
        size_t vboffset = drawlist->vertindex * sizeof(*v);
        if(cmd->numvertices == 0)
        {
            cmd->texture = font->texture;
            cmd->vboffset = vboffset;
            cmd->numvertices = 0;
        }
        else if(drawlist->cmdindex+1 < drawlist->maxcmds)
        {
            ++drawlist->cmdindex;
            ++cmd;
            cmd->texture = font->texture;
            cmd->vboffset = vboffset;
            cmd->numvertices = 0;
        }
        else
        {
            // TODO: log exceeded command buffer capacity
            assert(0);
            cmd = NULL;
        }
    }
    if(cmd != NULL)
    {
        int txth = font->charheight;
        int n;
        // horizontally align
        switch(halign)
        {
        case taa_UI_HALIGN_CENTER:
            {
                int txtw = taa_ui_calc_font_width(font, txt, txtlen);
                if(txtw <= w)
                {
                    int xoff = ((w - txtw) >> 1);
                    x += xoff;
                    w -= xoff;
                }
                else
                {
                    scrollx += ((txtw - w) >> 1);
                }
            }
            break;
        case taa_UI_HALIGN_LEFT:
            break;
        case taa_UI_HALIGN_RIGHT:
            {
                int txtw = taa_ui_calc_font_width(font, txt, txtlen);
                if(w > txtw)
                {
                    int xoff = w - txtw;
                    x += xoff;
                    w -= xoff;
                }
                else
                {
                    scrollx += txtw - w;
                }
            }
            break;
        }
        // vertically align
        switch(valign)
        {
        case taa_UI_VALIGN_CENTER:
            if(txth <= h)
            {
                int yoff =  ((h - txth) >> 1);
                y += yoff;
                h -= yoff;
            }
            else
            {
                scrolly += ((txth - h) >> 1);
            }
            break;
        case taa_UI_VALIGN_TOP:
            break;
        case taa_UI_VALIGN_BOTTOM:
            if(h > txth)
            {
                int yoff = h - txth;
                y += yoff;
                h -= yoff;
            }
            else
            {
                scrolly += txth - h;
            }
            break;
        }
        // clip the text
        if(x-scrollx+w > cliprect->x+cliprect->w)
        {
            w =  cliprect->x + cliprect->w - (x-scrollx);
        }
        if(x < cliprect->x)
        {
            scrollx += cliprect->x - x;
            w -= cliprect->x - x;
            x = cliprect->x;
        }
        if(y-scrolly+h > cliprect->y+cliprect->h)
        {
            h =  cliprect->y + cliprect->h - (y-scrolly);
        }
        if(y < cliprect->y)
        {
            scrolly += cliprect->y - y;
            h -= cliprect->y - y;
            y = cliprect->y;
        }
        // generate vertices
        n = taa_ui_gen_font_vertices(
            font,
            txt,
            txtlen,
            x,
            y,
            w,
            h,
            scrollx,
            scrolly,
            color,
            v,
            drawlist->maxverts - drawlist->vertindex);
        cmd->numvertices += n;
        drawlist->vertindex += n;
    }
}

//****************************************************************************
void taa_ui_begin_drawlist(
    taa_ui_drawlist* drawlist,
    taa_ui_drawlist_cmd* cmds,
    size_t maxcmds,
    taa_ui_vertex* verts,
    size_t maxverts)
{
    drawlist->cmds = cmds;
    drawlist->maxcmds = maxcmds;
    drawlist->verts = verts;
    drawlist->vertindex = 0;
    drawlist->maxverts = maxverts;
    drawlist->cmdindex = 0;
    drawlist->cmds->numvertices = 0;
}

//****************************************************************************
void taa_ui_create_drawlist(
    taa_ui_drawlist** drawlist_out)
{
    taa_ui_drawlist* drawlist;
    drawlist = (taa_ui_drawlist*) taa_memalign(16, sizeof(*drawlist));
    // initialize struct
    drawlist->cmds = NULL;
    drawlist->verts = NULL;
    drawlist->stackdepth = 0;
    drawlist->cmdindex = 0;
    drawlist->maxcmds = 0;
    drawlist->vertindex = 0;
    drawlist->maxverts = 0;
    taa_mat44_identity( drawlist->transformstack + 0);
    // set out param
    *drawlist_out =  drawlist;
}

//****************************************************************************
void taa_ui_destroy_drawlist(
    taa_ui_drawlist* drawlist)
{
    taa_memalign_free(drawlist);
}

//****************************************************************************
void taa_ui_end_drawlist(
    taa_ui_drawlist* drawlist,
    size_t* numcmds_out,
    size_t* numverts_out)
{
    assert(drawlist->stackdepth == 0);
    if(drawlist->cmds[drawlist->cmdindex].numvertices > 0)
    {
        // if a command was in progress, finish it
        ++drawlist->cmdindex;
    }
    *numcmds_out = drawlist->cmdindex;
    *numverts_out = drawlist->vertindex;
}

//****************************************************************************
void taa_ui_pop_drawlist_transform(
    taa_ui_drawlist* drawlist)
{
}

//****************************************************************************
void taa_ui_push_drawlist_transform(
    taa_ui_drawlist* drawlist,
    const taa_mat44* transform)
{
}
