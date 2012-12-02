/**
 * @brief     win32 tool for generating bitmap fonts
 * @author    Thomas Atwood (tatwood.net)
 * @date      2011
 * @copyright unlicense / public domain
 ****************************************************************************/
#if !defined(NDEBUG) && defined(_MSC_FULL_VER)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <taa/ui.h>
#include <taa/filestream.h>
#include <assert.h>
#include <stdlib.h>

typedef struct taa_ui_font_file_s taa_ui_font_file;
typedef struct main_win_s main_win;

struct taa_ui_font_file_s
{
    uint32_t maxcharwidth;
    uint32_t charheight;
    uint32_t texwidth;
    uint32_t texheight;
    taa_ui_font_char characters[256];
    uint8_t* texture;
};

/**
 * @param weight 'b' for bold, 'n' for normal
 */
static void gen_win32font(
    const char* name,
    int32_t size,
    char weight,
    taa_ui_font_file* font_out)
{
    enum
    {
        CH_MIN = ' ',
        CH_MAX = '~',
        CH_COUNT = (CH_MAX - CH_MIN) + 1,
        CH_PADDING = 2
    };
    int32_t i;
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
    int32_t maxcharwidth;
    int32_t charheight;
    float aspect;
    char s[CH_COUNT];
    uint32_t* bits32;
    uint8_t* bits8;
    HFONT hfont;
    HDC hdc;
    HBITMAP hbmp;
    BITMAPINFO bmi;
    SIZE sz;
    taa_ui_font_char* chars = font_out->characters;
    for(i = 0; i < CH_COUNT; ++i)
    {
        s[i] = i + CH_MIN;
    }
    hfont = CreateFont(
        size,
        0,
        0,
        0,
        (weight == 'b' || weight == 'B') ? 700 : 0,
        0,
        0,
        0,
        ANSI_CHARSET,
        OUT_TT_ONLY_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY,
        FF_DONTCARE | DEFAULT_PITCH,
        name);
    hdc = CreateCompatibleDC(NULL);
    SelectObject(hdc, hfont);
    // determine the tallest character
    GetTextExtentPoint32(hdc, s, CH_COUNT, &sz);
    charheight = sz.cy;
    // determine how large the image should be
    width = 128;
    height = 128;
    while(1)
    {
        x = 0;
        y = 0;
        for(i = 0; i < CH_COUNT; ++i)
        {
            GetTextExtentPoint32(hdc, s + i, 1, &sz);
            if(x + sz.cx >= width)
            {
                x = 0;
                y += charheight + CH_PADDING;
            }
            x += sz.cx + CH_PADDING;
        }
        if(x >= width || (y + charheight) >= height)
        {
            // if texture isn't big enough, double its size and try again
            if(width <= height)
            {
                width *= 2;
            }
            else
            {
                height *= 2;
            }
        }
        else
        {
            break;
        }
    }
    aspect = ((float) width)/height;
    // Destroy the DC used for the size tests and create a new one. Since
    // no bitmap was assigned to the original DC, it became monochrome. A
    // bitmap was not assigned to it because the bitmap's desired size was
    // not known. Now that the bitmap size is known, a new DC will be
    // created for texture generation. The first operation performed on it
    // must be assignment of the bitmap, or it will be monochrome as well.
    DeleteDC(hdc);
    hdc = CreateCompatibleDC(NULL);
    bits32 = (uint32_t*) calloc(width * height, sizeof(*bits32));
    hbmp = CreateBitmap(width, height, 1, 32, bits32);
    SelectObject(hdc, hbmp);
    SelectObject(hdc, hfont);
    SetTextColor(hdc, 0x00FFFFFF);
    SetBkColor(hdc, 0x00000000);
    //SelectObject(hdc, GetStockObject(DC_BRUSH));
    //SetDCBrushColor(hdc, 0x00FF0000);
    //Rectangle(hdc, -1, -1, width + 1, height + 1);
    memset(chars, 0, sizeof(chars));
    maxcharwidth = 0;
    x = 0;
    y = 0;
    for(i = 0; i < CH_COUNT; ++i)
    {
        taa_ui_font_char* c = chars + s[i];
        GetTextExtentPoint32(hdc, s + i, 1, &sz);
        if(x + sz.cx >= width)
        {
            x = 0;
            y += charheight + CH_PADDING;
        }
        SelectClipRgn(hdc, NULL);
        IntersectClipRect(hdc, x, y, x + sz.cx, y + charheight);
        // draw the character to the device
        TextOut(hdc, x, y, s + i, 1);
        // fill out the character data
        c->width = sz.cx;
        c->uv0.x = ((float) x)/width;
        c->uv0.y = ((float) (height - y))/height;
        c->uv1.x = c->uv0.x + ((float) sz.cx)/width;
        c->uv1.y = c->uv0.y - ((float) charheight)/height;
        if(sz.cx > maxcharwidth)
        {
            maxcharwidth = sz.cx;
        }
        x += sz.cx + CH_PADDING;
    }
    // copy the device image to the bitmap
    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = width * height * sizeof(uint32_t);
    GetDIBits(hdc, hbmp, 0, height, bits32, &bmi, DIB_RGB_COLORS);
    // convert the 32 bit image data to 8 bits
    bits8 = (uint8_t*) bits32;
    for(i = 0, x = width*height; i < x; ++i)
    {
        bits8[i] = (uint8_t) bits32[i]; // Use the b channel.
    }
    font_out->maxcharwidth = maxcharwidth;
    font_out->charheight = charheight;
    font_out->texwidth = width;
    font_out->texheight = height;
    font_out->texture = bits8;
    // clean up
    DeleteObject(hfont);
    DeleteObject(hbmp);
    DeleteDC(hdc);
}

