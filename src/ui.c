/**
 * @brief     ui simulation implementation
 * @author    Thomas Atwood (tatwood.net)
 * @date      2011
 * @copyright unlicense / public domain
 ****************************************************************************/
#include <taaui/ui.h>
#include <taa/error.h>
#include <ctype.h>
#include <string.h>

//****************************************************************************
typedef enum taa_ui_layout_e taa_ui_layout;
typedef struct taa_ui_event_s taa_ui_event;
typedef struct taa_ui_stack_s taa_ui_stack;
typedef struct taa_ui_state_s taa_ui_state;

enum taa_ui_layout_e
{
    taa_UI_LAYOUT_ABS,
    taa_UI_LAYOUT_COL,
    taa_UI_LAYOUT_ROW,
    taa_UI_LAYOUT_TABLE
};

struct taa_ui_event_s
{
    taa_window_event winevent;
    int32_t isconsumed;
};

struct taa_ui_stack_s
{
    uint32_t id;
    taa_ui_styleid styleid;
    taa_ui_halign halign;
    taa_ui_valign valign;
    taa_ui_layout layout;
    int32_t layoutoffset;
    int32_t scrollx;
    int32_t scrolly;
    uint32_t flags;
    uint32_t ctrlcursor;
    // rect around entire control, excluding margins
    taa_ui_rect rect;
    // rect around control, excluding margins, borders and padding
    taa_ui_rect contentrect;
    // rect used to clip the visible area of the control
    taa_ui_rect cliprect;
    // rect around children of the the control
    taa_ui_rect childrect;
    // rect used to clip the visible area of child controls
    taa_ui_rect childclip;
};

struct taa_ui_state_s
{
    int32_t focusx;
    int32_t focusy;
    int32_t hoverx;
    int32_t hovery;
    int32_t isscrolling;
    taa_keyboard_state kb;
    taa_mouse_state mouse;
};

struct taa_ui_s
{
    taa_ui_stylesheet stylesheet;
    // state maintained across 2 frames
    taa_ui_state state;
    taa_ui_state prevstate;
    // input events for the current frame
    uint32_t numevents;
    taa_ui_event events[32];
    // 
    int32_t viewwidth;
    int32_t viewheight;
    uint32_t idcounter;
    uint32_t caret;
    uint32_t selectstart;
    uint32_t selectlength;
    uint32_t framecounter;
    int32_t focuscaptured;
    int32_t hovercaptured;
    // control buffers
    uint32_t stacksize;
    uint32_t stackcapacity;
    uint32_t ctrlcapacity;
    uint32_t textcapacity;
    uint32_t textoffset;
    taa_ui_stack* stack;
    taa_ui_control* ctrlbuffer;
    int32_t* ctrlparent;
    uint32_t ctrlcursor;
    char* textbuffer;
    taa_ui_controllist drawlist;
};

//****************************************************************************
// static functions

//****************************************************************************
static void taa_ui_pushidcontrol(
    taa_ui* ui,
    int32_t type,
    taa_ui_styleid styleid,
    const taa_ui_rect* rect,
    const taa_ui_rect* cliprect,
    uint32_t flags,
    uint32_t id)
{
    taa_ui_control* ctrl = ui->ctrlbuffer + ui->ctrlcursor - 1;
    if(ctrl >= ui->ctrlbuffer)
    {
        ctrl->type = type;
        ctrl->styleid = styleid;
        ctrl->flags = flags;
        ctrl->rect = *rect;
        ctrl->cliprect = *cliprect;
        ctrl->data.type = taa_UI_DATA_ID;
        ctrl->data.id.id = id;
        --ui->ctrlcursor;
    }
    else
    {
        // TODO: log exceeded control limit
        assert(0);
    }
}

//****************************************************************************
static void taa_ui_pushscrollcontrol(
    taa_ui* ui,
    int32_t type,
    taa_ui_styleid styleid,
    const taa_ui_rect* rect,
    const taa_ui_rect* cliprect,
    uint32_t flags,
    taa_ui_styleid barstyleid,
    const taa_ui_rect* barrect)
{
    taa_ui_control* ctrl = ui->ctrlbuffer + ui->ctrlcursor - 1;
    if(ctrl >= ui->ctrlbuffer)
    {
        ctrl->type = type;
        ctrl->styleid = styleid;
        ctrl->flags = flags;
        ctrl->rect = *rect;
        ctrl->cliprect = *cliprect;
        ctrl->data.type = taa_UI_DATA_SCROLL;
        ctrl->data.scroll.barstyleid = barstyleid;
        ctrl->data.scroll.barrect = *barrect;
        --ui->ctrlcursor;
    }
    else
    {
        // TODO: log exceeded control limit
        assert(0);
    }
}

//****************************************************************************
static void taa_ui_pushtextcontrol(
    taa_ui* ui,
    int32_t type,
    taa_ui_styleid styleid,
    const taa_ui_rect* rect,
    const taa_ui_rect* cliprect,
    uint32_t flags,
    const char* txt,
    uint32_t txtlen)
{
    taa_ui_control* ctrl = ui->ctrlbuffer + ui->ctrlcursor - 1;
    if(ctrl >= ui->ctrlbuffer)
    {
        char* dsttxt = ui->textbuffer;
        uint32_t maxtxt = ui->textcapacity - ui->textoffset;
        txtlen = (txtlen < maxtxt) ? txtlen : maxtxt;
        ctrl->type = type;
        ctrl->styleid = styleid;
        ctrl->flags = flags;
        ctrl->rect = *rect;
        ctrl->cliprect = *cliprect;
        ctrl->data.type = taa_UI_DATA_TEXT;
        ctrl->data.text.text = dsttxt;
        ctrl->data.text.textlength = 0;
        if(cliprect->w > 0 && cliprect->h > 0)
        {
            // only consume text if the control is visible
            if((ui->textoffset+txtlen) <= ui->textcapacity)
            {
                dsttxt += ui->textoffset;
                memcpy(dsttxt, txt, txtlen);
                ctrl->data.text.text = dsttxt;
                ctrl->data.text.textlength = txtlen;
                ui->textoffset += txtlen;
            }
            else
            {
                // TODO: log exceeded text limit
                assert(0);
            }
        }
        --ui->ctrlcursor;
    }
    else
    {
        // TODO: log exceeded control limit
        assert(0);
    }
}

