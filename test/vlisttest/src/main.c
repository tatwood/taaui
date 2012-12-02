#if defined(_DEBUG) && defined(_MSC_FULL_VER)
#include <crtdbg.h>
#endif

#include <taa/ui.h>
#include <taa/uirender.h>
#include <taa/glcontext.h>
#include "uitheme.h"
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void uilist(
    taa_ui* ui,
    const taa_ui_rect* rect,
    int* vscroll)
{
    static taa_ui_rect fillrect={0,0,taa_UI_WIDTH_FILL,taa_UI_HEIGHT_FILL};
    static taa_ui_rect rowrect={0,0,taa_UI_WIDTH_FILL,taa_UI_HEIGHT_AUTO};
    static taa_ui_rect scrlrect={0,0,taa_UI_WIDTH_DEFAULT,taa_UI_HEIGHT_FILL};
    taa_ui_handle scrollhandle;
    int i;
    taa_ui_push_cols(ui, taa_UI_VALIGN_TOP, 2, rect);
    taa_ui_push_container(ui,UITHEME_SCROLLPANE,0,&fillrect,0,*vscroll,-1);
    taa_ui_push_rows(ui, taa_UI_HALIGN_LEFT, 0, &fillrect);
    srand(0);
    for(i = 0; i < 200; ++i)
    {
        char lbl[40];
        int j;
        for(j = 0; j < sizeof(lbl)-1; ++j)
        {
            lbl[j] = rand() % 26 + 'a';
        }
        lbl[sizeof(lbl)-1] = '\0';
        j = i & 1;
        taa_ui_push_container(ui,UITHEME_LISTROW0+j,0,&rowrect,0,0,-1);
        taa_ui_label(ui, UITHEME_LABEL, &rowrect, lbl);
        taa_ui_pop_container(ui, NULL, NULL, NULL);
    }
    taa_ui_pop_rows(ui);
    scrollhandle = taa_ui_pop_container(ui, NULL, vscroll, NULL);
    taa_ui_vscrollbar(
        ui,
        UITHEME_SCROLLPANE,
        UITHEME_SCROLLBAR,
        0,
        &scrlrect,
        scrollhandle,
        vscroll,
        NULL);
    taa_ui_pop_cols(ui);
}

typedef struct main_win_s main_win;

struct main_win_s
{
    taa_window_display windisplay;
    taa_window win;
    taa_glcontext_display rcdisplay;
    taa_glcontext_surface rcsurface;
    taa_glcontext rc;
};

//****************************************************************************
static int main_init_window(
    main_win* mwin)
{
    int err = 0;
    int rcattribs[] =
    {
        taa_GLCONTEXT_BLUE_SIZE   ,  8,
        taa_GLCONTEXT_GREEN_SIZE  ,  8,
        taa_GLCONTEXT_RED_SIZE    ,  8,
        taa_GLCONTEXT_DEPTH_SIZE  , 24,
        taa_GLCONTEXT_STENCIL_SIZE,  8,
        taa_GLCONTEXT_NONE
    };
    taa_glcontext_config rcconfig;
    memset(mwin, 0, sizeof(*mwin));
    mwin->windisplay = taa_window_open_display();
    err = (mwin->windisplay != NULL) ? 0 : -1;
    if(err == 0)
    {
        err = taa_window_create(
            mwin->windisplay,
            "vlist test",
            720,
            405,
            taa_WINDOW_FULLSCREEN,
            &mwin->win);
    }
    if(err == 0)
    {
        mwin->rcdisplay = taa_glcontext_get_display(mwin->windisplay);
        err = (mwin->rcdisplay != NULL) ? 0 : -1;
    }
    if(err == 0)
    {
        err = (taa_glcontext_initialize(mwin->rcdisplay)) ? 0 : -1;
    };
    if(err == 0)
    {
        int numconfig = 0;
        taa_glcontext_choose_config(
            mwin->rcdisplay,
            rcattribs,
            &rcconfig,
            1,
            &numconfig);
        err = (numconfig >= 1) ? 0 : -1;
    }
    if(err == 0)
    {
        mwin->rcsurface = taa_glcontext_create_surface(
            mwin->rcdisplay,
            rcconfig,
            mwin->win);
        err = (mwin->rcsurface != 0) ? 0 : -1;
    }
    if(err == 0)
    {
        mwin->rc = taa_glcontext_create(
            mwin->rcdisplay,
            mwin->rcsurface,
            rcconfig,
            NULL,
            NULL);
        err = (mwin->rc != NULL) ? 0 : -1;
    }
    if(err == 0)
    {
        int success = taa_glcontext_make_current(
            mwin->rcdisplay,
            mwin->rcsurface,
            mwin->rc);
        err = (success) ? 0 : -1;
    }
    if(err == 0)
    {
        taa_window_show(mwin->windisplay, mwin->win, 1);
    }
    return err;
}

