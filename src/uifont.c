/**
 * @brief     bitmap font implementation
 * @author    Thomas Atwood (tatwood.net)
 * @date      2011
 * @copyright unlicense / public domain
 ****************************************************************************/
#include <assert.h>
#include <stddef.h>
#include <string.h>

//****************************************************************************
void taa_ui_create_font(
    taa_ui_font* font_out)
{
    memset(font_out, 0, sizeof(*font_out));
    taa_texture2d_create(&font_out->texture);
}

//****************************************************************************
void taa_ui_destroy_font(
    taa_ui_font* font)
{
    taa_texture2d_destroy(font->texture);
}

//****************************************************************************
size_t taa_ui_count_font_vertices(
    const char* txt,
    size_t txtlen)
{
    return txtlen * 6;
}

//****************************************************************************
size_t taa_ui_gen_font_vertices(
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
    size_t vertcapacity)
{
    size_t vertcount = 0;
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
        int xend = x + w;
        int cx = x - scrollx;
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
            const taa_ui_font_char* fc = &font->characters[(uint8_t) *c];
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
                    taa_vec2_set(xl, yt, &verts_out->pos);
                    taa_vec2_set(ul, vt, &verts_out->uv);
                    verts_out->color = color;
                    ++verts_out;
                    taa_vec2_set(xl, yb, &verts_out->pos);
                    taa_vec2_set(ul, vb, &verts_out->uv);
                    verts_out->color = color;
                    ++verts_out;
                    taa_vec2_set(xr, yt, &verts_out->pos);
                    taa_vec2_set(ur, vt, &verts_out->uv);
                    verts_out->color = color;
                    ++verts_out;
                    taa_vec2_set(xr, yt, &verts_out->pos);
                    taa_vec2_set(ur, vt, &verts_out->uv);
                    verts_out->color = color;
                    ++verts_out;
                    taa_vec2_set(xl, yb, &verts_out->pos);
                    taa_vec2_set(ul, vb, &verts_out->uv);
                    verts_out->color = color;
                    ++verts_out;
                    taa_vec2_set(xr, yb, &verts_out->pos);
                    taa_vec2_set(ur, vb, &verts_out->uv);
                    verts_out->color = color;
                    ++verts_out;
                    vertcount += 6;
                }
                else
                {
                    // TODO: log warning; exceeded vertex capacity
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

//****************************************************************************
// TODO: remove. opaque file format processing should not be here
void taa_ui_load_font(
    taa_ui_font* font,
    const void* buf,
    size_t size)
{
    memcpy(font, buf, offsetof(taa_ui_font, texture));
    taa_texture2d_bind(font->texture);
    taa_texture2d_setparameter(taa_TEXPARAM_MAX_LEVEL, 0);
    taa_texture2d_setparameter(taa_TEXPARAM_MAG_FILTER,taa_TEXFILTER_NEAREST);
    taa_texture2d_setparameter(taa_TEXPARAM_MIN_FILTER,taa_TEXFILTER_NEAREST);
    taa_texture2d_setparameter(taa_TEXPARAM_WRAP_S,taa_TEXWRAP_CLAMP);
    taa_texture2d_setparameter(taa_TEXPARAM_WRAP_T,taa_TEXWRAP_CLAMP);
    taa_texture2d_image(
        0,
        taa_TEXFORMAT_ALPHA8,
        font->texwidth,
        font->texheight,
        ((const uint8_t*) buf) + offsetof(taa_ui_font, texture));
    taa_texture2d_bind(0);
}

//****************************************************************************
int taa_ui_calc_font_width(
    const taa_ui_font* font,
    const char* txt,
    size_t txtlen)
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