//****************************************************************************
static void taa_ui_rectintersect(
    const taa_ui_rect* a,
    const taa_ui_rect* b,
    taa_ui_rect* rout)
{
    int32_t xl = (a->x > b->x) ? a->x : b->x;
    int32_t xr = (a->x+a->w < b->x+b->w) ? a->x+a->w : b->x+b->w;
    int32_t yt = (a->y > b->y) ? a->y : b->y;
    int32_t yb = (a->y+a->h < b->y+b->h) ? a->y+a->h : b->y+b->h;
    rout->x = xl;
    rout->y = yt;
    rout->w = xr - xl;
    rout->h = yb - yt;
}

//****************************************************************************
static void taa_ui_rectunion(
    const taa_ui_rect* a,
    const taa_ui_rect* b,
    taa_ui_rect* rout)
{
    int32_t xl = (a->x < b->x) ? a->x : b->x;
    int32_t xr = (a->x+a->w > b->x+b->w) ? a->x+a->w : b->x+b->w;
    int32_t yt = (a->y < b->y) ? a->y : b->y;
    int32_t yb = (a->y+a->h > b->y+b->h) ? a->y+a->h : b->y+b->h;
    rout->x = xl;
    rout->y = yt;
    rout->w = xr - xl;
    rout->h = yb - yt;
}

//****************************************************************************
static int32_t taa_ui_align_x(const taa_ui_rect* r, taa_ui_halign halign)
{
    int32_t x = 0;
    switch(halign)
    {
    case taa_UI_HALIGN_CENTER:
        x = r->x + (r->w >> 1);
        break;
    case taa_UI_HALIGN_LEFT:
        x = r->x;
        break;
    case taa_UI_HALIGN_RIGHT:
        x = r->x + r->w;
        break;
    }
    return x;
}

//****************************************************************************
static int32_t taa_ui_align_y(const taa_ui_rect* r, taa_ui_valign valign)
{
    int32_t y = 0;
    switch(valign)
    {
    case taa_UI_VALIGN_CENTER:
        y = r->y + (r->h >> 1);
        break;
    case taa_UI_VALIGN_TOP:
        y = r->y;
        break;
    case taa_UI_VALIGN_BOTTOM:
        y = r->y + r->h;
        break;
    }
    return y;
}

//****************************************************************************
static void taa_ui_layoutrect(
    const taa_ui* ui,
    const taa_ui_style* style,
    const taa_ui_rect* rectin,
    taa_ui_rect* rectout)
{
    taa_ui_stack* p = ui->stack + ui->stacksize - 1;
    taa_ui_layout layout = p->layout;
    taa_ui_rect pr = p->contentrect;
    taa_ui_rect cr = p->childrect;
    taa_ui_rect lr;
    // initialize rect
    lr = *rectin;
    // check if width and height should be adjusted
    switch(lr.w)
    {
    case taa_UI_WIDTH_DEFAULT:
        lr.w = style->defaultw;
        break;
    case taa_UI_WIDTH_FILL:
        lr.w = pr.w - style->lmargin - style->rmargin;
        break;
    default:
        break;
    }
    switch(lr.h)
    {
    case taa_UI_HEIGHT_DEFAULT:
        lr.h = style->defaulth;
        break;
    case taa_UI_HEIGHT_FILL:
        lr.h = pr.h - style->tmargin - style->bmargin;
        break;
    default:
        break;
    }
    // determine x position based on alignment
    switch(p->halign)
    {
    case taa_UI_HALIGN_CENTER:
        lr.x += pr.x + ((lr.w<=pr.w) ? ((pr.w-lr.w)>>1) : -((lr.w-pr.w)>>1));
        break;
    case taa_UI_HALIGN_LEFT:
        lr.x += pr.x + style->lmargin;
        break;
    case taa_UI_HALIGN_RIGHT:
        lr.x = pr.x + pr.w - style->rmargin - lr.w - lr.x;
        break;
    }
    // determine y position based on alignment
    switch(p->valign)
    {
    case taa_UI_VALIGN_CENTER:
        lr.y += pr.y + ((lr.h<=pr.h) ? ((pr.h-lr.h)>>1) : -((lr.h-pr.h)>>1));
        break;
    case taa_UI_VALIGN_TOP:
        lr.y += pr.y + style->tmargin;
        break;
    case taa_UI_VALIGN_BOTTOM:
        lr.y = pr.y + pr.h - style->bmargin - lr.h - lr.y;
        break;
    }
    // rectangle has now been computed
    *rectout = lr;
    // adjust the parent's child rect to fit the new rect (including margins)
    lr.x -= style->lmargin;
    lr.y -= style->rmargin;
    lr.w += style->lmargin + style->rmargin;
    lr.h += style->tmargin + style->bmargin;
    taa_ui_rectunion(&cr, &lr, &p->childrect);
}

//****************************************************************************
static int32_t taa_ui_testrect(
    int32_t x,
    int32_t y,
    const taa_ui_rect* rect)
{
    return rect->x<=x && rect->y<=y && rect->x+rect->w>x && rect->y+rect->h>y;
}

//****************************************************************************
static int32_t taa_ui_testfocus(
    const taa_ui* ui,
    const taa_ui_rect* rect)
{
    const taa_ui_state* s = &ui->state;
    return ui->focuscaptured==0 && taa_ui_testrect(s->focusx,s->focusy,rect);
}

//****************************************************************************
static int32_t taa_ui_testhover(
    const taa_ui* ui,
    const taa_ui_rect* rect)
{
    const taa_ui_state* s = &ui->state;
    return ui->hovercaptured==0 && taa_ui_testrect(s->hoverx,s->hovery,rect);
}

//****************************************************************************
static int32_t taa_ui_tryfocus(
    taa_ui* ui,
    const taa_ui_rect* rect)
{
    int32_t i = taa_ui_testfocus(ui, rect);
    ui->focuscaptured |= i;
    return i;
}