//****************************************************************************
static void main_close_window(
    main_win* mwin)
{
    taa_window_show(mwin->windisplay, mwin->win, 0);
    if(mwin->rc != NULL)
    {
        taa_glcontext_make_current(mwin->rcdisplay,mwin->rcsurface,NULL);
        taa_glcontext_destroy(mwin->rcdisplay, mwin->rc);
        taa_glcontext_destroy_surface(
            mwin->rcdisplay,
            mwin->win,
            mwin->rcsurface);
    }
    if(mwin->rcdisplay != NULL)
    {
        taa_glcontext_terminate(mwin->rcdisplay);
    }
    taa_window_destroy(mwin->windisplay, mwin->win);
    if(mwin->windisplay != NULL)
    {
        taa_window_close_display(mwin->windisplay);
    }
}

//****************************************************************************
void main_exec(
    main_win* mwin)
{
    enum
    {
        MAX_UI_STACK  = 64,
        MAX_UI_CTRLS  = 1024,
        MAX_UI_TEXT   = 16*1024,
        MAX_UI_DRAW   = 1024,
        MAX_UI_VERTS = 512*1024
    };
    taa_mouse_state mouse;
    taa_keyboard_state kb;
    uitheme uitheme;
    taa_ui* ui;
    taa_ui_drawlist* drawlist;
    taa_ui_drawlist_cmd* uicmds;
    taa_ui_render_data* uirnd;
    taa_ui_vertex* uiverts;
    taa_vertexbuffer vb;
    char txt[32] = { '\0' };
    int32_t vscroll = 0;
    int32_t quit = 0;
    uint32_t vw;
    uint32_t vh;

    uitheme_create(&uitheme);
    taa_ui_create(
        MAX_UI_STACK,
        MAX_UI_CTRLS,
        MAX_UI_TEXT,
        uitheme.stylesheet,
        UITHEME_NUM_STYLES,
        &ui);
    taa_ui_create_drawlist(&drawlist);
    taa_ui_create_render_data(&uirnd);
    uicmds =(taa_ui_drawlist_cmd*)taa_memalign(16,MAX_UI_DRAW*sizeof(*uicmds));
    uiverts = (taa_ui_vertex*) taa_memalign(16,MAX_UI_VERTS*sizeof(*uiverts));

    taa_vertexbuffer_create(&vb);

    taa_keyboard_query(mwin->windisplay, &kb);
    taa_mouse_query(mwin->windisplay, mwin->win, &mouse);
    taa_window_get_size(mwin->windisplay, mwin->win, &vw, &vh);
    while(!quit)
    {
        enum { MAXEVENTS = 16 };
        taa_ui_rect dfltrect={0,0,taa_UI_WIDTH_DEFAULT,taa_UI_HEIGHT_DEFAULT};
        taa_ui_rect fillrect={0,0,taa_UI_WIDTH_FILL,taa_UI_HEIGHT_FILL};
        taa_ui_rect fillwrect={0,0,taa_UI_WIDTH_FILL,taa_UI_HEIGHT_AUTO};
        taa_window_event winevents[MAXEVENTS];
        const taa_window_event* evtitr;
        const taa_window_event* evtend;
        const taa_ui_controllist* uicontrols;
        size_t numuicmds;
        size_t numuiverts;
        int numevents;
        unsigned int flags;
        numevents = taa_window_update(
            mwin->windisplay,
            mwin->win,
            winevents,
            MAXEVENTS);
        taa_keyboard_update(winevents, numevents, &kb);
        taa_mouse_update(winevents, numevents, &mouse);
        evtitr = winevents;
        evtend = evtitr + numevents;
        while(evtitr != evtend)
        {
            switch(evtitr->type)
            {
            case taa_WINDOW_EVENT_CLOSE:
                quit = 1; // true
                break;
            case taa_WINDOW_EVENT_SIZE:
                vw = evtitr->size.width;
                vh = evtitr->size.height;
                break;
            case taa_WINDOW_EVENT_KEY_DOWN:
                if(evtitr->key.keycode == taa_KEY_ESCAPE)
                {
                    quit = 1;
                }
                break;
            default:
                break;
            };
            ++evtitr;
        }
        glViewport(0, 0, vw, vh);
        glClearColor(0.0f,0.25f,0.25f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        // begin ui simulation
        taa_ui_begin(ui, vw, vh, &kb, &mouse, winevents, numevents);
        // center window
        taa_ui_push_rect(
            ui,
            taa_UI_HALIGN_CENTER,
            taa_UI_VALIGN_CENTER,
            &fillrect);
        // window
        taa_ui_push_container(ui,UITHEME_WINDOW, 0, &fillrect, 0, 0, -1);
        taa_ui_push_rows(ui, taa_UI_HALIGN_LEFT, 3, &fillrect);
        // textbox row
        taa_ui_textbox(ui, UITHEME_TEXTBOX,0,&fillwrect,txt,sizeof(txt),NULL);
        // list view row
        uilist(ui, &fillrect, &vscroll);
        // button row
        taa_ui_push_rect(ui,taa_UI_HALIGN_RIGHT,taa_UI_VALIGN_TOP,&fillwrect);
        taa_ui_button(ui, UITHEME_BUTTON, 0, &dfltrect, "Ok", &flags);
        if(flags & taa_UI_FLAG_CLICKED)
        {
            quit = 1;
        }
        taa_ui_pop_rect(ui);
        // end window
        taa_ui_pop_rows(ui);
        taa_ui_pop_container(ui, NULL, NULL, NULL);
        // end center
        taa_ui_pop_rect(ui);
        uicontrols = taa_ui_end(ui);
        // create ui draw list
        taa_ui_begin_drawlist(
            drawlist,
            uicmds,
            MAX_UI_DRAW,
            uiverts,
            MAX_UI_VERTS);
        uitheme_draw(&uitheme, uicontrols, drawlist);
        taa_ui_end_drawlist(drawlist, &numuicmds, &numuiverts);
        // render ui
        taa_vertexbuffer_bind(vb);
        taa_vertexbuffer_data(
            numuiverts * sizeof(*uiverts),
            uiverts,
            taa_BUFUSAGE_DYNAMIC_DRAW);
        taa_ui_render(uirnd, vw, vh, vb, uicmds, numuicmds);
        // flip
        taa_glcontext_swap_buffers(mwin->rcdisplay, mwin->rcsurface);
    }
    taa_vertexbuffer_destroy(vb);
    taa_memalign_free(uiverts);
    taa_memalign_free(uicmds);
    taa_ui_destroy_render_data(uirnd);
    taa_ui_destroy_drawlist(drawlist);
    taa_ui_destroy(ui);
    uitheme_destroy(&uitheme);
}

int main(int argc, const char** argv)
{
    main_win mwin;
    int err;

    err = main_init_window(&mwin);
    if(err == 0)
    {
        main_exec(&mwin);
    }
    main_close_window(&mwin);

#if defined(_DEBUG) && defined(_MSC_FULL_VER)
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
    _CrtCheckMemory();
    _CrtDumpMemoryLeaks();
#endif
    return EXIT_SUCCESS;
}
