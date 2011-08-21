/**
 * @brief     bitmap font implementation
 * @author    Thomas Atwood (tatwood.net)
 * @date      2011
 * @copyright unlicense / public domain
 ****************************************************************************/
#include <taaui/font.h>
#include <taa/error.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>

//****************************************************************************
void taa_font_create(
    taa_font* fontout)
{
    memset(fontout, 0, sizeof(*fontout));
    taa_texture2d_create(&fontout->texture);
}

//****************************************************************************
void taa_font_destroy(
    taa_font* font)
{
    taa_texture2d_destroy(font->texture);
}

//****************************************************************************
uint32_t taa_font_countvertices(
    const char* txt,
    uint32_t txtlen)
{
    return txtlen * 6;
}

//****************************************************************************
uint32_t taa_font_genvertices(
    const taa_font* font,
    const char* txt,
    uint32_t txtlen,
    int32_t x,
    int32_t y,
    uint32_t w,
    uint32_t h,
    int32_t scrollx,
    int32_t scrolly,
    uint32_t color,
    taa_font_vertex* vertsout,
    uint32_t vertcapacity)
{
    uint32_t vertcount = 0;
    const char* c = txt;
    const char* end = c + txtlen;
    float ymin = (float) y;
    float ymax = (float) (y + h);
    float yt = (float) (y - scrolly);
    float yb = yt + font->charheight;

    if(yb > ymin)
    {
        float xmin = (float) x;
        float xmax = (float) (x + w);
        float vtoff = 0.0f;
        float vboff = 0.0f;
        int32_t xend = x + w;
        int32_t cx = x - scrollx;
        if(yt < ymin)
        {
            vtoff = (yt - ymin)/((float) font->texheight);
            yt = ymin;
        }
        if(yb > ymax)
        {
            vboff = (yb - ymax)/((float) font->texheight);
            yb = ymax;
        }

        while(c != end && cx < xend)
        {
            const taa_font_char* fc = &font->characters[(uint8_t) *c];
            float xl = (float) cx;
            float xr = (float) (cx + fc->width);
            if(xr > xmin)
            {
                float ul = 0.0f;
                float ur = 0.0f;
                float vt = vtoff + fc->uv0.y;
                float vb = vboff + fc->uv1.y;
                if(xl < xmin)
                {
                    ul = (xmin - xl)/((float) font->texwidth);
                    xl = xmin;
                }
                if(xr > xmax)
                {
                    ur = (xmax - xr)/((float) font->texwidth);
                    xr = xmax;
                }
                ul += fc->uv0.x;
                ur += fc->uv1.x;

                if((vertcount+6) <= vertcapacity)
                {
                    taa_vec2_set(xl, yt, &vertsout->pos);
                    taa_vec2_set(ul, vt, &vertsout->uv);
                    vertsout->color = color;
                    ++vertsout;
                    taa_vec2_set(xl, yb, &vertsout->pos);
                    taa_vec2_set(ul, vb, &vertsout->uv);
                    vertsout->color = color;
                    ++vertsout;
                    taa_vec2_set(xr, yt, &vertsout->pos);
                    taa_vec2_set(ur, vt, &vertsout->uv);
                    vertsout->color = color;
                    ++vertsout;
                    taa_vec2_set(xr, yt, &vertsout->pos);
                    taa_vec2_set(ur, vt, &vertsout->uv);
                    vertsout->color = color;
                    ++vertsout;
                    taa_vec2_set(xl, yb, &vertsout->pos);
                    taa_vec2_set(ul, vb, &vertsout->uv);
                    vertsout->color = color;
                    ++vertsout;
                    taa_vec2_set(xr, yb, &vertsout->pos);
                    taa_vec2_set(ur, vb, &vertsout->uv);
                    vertsout->color = color;
                    ++vertsout;
                    vertcount += 6;
                }
                else
                {
                    // TODO: exceeded vertex capacity
                    assert(0);
                    break;
                }
            }
            // TODO: newline support
            cx += fc->width;
            ++c;
        }
    }

    return vertcount;
}

void taa_font_load(
    taa_font* font,
    const void* buf,
    uint32_t size)
{
    memcpy(font, buf, offsetof(taa_font, texture));
    taa_texture2d_bind(font->texture);
    taa_texture2d_setparameters(
        0,
        taa_TEXFILTER_NEAREST,
        taa_TEXFILTER_NEAREST,
        taa_TEXWRAP_CLAMP_TO_EDGE,
        taa_TEXWRAP_CLAMP_TO_EDGE);
    taa_texture2d_setimage(
        0,
        taa_TEXFORMAT_ALPHA8,
        font->texwidth,
        font->texheight,
        ((const uint8_t*) buf) + offsetof(taa_font, texture));
    taa_texture2d_unbind();
    // make sure that the texture data gets shared among all threads
    taa_render_finish();
}

uint32_t taa_font_textwidth(
    const taa_font* font,
    const char* txt,
    uint32_t txtlen)
{
    int32_t w = 0;
    const char* end = txt + txtlen;
    while(txt != end)
    {
        assert(*txt != '\0');
        w += font->characters[(uint8_t) *(txt++)].width;
    }
    return w;
}