//****************************************************************************
static int32_t taa_ui_tryhover(
    taa_ui* ui,
    const taa_ui_rect* rect)
{
    int32_t i = taa_ui_testhover(ui, rect);
    ui->hovercaptured |= i;
    return i;
}

//****************************************************************************
// extern functions

//****************************************************************************
void taa_ui_begin(
    taa_ui* ui,
    const taa_window_state* winstate)
{
    const taa_window_event* evt = winstate->events;
    const taa_window_event* evtend = evt + winstate->numevents;
    taa_ui_stack* stack = ui->stack;
    taa_ui_event* uievt = ui->events;
    taa_ui_event* uievtend=uievt+(sizeof(ui->events)/sizeof(*ui->events));
    ui->prevstate = ui->state;
    ui->state.hoverx = winstate->mousestate.cursorx;
    ui->state.hovery = winstate->mousestate.cursory;
    memcpy(ui->state.kb, winstate->kbstate, sizeof(ui->state.kb));
    ui->state.mouse = winstate->mousestate;
    ui->viewwidth = winstate->width & ~1;
    ui->viewheight = winstate->height & ~1;
    ui->focuscaptured = 0;
    ui->hovercaptured = 0;
    ui->textoffset = 0;
    ui->ctrlcursor = ui->ctrlcapacity;
    while(evt != evtend)
    {
        int32_t keep = 0;
        switch(evt->type)
        {
        case taa_WINDOW_EVENT_KEY_DOWN:
            keep = 1;
            break;
        case taa_WINDOW_EVENT_KEY_UP:
            keep = 1;
            break;
        case taa_WINDOW_EVENT_MOUSE_BUTTON1_DOWN:
            ui->state.focusx = evt->mouse.state.cursorx;
            ui->state.focusy = evt->mouse.state.cursory;
            ui->state.isscrolling = 0;
            keep = 1;
            break;
        case taa_WINDOW_EVENT_MOUSE_BUTTON1_UP:
            keep = 1;
            break;
        default:
            break;
        }
        if(keep)
        {
            uievt->winevent = *evt;
            uievt->isconsumed = 0;
            ++uievt;
            if(uievt == uievtend)
            {
                break;
            }
        }
        ++evt;
    }
    ui->numevents = (uint32_t) (uievt - ui->events);
    // create an entry on the stack, so there will always be something
    // at the top level
    ui->stacksize = 1;
    memset(stack, 0, sizeof(*stack));
    stack->id = -1;
    stack->styleid = -1;
    stack->halign = taa_UI_HALIGN_LEFT;
    stack->valign = taa_UI_VALIGN_TOP;
    stack->layout = taa_UI_LAYOUT_ABS;
    stack->rect.w = ui->viewwidth;
    stack->rect.h = ui->viewheight;
    stack->contentrect = stack->rect;
    stack->cliprect = stack->rect;
    stack->childclip = stack->rect;
}

//****************************************************************************
void taa_ui_button(
    taa_ui* ui,
    taa_ui_styleid styleid,
    const taa_ui_rect* rect,
    const char* txt,
    uint32_t* flagsout)
{
    const taa_ui_stack* p = ui->stack + ui->stacksize - 1;
    const taa_ui_style* style = ui->stylesheet.styles + styleid;
    taa_ui_state* state = &ui->state;
    uint32_t flags = taa_UI_FLAG_NONE;
    taa_ui_rect scrnrect;
    taa_ui_rect cliprect;
    taa_ui_layoutrect(ui, style, rect, &scrnrect);
    taa_ui_rectintersect(&p->childclip, &scrnrect, &cliprect);
    if(taa_ui_tryhover(ui, &cliprect))
    {
        flags |= taa_UI_FLAG_HOVER;
    }
    if(taa_ui_tryfocus(ui, &cliprect))
    {
        taa_ui_event* evt = ui->events;
        taa_ui_event* evtend = evt + ui->numevents;
        int32_t cx = state->mouse.cursorx;
        int32_t cy = state->mouse.cursory;
        if(taa_ui_testrect(cx,cy,&cliprect))
        {
            if(state->mouse.button1)
            {
                flags |= taa_UI_FLAG_PRESSED;
            }
        }
        if(state->kb[taa_KEY_ENTER] != 0)
        {
            flags |= taa_UI_FLAG_PRESSED;
        }
        // process events
        while(evt != evtend)
        {
            taa_window_eventtype evttype = evt->winevent.type;
            evt->isconsumed = 1;
            if(evttype == taa_WINDOW_EVENT_KEY_DOWN)
            {
                switch(evt->winevent.key.keycode)
                {
                case taa_KEY_TAB:
                    evt->isconsumed = 0;
                    break;
                case taa_KEY_LEFT:
                    evt->isconsumed = 0;
                    break;
                case taa_KEY_UP:
                    evt->isconsumed = 0;
                    break;
                case taa_KEY_RIGHT:
                    evt->isconsumed = 0;
                    break;
                case taa_KEY_DOWN:
                    evt->isconsumed = 0;
                    break;
                case taa_KEY_ENTER:
                    flags |= taa_UI_FLAG_CLICKED;
                    break;
                default:
                    break;
                }
            }
            else if(evttype == taa_WINDOW_EVENT_MOUSE_BUTTON1_UP)
            {
                const taa_mouse_state* evtm = &evt->winevent.mouse.state;
                if(taa_ui_testrect(evtm->cursorx,evtm->cursory,&cliprect))
                {
                    flags |= taa_UI_FLAG_CLICKED;
                }
            }
            ++evt;
        }
        flags |= taa_UI_FLAG_FOCUS;
    }
    taa_ui_pushtextcontrol(
        ui,
        taa_UI_BUTTON,
        styleid,
        &scrnrect,
        &cliprect,
        flags,
        txt,
        strlen(txt));
    if(flagsout != NULL)
    {
        *flagsout = flags;
    }
}

