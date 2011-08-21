/**
 * @brief     ui render buffer and command list generation implementation
 * @author    Thomas Atwood (tatwood.net)
 * @date      2011
 * @copyright unlicense / public domain
 ****************************************************************************/
#include <taaui/uidrawlist.h>

struct taa_uidrawlist_builder_s
{
    taa_vertexbuffer vb[2];
    uint8_t* verts;
    taa_uidrawlist_cmd* cmds;
    uint32_t vertoffset;
    uint32_t vertssize;
    uint32_t cmdindex;
    uint32_t maxcmds;
    uint32_t writevb;
};

//****************************************************************************
void taa_uidrawlist_addrect(
    taa_uidrawlist_builder* dlb,
    taa_texture2d texture,
    uint32_t color,
    int32_t x,
    int32_t y,
    int32_t w,
    int32_t h,
    const taa_ui_rect* cliprect,
    const taa_vec2* uvlt,
    const taa_vec2* uvrb)
{
    taa_uidrawlist_vertex* v = (taa_uidrawlist_vertex*)(dlb->verts+dlb->vertoffset);
    taa_uidrawlist_cmd* cmd = dlb->cmds + dlb->cmdindex;
    if(dlb->vertoffset + 6*sizeof(v) <= dlb->vertssize)
    {
        int32_t xr = x + w;
        int32_t yb = y + h;
        if (x  < (cliprect->x + ((int32_t) cliprect->w)) &&
            xr > cliprect->x &&
            y  < (cliprect->y + ((int32_t) cliprect->h)) &&
            yb > cliprect->y)
        {
            // if rectangle is visible, add it to the draw call list
            if (memcmp(&cmd->texture, &texture, sizeof(texture)) != 0)
            {
                // if the texture is different than the texture from the
                // previous draw call, a new command must be generated
                if(cmd->numvertices == 0)
                {
                    cmd->texture = texture;
                    cmd->vboffset = dlb->vertoffset;
                    cmd->numvertices = 0;
                }
                else if(dlb->cmdindex+1 < dlb->maxcmds)
                {
                    ++dlb->cmdindex;
                    ++cmd;
                    cmd->texture = texture;
                    cmd->vboffset = dlb->vertoffset;
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
                dlb->vertoffset += 6*sizeof(*v);
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
void taa_uidrawlist_addtext(
    taa_uidrawlist_builder* dlb,
    const taa_font* font,
    uint32_t color,
    const char* txt,
    uint32_t txtlen,
    int32_t x,
    int32_t y,
    int32_t w,
    int32_t h,
    int32_t scrollx,
    int32_t scrolly,
    taa_ui_halign halign,
    taa_ui_valign valign,
    const taa_ui_rect* cliprect)
{
    taa_uidrawlist_cmd* cmd = dlb->cmds + dlb->cmdindex;
    taa_font_vertex* v = (taa_font_vertex*) (dlb->verts + dlb->vertoffset);
    // get a draw call command pointer
    if (memcmp(&cmd->texture, &font->texture, sizeof(font->texture)) != 0)
    {
        // if the font texture is different than the texture from the previous
        // draw call, a new command must be generated
        if(cmd->numvertices == 0)
        {
            cmd->texture = font->texture;
            cmd->vboffset = dlb->vertoffset;
            cmd->numvertices = 0;
        }
        else if(dlb->cmdindex+1 < dlb->maxcmds)
        {
            ++dlb->cmdindex;
            ++cmd;
            cmd->texture = font->texture;
            cmd->vboffset = dlb->vertoffset;
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
        int32_t txth = font->charheight;
        int32_t n;
        // horizontally align
        switch(halign)
        {
        case taa_UI_HALIGN_CENTER:
            {
                int32_t txtw = taa_font_textwidth(font, txt, txtlen);
                if(txtw <= w)
                {
                    x += ((w - txtw) >> 1);
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
                int32_t txtw = taa_font_textwidth(font, txt, txtlen);
                if(w > txtw)
                {
                    x += w - txtw;
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
                y = y + ((h-txth)>>1);
            }
            else
            {
                scrolly += ((txth-h)>>1);
            }
            break;
        case taa_UI_VALIGN_TOP:
            break;
        case taa_UI_VALIGN_BOTTOM:
            if(h > txth)
            {
                y += h - txth;
            }
            else
            {
                scrolly += txth - h;
            }
            break;
        }
        // clip the text
        if(x+w > cliprect->x+cliprect->w)
        {
            w =  cliprect->x + cliprect->w - x;
        }
        if(x < cliprect->x)
        {
            scrollx += cliprect->x - x;
            w -= cliprect->x - x;
            x = cliprect->x;
        }
        if(y+h > cliprect->y+cliprect->h)
        {
            h =  cliprect->y + cliprect->h - y;
        }
        if(y < cliprect->y)
        {
            scrolly += cliprect->y - y;
            h -= cliprect->y - y;
            y = cliprect->y;
        }
        // generate vertices
        n = taa_font_genvertices(
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
            (dlb->vertssize - dlb->vertoffset)/sizeof(*v));
        cmd->numvertices += n;
        dlb->vertoffset += n*sizeof(*v);
    }
}

//****************************************************************************
void taa_uidrawlist_begin(
    taa_uidrawlist_builder* dlb)
{
    taa_vertexbuffer_bind(dlb->vb[dlb->writevb]);
    dlb->verts = (uint8_t*) taa_vertexbuffer_map(taa_BUFACCESS_WRITE_ONLY);
    dlb->vertoffset = 0;
    dlb->cmdindex = 0;
    dlb->cmds->numvertices = 0;
}

//****************************************************************************
void taa_uidrawlist_createbuilder(
    uint32_t verticessize,
    uint32_t maxcmds,
    taa_uidrawlist_builder** dlbout)
{
    void* buf;
    uint32_t bufsize;
    taa_uidrawlist_builder* dlb;
    bufsize = sizeof(*dlb) + maxcmds*sizeof(*dlb->cmds);
    buf = malloc(bufsize);
    dlb = (taa_uidrawlist_builder*) buf;
    buf = dlb + 1;
    dlb->cmds = (taa_uidrawlist_cmd*) buf;
    taa_vertexbuffer_create(dlb->vb + 0);
    taa_vertexbuffer_bind(dlb->vb[0]);
    taa_vertexbuffer_data(verticessize, NULL, taa_BUFUSAGE_DYNAMIC_DRAW);
    taa_vertexbuffer_create(dlb->vb + 1);
    taa_vertexbuffer_bind(dlb->vb[1]);
    taa_vertexbuffer_data(verticessize, NULL, taa_BUFUSAGE_DYNAMIC_DRAW);
    dlb->verts = NULL;
    dlb->vertoffset = 0;
    dlb->vertssize = verticessize;
    dlb->cmdindex = 0;
    dlb->maxcmds = maxcmds;
    dlb->writevb = 0;
    *dlbout = dlb;
}

//****************************************************************************
void taa_uidrawlist_destroybuilder(
    taa_uidrawlist_builder* dlb)
{
    taa_vertexbuffer_destroy(dlb->vb[0]);
    taa_vertexbuffer_destroy(dlb->vb[1]);
    free(dlb);
}

//****************************************************************************
void taa_uidrawlist_end(
    taa_uidrawlist_builder* dlb,
    taa_uidrawlist* dlout)
{
    dlout->vb = dlb->vb[dlb->writevb];
    dlout->cmds = dlb->cmds;
    dlout->numcmds = dlb->cmdindex;
    if(dlout->cmds[dlb->cmdindex].numvertices > 0)
    {
        // if a command was in progress, finish it
        ++dlout->numcmds;
    }
    dlb->writevb ^= 1;
}