//****************************************************************************
int main(int argc, const char** argv)
{
    enum
    {
        ARG_NAME  = 1,
        ARG_SIZE = 2,
        ARG_WEIGHT = 3,
        ARG_OUT = 4
    };
    int err = EXIT_SUCCESS;
    if(argc == 5)
    {
        FILE* fp = fopen(argv[ARG_OUT], "wb");
        if(fp != NULL)
        {
            const char* name = argv[ARG_NAME];
            int size = 12;
            char w = tolower(*(argv[ARG_WEIGHT]));
            taa_ui_font_file font;
            taa_ui_font_char* chitr;
            taa_ui_font_char* chend;
            taa_filestream fs;

            sscanf(argv[ARG_SIZE], "%i", &size);
            gen_win32font(name, size, w, &font);

            taa_filestream_create(fp, 1024*1024, taa_FILESTREAM_WRITE, &fs);
            taa_filestream_write_i32(&fs, font.maxcharwidth);
            taa_filestream_write_i32(&fs, font.charheight);
            taa_filestream_write_i32(&fs, font.texwidth);
            taa_filestream_write_i32(&fs, font.texheight);
            chitr = font.characters;
            chend = chitr + sizeof(font.characters)/sizeof(*font.characters);
            while(chitr != chend)
            {
                taa_filestream_write_i32(&fs, chitr->width);
                taa_filestream_write_f32(&fs, chitr->uv0.x);
                taa_filestream_write_f32(&fs, chitr->uv0.y);
                taa_filestream_write_f32(&fs, chitr->uv1.x);
                taa_filestream_write_f32(&fs, chitr->uv1.y);
                ++chitr;
            }
            taa_filestream_write_i8n(
                &fs,
                font.texture,
                font.texwidth*font.texheight);
            taa_filestream_destroy(&fs);
            free(font.texture);
            fclose(fp);
        }
        else
        {
           printf("error opening file: %s\n", argv[4]);
           assert(0);
           err = EXIT_FAILURE;
        }
    }
    else
    {
        printf("usage: genfont <font name> <size> <b|n> <_out file>\n");
    }

#if defined(_DEBUG) && defined(_MSC_FULL_VER)
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
    _CrtCheckMemory();
    _CrtDumpMemoryLeaks();
#endif
    return err;
}