//****************************************************************************
void taa_ui_col_begin(
    taa_ui* ui,
    int32_t w,
    taa_ui_halign halign,
    taa_ui_valign valign)
{
    taa_ui_stack* s = ui->stack + ui->stacksize;
    taa_ui_stack* p = s - 1;
    taa_CHECK_ERROR(ui->stacksize < ui->stackcapacity, "stack overflow");
    switch(w)
    {
    case taa_UI_WIDTH_DEFAULT: w = w; break;
    case taa_UI_WIDTH_FILL: w = p->contentrect.w - p->layoutoffset; break;
    default: break;
    }
    s->id = -1;
    s->styleid = -1;
    s->halign = halign;
    s->valign = valign;
    s->layout = taa_UI_LAYOUT_COL;
    s->layoutoffset = 0;
    s->scrollx = 0;
    s->scrolly = 0;
    s->flags = 0;
    s->ctrlcursor = ui->ctrlcursor;
    // row rect
    s->rect.x = p->contentrect.x + p->layoutoffset;
    s->rect.y = p->contentrect.y;
    s->rect.w = w;
    s->rect.h = p->contentrect.h;
    // content rect is row rect
    s->contentrect = s->rect;
    // cliprect is row rect clipped against parent
    taa_ui_rectintersect(&p->childclip, &s->rect, &s->cliprect);
    // initialize the child rect
    s->childrect.x = taa_ui_align_x(&s->contentrect, s->halign);
    s->childrect.y = taa_ui_align_y(&s->contentrect, s->valign);
    s->childrect.w = 0;
    s->childrect.h = 0;
    // child cliprect is child rect clipped by the row's cliprect
    taa_ui_rectintersect(&s->cliprect, &s->contentrect, &s->childclip);
    // adjust parent's childrect to contain the row
    taa_ui_rectunion(&p->childrect, &s->rect, &p->childrect);
    p->layoutoffset += w;
    ++ui->stacksize;
}

//****************************************************************************
void taa_ui_col_end(
    taa_ui* ui)
{
    taa_ui_stack* s = ui->stack + ui->stacksize - 1;
    assert(s->layout == taa_UI_LAYOUT_COL);
    assert(ui->stacksize > 1);
    --ui->stacksize;
}

//****************************************************************************
void taa_ui_container_begin(
    taa_ui* ui,
    taa_ui_styleid styleid,
    const taa_ui_rect* rect,
    int32_t scrollx,
    int32_t scrolly,
    uint32_t id)
{
    const taa_ui_stack* p = ui->stack + ui->stacksize - 1;
    const taa_ui_style* style = ui->stylesheet.styles + styleid;
    taa_ui_stack* s = ui->stack + ui->stacksize;
    int32_t loffset = style->lborder + style->lpadding;
    int32_t roffset = style->rborder + style->rpadding;
    int32_t toffset = style->tborder + style->tpadding;
    int32_t boffset = style->bborder + style->bpadding;
    taa_CHECK_ERROR(ui->stacksize < ui->stackcapacity, "stack overflow");
    s->id = id;
    s->styleid = styleid;
    s->halign = style->halign;
    s->valign = style->valign;
    s->layout = taa_UI_LAYOUT_ABS;
    s->layoutoffset = 0;
    s->scrollx = scrollx;
    s->scrolly = scrolly;
    s->flags = 0;
    s->ctrlcursor = ui->ctrlcursor;
    // control rect
    taa_ui_layoutrect(ui, style, rect, &s->rect);
    // content rect is control rect excluding border and padding
    s->contentrect.x = s->rect.x + loffset;
    s->contentrect.y = s->rect.y + toffset;
    s->contentrect.w = s->rect.w - loffset - roffset;
    s->contentrect.h = s->rect.h - toffset - boffset;
    // cliprect is control rect clipped against parent
    taa_ui_rectintersect(&p->childclip, &s->rect, &s->cliprect);
    // initialize the child rect
    s->childrect.x = taa_ui_align_x(&s->contentrect, s->halign);
    s->childrect.y = taa_ui_align_y(&s->contentrect, s->valign);
    s->childrect.w = 0;
    s->childrect.h = 0;
    // child cliprect is content rect clipped by the control's cliprect
    taa_ui_rectintersect(&s->cliprect, &s->contentrect, &s->childclip);
    // now that clip rects are calculated, scroll the content
    s->contentrect.x -= scrollx;
    s->contentrect.y -= scrolly;
    // just set focused flag, let children try to capture it
    if(taa_ui_testfocus(ui, &s->cliprect))
    {
        s->flags |= taa_UI_FLAG_FOCUS;
    }
    ++ui->stacksize;
}

//****************************************************************************
void taa_ui_container_end(
    taa_ui* ui,
    int32_t* childwout,
    int32_t* childhout,
    int32_t* scrollxout,
    int32_t* scrollyout,
    uint32_t* flagsout)
{
    uint32_t flags = taa_UI_FLAG_NONE;
    const taa_ui_stack* s = ui->stack + ui->stacksize - 1;
    taa_CHECK_ERROR(ui->stacksize > 1, "stack is empty");
    // attempt to capture focus or hover now in case children did not
    flags = s->flags;
    if(taa_ui_tryfocus(ui, &s->cliprect))
    {
        flags |= taa_UI_FLAG_FOCUS;
    }
    if(taa_ui_tryhover(ui, &s->cliprect))
    {
        flags |= taa_UI_FLAG_HOVER;
    }
    taa_ui_pushidcontrol(
        ui,
        taa_UI_CONTAINER,
        s->styleid,
        &s->rect,
        &s->cliprect,
        flags,
        s->id);
    if(childwout != NULL)
    {
        *childwout = s->childrect.w;
    }
    if(childhout != NULL)
    {
        *childhout = s->childrect.h;
    }
    if(scrollxout != NULL)
    {
        *scrollxout = s->scrollx;
    }
    if(scrollyout != NULL)
    {
        *scrollyout = s->scrolly;
    }
    if(flagsout != NULL)
    {
        *flagsout = flags;
    }
    --ui->stacksize;
}

//****************************************************************************
void taa_ui_create(
    uint32_t stacksize,
    uint32_t maxcontrols,
    uint32_t maxtext,
    taa_ui** uiout)
{
    taa_ui* ui;
    uint32_t sz =
        sizeof(*ui) +
        sizeof(*ui->stack)*stacksize +
        sizeof(*ui->ctrlbuffer)*maxcontrols +
        sizeof(*ui->textbuffer)*maxtext;
    void* buf = calloc(sz, 1);
    ui = (taa_ui*) buf;
    buf = ui + 1;
    ui->stack = (taa_ui_stack*) buf;
    buf = ui->stack + stacksize;
    ui->ctrlbuffer = (taa_ui_control*) buf;
    buf = ui->ctrlbuffer + maxcontrols;
    ui->textbuffer = (char*) buf;
    ui->stackcapacity = stacksize;
    ui->ctrlcapacity = maxcontrols;
    ui->textcapacity = maxtext;
    *uiout = ui;
}

//****************************************************************************
void taa_ui_destroy(
    taa_ui* ui)
{
    free(ui);
}

//****************************************************************************
const taa_ui_controllist* taa_ui_end(
    taa_ui* ui)
{
    taa_ui_event* evt = ui->events;
    taa_ui_event* evtend = evt + ui->numevents;
    taa_ui_controllist* dl = &ui->drawlist;
    taa_ui_control* ctrls = ui->ctrlbuffer + ui->ctrlcursor;
    taa_ui_control* ctrlend = ui->ctrlbuffer + ui->ctrlcapacity;
    assert(ui->stacksize == 1);
    dl->controls = ctrls;
    dl->numcontrols = ui->ctrlcapacity - ui->ctrlcursor;
    dl->caret = ui->caret;
    dl->selectstart = ui->selectstart;
    dl->selectlength = ui->selectlength;
    dl->framecounter = ui->framecounter;
    dl->viewwidth = ui->viewwidth;
    dl->viewheight = ui->viewheight;
    ++ui->framecounter;
    while(evt != evtend)
    {
        if (!evt->isconsumed && evt->winevent.type==taa_WINDOW_EVENT_KEY_DOWN)
        {
            // if there are any unconsumed key down events, check to see if
            // focus navigation is necessary
            const taa_ui_control* ctrlitr = ctrls;
            switch(evt->winevent.key.keycode)
            {
            case taa_KEY_TAB:
                break;
            }
        }
        ++evt;
    }
    return dl;
}

//****************************************************************************
uint32_t taa_ui_generateid(
    taa_ui* ui)
{
    return ++ui->idcounter;
}

//****************************************************************************
void taa_ui_label(
    taa_ui* ui,
    taa_ui_styleid styleid,
    const taa_ui_rect* rect,
    const char* txt)
{
    const taa_ui_stack* p = ui->stack + ui->stacksize - 1;
    const taa_ui_style* style = ui->stylesheet.styles + styleid;
    uint32_t txtlen = strlen(txt);
    taa_ui_rect scrnrect;
    taa_ui_rect cliprect;
    taa_ui_layoutrect(ui, style, rect, &scrnrect);
    taa_ui_rectintersect(&p->childclip, &scrnrect, &cliprect);
    taa_ui_pushtextcontrol(
        ui,
        taa_UI_LABEL,
        styleid,
        &scrnrect,
        &cliprect,
        0,
        txt,
        txtlen);
}

//****************************************************************************
void taa_ui_row_begin(
    taa_ui* ui,
    int32_t h,
    taa_ui_halign halign,
    taa_ui_valign valign)
{
    taa_ui_stack* s = ui->stack + ui->stacksize;
    taa_ui_stack* p = s - 1;
    taa_CHECK_ERROR(ui->stacksize < ui->stackcapacity, "stack overflow");
    switch(h)
    {
    case taa_UI_HEIGHT_DEFAULT: h = h; break;
    case taa_UI_HEIGHT_FILL: h = p->contentrect.h - p->layoutoffset; break;
    default: break;
    }
    s->id = -1;
    s->styleid = -1;
    s->halign = halign;
    s->valign = valign;
    s->layout = taa_UI_LAYOUT_ROW;
    s->layoutoffset = 0;
    s->scrollx = 0;
    s->scrolly = 0;
    s->flags = 0;
    s->ctrlcursor = ui->ctrlcursor;
    // row rect
    s->rect.x = p->contentrect.x;
    s->rect.y = p->contentrect.y + p->layoutoffset;
    s->rect.w = p->contentrect.w;
    s->rect.h = h;
    // content rect is row rect
    s->contentrect = s->rect;
    // cliprect is row rect clipped against parent
    taa_ui_rectintersect(&p->childclip, &s->rect, &s->cliprect);
    // initialize the child rect
    s->childrect.x = taa_ui_align_x(&s->contentrect, s->halign);
    s->childrect.y = taa_ui_align_y(&s->contentrect, s->valign);
    s->childrect.w = 0;
    s->childrect.h = 0;
    // child cliprect is child rect clipped by the row's cliprect
    taa_ui_rectintersect(&s->cliprect, &s->contentrect, &s->childclip);
    // adjust parent's childrect to contain the row
    taa_ui_rectunion(&p->childrect, &s->rect, &p->childrect);
    p->layoutoffset += h;
    ++ui->stacksize;
}

//****************************************************************************
void taa_ui_row_end(
    taa_ui* ui)
{
    taa_ui_stack* s = ui->stack + ui->stacksize - 1;
    assert(s->layout == taa_UI_LAYOUT_ROW);
    assert(ui->stacksize > 1);
    --ui->stacksize;
}

//****************************************************************************
void taa_ui_setfocus(
    taa_ui* ui,
    int32_t x,
    int32_t y)
{
    ui->state.focusx = x;
    ui->state.focusy = y;
}

//****************************************************************************
void taa_ui_setstylesheet(
    taa_ui* ui,
    const taa_ui_stylesheet* styles)
{
    ui->stylesheet = *styles;
}

//****************************************************************************
void taa_ui_sizecols(
    taa_ui* ui,
    const taa_ui_styleid* styleids,
    int32_t* widthsin,
    uint32_t numcols,
    int32_t* widthsout)
{
    const taa_ui_stack* p = ui->stack + ui->stacksize - 1;
    const taa_ui_style* styles = ui->stylesheet.styles;
    int32_t pw = p->contentrect.w;
    int32_t fixedw = 0;
    int32_t fillw = 0;
    int32_t numfill = 0;
    const int32_t* witr;
    const int32_t* wend;
    const uint32_t* styleiditr;
    int32_t* woutitr;
    // first pass: determine amount of auto filled space and cols
    witr = widthsin;
    wend = witr + numcols;
    styleiditr = styleids;
    while(witr != wend)
    {
        const taa_ui_style* style = styles + (*styleiditr);
        int32_t w = *witr;
        assert(style < ui->stylesheet.styles+ui->stylesheet.numstyles);
        fixedw += style->lmargin + style->rmargin;
        switch(w)
        {
        case taa_UI_WIDTH_DEFAULT:
            fixedw += style->defaultw;
            break;
        case taa_UI_WIDTH_FILL:
            ++numfill;
            break;
        default:
            fixedw += w;
            break;
        }
        ++witr;
        ++styleiditr;
    }
    fillw = (pw > fixedw) ? pw - fixedw : 0;
    if(numfill > 1)
    {
        fillw /= numfill;
    }
    // second pass: set out params
    witr = widthsin;
    wend = witr + numcols;
    styleiditr = styleids;
    woutitr = widthsout;
    while(witr != wend)
    {
        const taa_ui_style* style = styles + (*styleiditr);
        int32_t win = *witr;
        int32_t wout = style->lmargin + style->rmargin;
        switch(win)
        {
        case taa_UI_WIDTH_DEFAULT:
            wout += style->defaultw;
            break;
        case taa_UI_WIDTH_FILL:
            wout += fillw;
            break;
        default:
            wout += win;
            break;
        }
        *woutitr = wout;
        ++witr;
        ++styleiditr;
        ++woutitr;
    }
}

//****************************************************************************
void taa_ui_sizerows(
    taa_ui* ui,
    const taa_ui_styleid* styleids,
    int32_t* heightsin,
    uint32_t numrows,
    int32_t* heightsout)
{
    const taa_ui_stack* p = ui->stack + ui->stacksize - 1;
    const taa_ui_style* styles = ui->stylesheet.styles;
    int32_t ph = p->contentrect.h;
    int32_t fixedh = 0;
    int32_t fillh = 0;
    int32_t numfill = 0;
    const int32_t* hitr;
    const int32_t* hend;
    const uint32_t* styleiditr;
    int32_t* houtitr;
    // first pass: determine amount of auto filled space and cols
    hitr = heightsin;
    hend = hitr + numrows;
    styleiditr = styleids;
    while(hitr != hend)
    {
        const taa_ui_style* style = styles + (*styleiditr);
        int32_t h = *hitr;
        assert(style < ui->stylesheet.styles+ui->stylesheet.numstyles);
        fixedh += style->tmargin + style->bmargin;
        switch(h)
        {
        case taa_UI_HEIGHT_DEFAULT:
            fixedh += style->defaulth;
            break;
        case taa_UI_HEIGHT_FILL:
            ++numfill;
            break;
        default:
            fixedh += h;
            break;
        }
        ++hitr;
        ++styleiditr;
    }
    fillh = (ph > fixedh) ? ph - fixedh : 0;
    if(numfill > 1)
    {
        fillh /= numfill;
    }
    // second pass: set out params
    hitr = heightsin;
    hend = hitr + numrows;
    styleiditr = styleids;
    houtitr = heightsout;
    while(hitr != hend)
    {
        const taa_ui_style* style = styles + (*styleiditr);
        int32_t hin = *hitr;
        int32_t hout = style->tmargin + style->bmargin;
        switch(hin)
        {
        case taa_UI_HEIGHT_DEFAULT:
            hout += style->defaulth;
            break;
        case taa_UI_HEIGHT_FILL:
            hout += fillh;
            break;
        default:
            hout += hin;
            break;
        }
        *houtitr = hout;
        ++hitr;
        ++styleiditr;
        ++houtitr;
    }
}

//****************************************************************************
void taa_ui_table_begin(
    taa_ui* ui,
    const taa_ui_rect* rect)
{
    taa_ui_stack* s = ui->stack + ui->stacksize;
    const taa_ui_stack* p = s - 1;
    taa_ui_style style;
    taa_CHECK_ERROR(ui->stacksize < ui->stackcapacity, "stack overflow");
    memset(&style, 0, sizeof(style));
    s->id = -1;
    s->styleid = -1;
    s->halign = taa_UI_HALIGN_LEFT;
    s->valign = taa_UI_VALIGN_TOP;
    s->layout = taa_UI_LAYOUT_TABLE;
    s->layoutoffset = 0;
    s->scrollx = 0;
    s->scrolly = 0;
    s->flags = 0;
    s->ctrlcursor = ui->ctrlcursor;
    taa_ui_layoutrect(ui, &style, rect, &s->rect);
    s->contentrect = s->rect;
    // cliprect is control rect clipped against parent
    taa_ui_rectintersect(&p->childclip, &s->rect, &s->cliprect);
    // initialize the child rect
    s->childrect.x = s->contentrect.x;
    s->childrect.y = s->contentrect.y;
    s->childrect.w = 0;
    s->childrect.h = 0;
    // child cliprect is content rect clipped by the control's cliprect
    taa_ui_rectintersect(&s->cliprect, &s->contentrect, &s->childclip);
    ++ui->stacksize;
}

//****************************************************************************
void taa_ui_table_end(
    taa_ui* ui)
{
    taa_ui_stack* s = ui->stack + ui->stacksize - 1;
    assert(s->layout==taa_UI_LAYOUT_TABLE);
    assert(ui->stacksize > 1);
    --ui->stacksize;
}

//****************************************************************************
void taa_ui_textbox(
    taa_ui* ui,
    taa_ui_styleid styleid,
    const taa_ui_rect* rect,
    char* txt,
    uint32_t txtsize,
    uint32_t* flagsout)
{
    const taa_ui_stack* p = ui->stack + ui->stacksize - 1;

    const taa_ui_style* style = ui->stylesheet.styles + styleid;
    taa_ui_state* state = &ui->state;
    taa_ui_state* prev = &ui->prevstate;
    uint32_t flags = taa_UI_FLAG_NONE;
    uint32_t txtlen = strlen(txt);
    taa_ui_rect scrnrect;
    taa_ui_rect cliprect;
    taa_ui_layoutrect(ui, style, rect, &scrnrect);
    taa_ui_rectintersect(&p->childclip, &scrnrect, &cliprect);
    if(taa_ui_tryhover(ui, &cliprect))
    {
        flags |= taa_UI_FLAG_HOVER;
    }
    if(taa_ui_tryfocus(ui, &cliprect))
    {
        taa_ui_event* evt = ui->events;
        taa_ui_event* evtend = evt + ui->numevents;
        uint32_t selmin = ui->selectstart;
        uint32_t selmax = selmin + ui->selectlength;
        uint32_t caret = ui->caret;
        if (state->focusx!=prev->focusx || state->focusy!=prev->focusy)
        {
            // if the focus point changed, reset the caret
            caret = 0;
            selmin = 0;
            selmax = 0;
        }
        // process key events
        while(evt != evtend)
        {
            evt->isconsumed = 1;
            if(evt->winevent.type == taa_WINDOW_EVENT_KEY_DOWN)
            {
                taa_window_keyevent* k = &evt->winevent.key;
                if(ui->state.kb[taa_KEY_SHIFT] != 0)
                {
                    // if the shift key is down, adjust selection
                    switch(k->keycode)
                    {
                    case taa_KEY_END:
                        if(caret != txtlen)
                        {
                            if(ui->state.kb[taa_KEY_SHIFT] != 0)
                            {
                                if(selmin == selmax)
                                {
                                    selmin = caret;
                                }
                                selmax = txtlen;
                            }
                        }
                        break;
                    case taa_KEY_HOME:
                        if(caret > 0)
                        {
                            if(ui->state.kb[taa_KEY_SHIFT] != 0)
                            {
                                if(selmin == selmax)
                                {
                                    selmax = caret;
                                }
                                selmin = 0;
                            }
                        }
                        break;
                    case taa_KEY_LEFT:
                        if(caret > 0)
                        {
                            if(ui->state.kb[taa_KEY_SHIFT] != 0)
                            {
                                if(selmin == selmax)
                                {
                                    selmax = caret;
                                }
                                selmin = (selmin<caret-1) ?selmin:caret-1;
                            }
                        }
                        break;
                    case taa_KEY_RIGHT:
                        if(caret < txtlen)
                        {
                            if(ui->state.kb[taa_KEY_SHIFT] != 0)
                            {
                                if(selmin == selmax)
                                {
                                    selmin = caret;
                                }
                                selmax = (selmax>caret+1) ?selmax:caret+1;
                            }
                        }
                        break;
                    default:
                        break;
                    }
                }
                else if(k->ascii>0 && selmin!=selmax)
                {
                    // shift is not down, remove the selection
                    uint32_t sellen = selmax - selmin;
                    memmove(txt+selmin,txt+selmax,txtlen+1-selmin-sellen);
                    selmin = selmax = 0;
                    txtlen -= sellen;
                }
                switch(k->keycode)
                {
                case taa_KEY_BACKSPACE:
                    if(caret > 0)
                    {
                        char* dst = txt + caret - 1;
                        char* src = txt + caret;
                        char* end = txt + txtlen;
                        while(dst < end) { *dst++ = *src++; }
                        --caret;
                        --txtlen;
                    }
                    break;
                case taa_KEY_TAB:
                    evt->isconsumed = 0;
                    break;
                case taa_KEY_END:
                    caret = txtlen;
                    break;
                case taa_KEY_HOME:
                    caret = 0;
                    break;
                case taa_KEY_LEFT:
                    if(caret > 0)
                    {
                        --caret;
                    }
                    break;
                case taa_KEY_RIGHT:
                    if(caret < txtlen)
                    {
                        ++caret;
                    }
                    break;
                case taa_KEY_DELETE:
                    if(caret < txtlen)
                    {
                        char* dst = txt + caret;
                        char* src = txt + caret + 1;
                        char* end = txt + txtlen;
                        while(dst < end) { *dst++ = *src++; }
                        --txtlen;
                    }
                    break;
                default:
                    if(txtlen < txtsize - 1)
                    {
                        char ch = k->ascii;
                        if(isprint(ch))
                        {
                            char* dst = txt + txtlen + 1;
                            char* src = txt + txtlen;
                            char* end = txt + caret;
                            while(dst > end) { *dst-- = *src--; }
                            *dst = ch;
                            ++caret;
                            ++txtlen;
                        }
                    }
                    break;
                }
            }
            else if(evt->winevent.type == taa_WINDOW_EVENT_MOUSE_BUTTON1_DOWN)
            {
                // find the character that matches the cursor position
                const char* s = txt;
                const taa_font* font = style->font;
                if(font != NULL)
                {
                    int32_t xitr = scrnrect.x - caret;
                    while(*s != '\0')
                    {
                        xitr += font->characters[(uint8_t) (*s)].width;
                        if(xitr > evt->winevent.mouse.state.cursorx)
                        {
                            break;
                        }
                        ++caret;
                        ++s;
                    }
                }
                if(ui->state.kb[taa_KEY_SHIFT] != 0)
                {
                    // if the shift key was down, adjust selection
                    selmin = (selmin < caret) ? selmin : caret;
                    selmax = (selmax > caret) ? selmax : caret;
                }
                else
                {
                    selmin = selmax = caret;
                }
            }
            else if(evt->winevent.type == taa_WINDOW_EVENT_MOUSE_MOVE)
            {
                if(evt->winevent.mouse.state.button1)
                {
                    // if dragging the mouse
                    // find the character that matches the cursor position
                    const char* s = txt;
                    const taa_font* font = style->font;
                    if(font != NULL)
                    {
                        int32_t xitr = scrnrect.x - caret;
                        while(*s != '\0')
                        {
                            xitr += font->characters[(uint8_t) (*s)].width;
                            if(xitr > evt->winevent.mouse.state.cursorx)
                            {
                                break;
                            }
                            ++caret;
                            ++s;
                        }
                    }
                    selmin = (selmin < caret) ? selmin : caret;
                    selmax = (selmax > caret) ? selmax : caret;
                }
            }
            ++evt;
        }
        flags |= taa_UI_FLAG_FOCUS;
        ui->caret = caret;
        ui->selectstart = selmin;
        ui->selectlength = selmax - selmin;
    }
    taa_ui_pushtextcontrol(
        ui,
        taa_UI_TEXTBOX,
        styleid,
        &scrnrect,
        &cliprect,
        flags,
        txt,
        txtlen);
    if(flagsout != NULL)
    {
        *flagsout = flags;
    }
}
//****************************************************************************
void taa_ui_vscrollbar(
    taa_ui* ui,
    taa_ui_styleid panestyleid,
    taa_ui_styleid barstyleid,
    const taa_ui_rect* rect,
    int32_t datasize,
    int32_t* value,
    uint32_t* flagsout)
{
    // datasize = size of the scrollable content
    // scrollh = size of the scroll pane minus border, padding and bar margin
    // scrollrange = difference in size between data and scrollh
    // barh = size of the draggable scroll bar
    // barrange = difference in size between scroll bar and its pane
    const taa_ui_stack* p = ui->stack + ui->stacksize - 1;
    const taa_ui_style* style = ui->stylesheet.styles + panestyleid;
    const taa_ui_style* barstyle = ui->stylesheet.styles + barstyleid;
    int32_t loffset = style->lborder + style->lpadding;
    int32_t roffset = style->rborder + style->rpadding;
    int32_t toffset = style->tborder + style->tpadding;
    int32_t boffset = style->bborder + style->bpadding;
    int32_t tbaroffset = barstyle->tborder + barstyle->tpadding;
    int32_t bbaroffset = barstyle->bborder + barstyle->bpadding;
    int32_t barmarginw = barstyle->lmargin + barstyle->rmargin;
    int32_t barmarginh = barstyle->tmargin + barstyle->bmargin;
    uint32_t flags = taa_UI_FLAG_NONE;
    taa_ui_rect scrnrect;
    taa_ui_rect cliprect;
    taa_ui_rect barrect;
    int32_t scrollh;
    int32_t scrollrange;
    int32_t barmin;
    int32_t barh;
    int32_t barrange;
    // calculate control rects
    taa_ui_layoutrect(ui, style, rect, &scrnrect);
    taa_ui_rectintersect(&p->childclip, &scrnrect, &cliprect);
    // calculate scroll ranges and sizes
    scrollh = scrnrect.h - toffset - boffset - barmarginh;
    scrollrange = datasize - scrollh;
    barmin = tbaroffset + bbaroffset;
    barh = (scrollh*scrollh)/datasize;
    barh = (barh >= barmin) ? barh : barmin;
    barh = (barh <= scrollh) ? barh : scrollh;
    barrange = scrollh - barh;
    // set bar rect
    barrect.x = scrnrect.x + loffset + barstyle->lmargin;
    barrect.y = scrnrect.y + toffset + barstyle->tmargin;
    barrect.w = scrnrect.w - loffset - roffset - barmarginw;
    barrect.h = barh;
    if(scrollh > 0 && scrollrange > 0 && barrange > 0)
    {
        // if scrollable
        int32_t val = *value;
        int32_t bary = (val*barrange)/scrollrange;
        barrect.y += bary;
        if(taa_ui_tryhover(ui, &cliprect))
        {
            flags |= taa_UI_FLAG_HOVER;
        }
        if(taa_ui_tryfocus(ui, &cliprect))
        {
            int32_t dy = 0;
            taa_ui_event* evt = ui->events;
            taa_ui_event* evtend = evt + ui->numevents;
            if(ui->state.isscrolling)
            {
                int32_t my = ui->state.mouse.cursory;
                int32_t mp = ui->prevstate.mouse.cursory;
                dy = my - mp;
            }
            while(evt != evtend)
            {
                taa_window_eventtype evttype = evt->winevent.type;
                evt->isconsumed = 1;
                if(evttype == taa_WINDOW_EVENT_KEY_DOWN)
                {
                    switch(evt->winevent.key.keycode)
                    {
                    case taa_KEY_TAB:
                        evt->isconsumed = 0;
                        break;
                    case taa_KEY_UP:
                        --dy;
                        break;
                    case taa_KEY_DOWN:
                        ++dy;
                        break;
                    case taa_KEY_PAGE_UP:
                        dy -= barh;
                        break;
                    case taa_KEY_PAGE_DOWN:
                        dy += barh;
                        break;
                    default:
                        break;
                    }
                }
                else if(evttype==taa_WINDOW_EVENT_MOUSE_BUTTON1_DOWN)
                {
                    const taa_mouse_state* evtm;
                    evtm = &evt->winevent.mouse.state;
                    if(taa_ui_testrect(
                        evtm->cursorx,
                        evtm->cursory,
                        &barrect))
                    {
                        ui->state.isscrolling = 1;
                    }
                }
                else if(evttype == taa_WINDOW_EVENT_MOUSE_BUTTON1_UP)
                {
                    ui->state.isscrolling = 0;
                }
                ++evt;
            }
            if(dy != 0)
            {
                int32_t ds = (dy*scrollrange)/barrange;
                val += ds;
                bary += dy;
                if(val < 0)
                {
                    val = 0;
                    dy -= bary;
                }
                if(val > scrollrange)
                {
                    val = scrollrange;
                    dy -= bary-barrange;
                }
                barrect.y += dy;
                *value = val;
            }
            flags |= taa_UI_FLAG_FOCUS;
        }
    }
    else
    {
        *value = 0;
        barrect.h = 0;
        barrect.w = 0;
    }
    taa_ui_pushscrollcontrol(
        ui,
        taa_UI_VSCROLLBAR,
        panestyleid,
        &scrnrect,
        &cliprect,
        flags,
        barstyleid,
        &barrect);
    if(flagsout != NULL)
    {
        *flagsout = flags;
    }
}
