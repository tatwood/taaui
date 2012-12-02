/**
 * @brief     ui input handling implementation
 * @author    Thomas Atwood (tatwood.net)
 * @date      2011
 * @copyright unlicense / public domain
 ****************************************************************************/
#ifndef taa_UIINPUT_H_
#define taa_UIINPUT_H_

#include <taa/ui.h>
#include <assert.h>
#include <ctype.h>
#include <limits.h>

enum taa_uiinput_textfilter_e
{
    taa_UIINPUT_FILTER_NONE,
    taa_UIINPUT_FILTER_NUMERIC,
    taa_UIINPUT_FILTER_PRINT
};

typedef enum taa_uiinput_textfilter_e taa_uiinput_textfilter;
typedef struct taa_uiinput_event_s taa_uiinput_event;
typedef struct taa_uiinput_state_s taa_uiinput_state;

struct taa_uiinput_event_s
{
    taa_window_event winevent;
    int isconsumed;
};

struct taa_uiinput_state_s
{
    taa_ui_control focuscontrol;
    // input device state for the frame
    taa_keyboard_state kb;
    taa_mouse_state mouse;
    // input events for the frame
    size_t numevents;
    taa_uiinput_event events[16];
    int focusindex;
    int focusparent[2];
    int caret;
    int selectstart;
    int selectlength;
    int isdragging;
    int dragx;
    int dragy;
};

//****************************************************************************
static int taa_uiinput_testrect(
    int x,
    int y,
    const taa_ui_rect* rect)
{
    return
        rect->w > 0 &&
        rect->h > 0 &&
        rect->x <= x &&
        rect->y <= y && 
        rect->x+rect->w > x &&
        rect->y+rect->h > y;
}

//****************************************************************************
static void taa_uiinput_begin(
    taa_uiinput_state* prevstate,
    taa_uiinput_state* nextstate,
    const taa_keyboard_state* kb,
    const taa_mouse_state* mouse,
    const taa_window_event* winevents,
    size_t numevents)
{
    const taa_window_event* evt = winevents;
    const taa_window_event* evtend = evt + numevents;
    taa_uiinput_event* inputevt;
    taa_uiinput_event* inputevtend;
    size_t maxevents;
    *prevstate = *nextstate;
    maxevents = sizeof(nextstate->events)/sizeof(nextstate->events[0]);
    inputevt = nextstate->events;
    inputevtend = inputevt + maxevents;
    while(evt != evtend)
    {
        int keep = 0;
        switch(evt->type)
        {
        case taa_WINDOW_EVENT_KEY_DOWN:
        case taa_WINDOW_EVENT_KEY_UP:
        case taa_WINDOW_EVENT_MOUSE_BUTTON1_DOWN:
        case taa_WINDOW_EVENT_MOUSE_BUTTON1_UP:
            keep = 1;
            break;
        default:
            break;
        }
        if(keep)
        {
            inputevt->winevent = *evt;
            inputevt->isconsumed = 0;
            ++inputevt;
            if(inputevt == inputevtend)
            {
                break;
            }
        }
        ++evt;
    }
    nextstate->focusindex = -1;
    nextstate->isdragging = 0;
    nextstate->kb = *kb;
    nextstate->mouse = *mouse;
    nextstate->numevents = (uint32_t) (ptrdiff_t)(inputevt-nextstate->events);
}

//****************************************************************************
static unsigned int taa_uiinput_button(
    taa_uiinput_state* prevstate)
{
    taa_uiinput_event* evt = prevstate->events;
    taa_uiinput_event* evtend = evt + prevstate->numevents;
    taa_ui_rect* rect = &prevstate->focuscontrol.rect;
    unsigned int flags = taa_UI_FLAG_FOCUS;
    int cx = prevstate->mouse.cursorx;
    int cy = prevstate->mouse.cursory;
    if(taa_uiinput_testrect(cx,cy,rect))
    {
        if(prevstate->mouse.button1)
        {
            flags |= taa_UI_FLAG_PRESSED;
        }
    }
    if(prevstate->kb.keys[taa_KEY_ENTER] != 0)
    {
        flags |= taa_UI_FLAG_PRESSED;
    }
    // process events
    while(evt != evtend)
    {
        taa_window_eventtype evttype = evt->winevent.type;
        if(evttype == taa_WINDOW_EVENT_KEY_DOWN)
        {
            if(evt->winevent.key.keycode == taa_KEY_ENTER)
            {
                flags |= taa_UI_FLAG_CLICKED;
                evt->isconsumed = 1;
            }
        }
        else if(evttype == taa_WINDOW_EVENT_MOUSE_BUTTON1_UP)
        {
            int32_t evtx = evt->winevent.mouse.cursorx;
            int32_t evty = evt->winevent.mouse.cursory;
            if(taa_uiinput_testrect(evtx, evty, rect))
            {
                flags |= taa_UI_FLAG_CLICKED;
            }
        }
        ++evt;
    }
    return flags;
}

//****************************************************************************
static void taa_uiinput_end(
    taa_uiinput_state* prevstate,
    taa_uiinput_state* nextstate,
    taa_ui_control* controls,
    int firstcontrol,
    size_t numcontrols)
{
    taa_uiinput_event* evt;
    taa_uiinput_event* evtend;
    int focusindex;
    focusindex = nextstate->focusindex;
    // check if any unhandled keyboard navigation events are left over from
    // this frame.
    if(prevstate->focusindex >= 0)
    {
        const taa_ui_control* focusctrl = &prevstate->focuscontrol;
        taa_keyboard_keycode navkey = taa_KEY_UNKNOWN;
        int minx = focusctrl->rect.x;
        int maxx = focusctrl->rect.x + focusctrl->rect.w;
        int miny = focusctrl->rect.y;
        int maxy = focusctrl->rect.y + focusctrl->rect.h;
        evt = prevstate->events;
        evtend = evt + prevstate->numevents;
        while(evt != evtend)
        {
            if (
                !evt->isconsumed &&
                evt->winevent.type == taa_WINDOW_EVENT_KEY_DOWN)
            {
                // if there are any unconsumed key down events, check to see 
                // if focus navigation is necessary
                switch(evt->winevent.key.keycode)
                {
                case taa_KEY_TAB:
                    navkey = taa_KEY_TAB;
                    break;
                case taa_KEY_LEFT:
                    navkey = taa_KEY_LEFT;
                    minx = INT_MIN;
                    maxx = focusctrl->rect.x;
                    break;
                case taa_KEY_UP:
                    navkey = taa_KEY_UP;
                    miny = INT_MIN;
                    maxy = focusctrl->rect.y;
                    break;
                case taa_KEY_RIGHT:
                    navkey = taa_KEY_RIGHT;
                    minx = focusctrl->rect.x + focusctrl->rect.w - 1;
                    maxx = INT_MAX;
                    break;
                case taa_KEY_DOWN:
                    navkey = taa_KEY_DOWN;
                    miny = focusctrl->rect.y + focusctrl->rect.h - 1;
                    maxy = INT_MAX;
                    break;
                default:
                    break;
                }
                if(navkey != taa_KEY_UNKNOWN)
                {
                    break;
                }
            }
            ++evt;
        }
        if(navkey == taa_KEY_TAB)
        {
            // tab navigation. iterate backwards so tab order is same order
            // in which controls were added to the ui simulation
            taa_ui_control* focusctrl = controls + prevstate->focusindex;
            taa_ui_control* ctrlitr = focusctrl - 1;
            taa_ui_control* ctrlend = controls + firstcontrol - 1;
            if(ctrlitr == ctrlend)
            {
                ctrlitr = controls + firstcontrol + numcontrols - 1;
            }
            while(ctrlitr != focusctrl)
            {
                if(
                    (ctrlitr->flags & taa_UI_FLAG_DISABLED) == 0 &&
                    ctrlitr->type != taa_UI_CONTAINER_END &&
                    ctrlitr->type != taa_UI_LABEL)
                {
                    focusindex = (int) (ptrdiff_t) (ctrlitr - controls);
                    break;
                }
                --ctrlitr;
                if(ctrlitr == ctrlend)
                {
                    ctrlitr = controls + firstcontrol + numcontrols - 1;
                }
            }
        }
        else if(navkey != taa_KEY_UNKNOWN)
        {
            taa_ui_control* focusctrl = controls + prevstate->focusindex;
            taa_ui_control* ctrlitr;
            taa_ui_control* ctrlend;
            int depth;
            if(prevstate->focusparent[0] >= 0)
            {
                ctrlitr = controls + (prevstate->focusparent[0]+1);
                ctrlend = controls + (prevstate->focusparent[1]);
            }
            else
            {
                ctrlitr = controls + firstcontrol;
                ctrlend = ctrlitr + numcontrols;
            }
            depth = 0;
            while(ctrlitr != ctrlend)
            {
                if(
                    depth == 0 &&
                    (ctrlitr->flags & taa_UI_FLAG_DISABLED) == 0 &&
                    ctrlitr->type != taa_UI_CONTAINER_END &&
                    ctrlitr->type != taa_UI_LABEL)
                {
                    switch(navkey)
                    {
                    case taa_KEY_LEFT:
                        if(
                            ctrlitr->rect.x > minx &&
                            ctrlitr->rect.x < maxx &&
                            ctrlitr->rect.y <= maxy &&
                            ctrlitr->rect.y+ctrlitr->rect.h >= miny)
                        {
                            minx = ctrlitr->rect.x;
                            focusctrl = ctrlitr;
                        }
                        break;
                    case taa_KEY_UP:
                        if(
                            ctrlitr->rect.y > miny &&
                            ctrlitr->rect.y < maxy &&
                            ctrlitr->rect.x <= maxx &&
                            ctrlitr->rect.x+ctrlitr->rect.w >= minx)
                        {
                            miny = ctrlitr->rect.y;
                            focusctrl = ctrlitr;
                        }
                        break;
                    case taa_KEY_RIGHT:
                        if(
                            ctrlitr->rect.x > minx &&
                            ctrlitr->rect.x < maxx &&
                            ctrlitr->rect.y <= maxy &&
                            ctrlitr->rect.y+ctrlitr->rect.h >= miny)
                        {
                            maxx = ctrlitr->rect.x;
                            focusctrl = ctrlitr;
                        }
                        break;
                    case taa_KEY_DOWN:
                        if(
                            ctrlitr->rect.y > miny &&
                            ctrlitr->rect.y < maxy &&
                            ctrlitr->rect.x <= maxx &&
                            ctrlitr->rect.x+ctrlitr->rect.w >= minx)
                        {
                            maxy = ctrlitr->rect.y;
                            focusctrl = ctrlitr;
                        }
                        break;
                    default:
                        break;
                    }
                }
                if(ctrlitr->type == taa_UI_CONTAINER_BEGIN)
                {
                    ++depth;
                }
                else if(ctrlitr->type == taa_UI_CONTAINER_END)
                {
                    --depth;
                }
                ++ctrlitr;
            }
            assert(depth == 0);
            if(focusctrl != (controls + prevstate->focusindex))
            {
                focusindex = (int) (ptrdiff_t) (focusctrl - controls);
            }
        }
    }
    // check the input for the next frame to see if the user clicked on
    // a control and changed focus. this needs to be handled base on the
    // events for the next frame because the control rects are available for
    // point tests at this point in the frame and this will allow the focus to
    // be transferred for the newly focused control to receive the events on
    // the next frame.
    evt = nextstate->events;
    evtend = evt + nextstate->numevents;
    while(evt != evtend)
    {
        assert(!evt->isconsumed);
        if(evt->winevent.type == taa_WINDOW_EVENT_MOUSE_BUTTON1_DOWN)
        {
            taa_ui_control* ctrlitr = controls + (firstcontrol+numcontrols-1);
            taa_ui_control* ctrlend = controls + firstcontrol - 1;
            int x = evt->winevent.mouse.cursorx;
            int y = evt->winevent.mouse.cursory;
            nextstate->focusindex = -1;
            while(ctrlitr != ctrlend)
            {
                if(
                    ctrlitr->type != taa_UI_CONTAINER_END &&
                    ctrlitr->type != taa_UI_LABEL)
                {
                    if(taa_uiinput_testrect(x, y, &ctrlitr->cliprect))
                    {
                        focusindex = (int) (ptrdiff_t) (ctrlitr-controls);
                        break;
                    }
                }
                --ctrlitr;
            }
            break;
        }
        ++evt;
    }
    nextstate->focusindex = focusindex;
    if(focusindex != prevstate->focusindex)
    {
        nextstate->focusparent[0] = -1;
        nextstate->focusparent[1] = -1;
        nextstate->caret = 0;
        nextstate->selectstart = 0;
        nextstate->selectlength = 0;
    }
    if(focusindex >= 0)
    {
        nextstate->focuscontrol = controls[focusindex];
    }
}

//****************************************************************************
static unsigned int taa_uiinput_text(
    taa_uiinput_state* prevstate,
    taa_uiinput_state* nextstate,
    taa_uiinput_textfilter filter,
    const taa_ui_font* font,
    char* txt,
    size_t txtsize,
    size_t* txtlen_out)
{
    const taa_ui_rect* rect = &prevstate->focuscontrol.rect;
    const taa_keyboard_state* kb = &prevstate->kb;
    taa_uiinput_event* evt = prevstate->events;
    taa_uiinput_event* evtend = evt + prevstate->numevents;
    size_t txtlen = strlen(txt);
    int selmin = prevstate->selectstart;
    int selmax = selmin + prevstate->selectlength;
    int caret = prevstate->caret;
    if(selmin > ((int) txtlen))
    {
        selmin = txtlen;
    }
    if(selmax > ((int) txtlen))
    {
        selmax = txtlen;
    }
    if(caret > ((int) txtlen))
    {
        caret = txtlen;
    }
    // process key events
    while(evt != evtend)
    {
        taa_window_eventtype evttype = evt->winevent.type;
        if(evttype == taa_WINDOW_EVENT_KEY_DOWN)
        {
            uint8_t keycode = evt->winevent.key.keycode;
            char ascii = evt->winevent.key.ascii;
            if((kb->keys[taa_KEY_LSHIFT] | kb->keys[taa_KEY_RSHIFT]) != 0)
            {
                // if the shift key is down, adjust selection
                switch(keycode)
                {
                case taa_KEY_END:
                    if(caret != txtlen)
                    {
                        if(selmin == selmax)
                        {
                            selmin = caret;
                        }
                        selmax = txtlen;
                    }
                    evt->isconsumed = 1;
                    break;
                case taa_KEY_HOME:
                    if(caret > 0)
                    {
                        if(selmin == selmax)
                        {
                            selmax = caret;
                        }
                        selmin = 0;
                    }
                    evt->isconsumed = 1;
                    break;
                case taa_KEY_LEFT:
                    if(caret > 0)
                    {
                        if(selmin == selmax)
                        {
                            selmax = caret;
                        }
                        selmin = (selmin<caret-1) ?selmin:caret-1;
                    }
                    evt->isconsumed = 1;
                    break;
                case taa_KEY_RIGHT:
                    if(caret < ((int) txtlen))
                    {
                        if(selmin == selmax)
                        {
                            selmin = caret;
                        }
                        selmax = (selmax>caret+1) ?selmax:caret+1;
                    }
                    evt->isconsumed = 1;
                    break;
                default:
                    break;
                }
            }
            else if(ascii > 0 && selmin != selmax)
            {
                // shift is not down, remove the selection
                size_t sellen = selmax - selmin;
                memmove(txt+selmin,txt+selmax,txtlen+1-selmin-sellen);
                selmin = selmax = 0;
                txtlen -= sellen;
                if(caret > ((int) txtlen))
                {
                    caret = txtlen;
                }
            }
            switch(keycode)
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
                evt->isconsumed = 1;
                break;
            case taa_KEY_END:
                caret = txtlen;
                evt->isconsumed = 1;
                break;
            case taa_KEY_HOME:
                caret = 0;
                evt->isconsumed = 1;
                break;
            case taa_KEY_LEFT:
                if(caret > 0)
                {
                    --caret;
                }
                evt->isconsumed = 1;
                break;
            case taa_KEY_RIGHT:
                if(caret < ((int) txtlen))
                {
                    ++caret;
                }
                evt->isconsumed = 1;
                break;
            case taa_KEY_DELETE:
                if(caret < ((int) txtlen))
                {
                    char* dst = txt + caret;
                    char* src = txt + caret + 1;
                    char* end = txt + txtlen;
                    while(dst < end) { *dst++ = *src++; }
                    --txtlen;
                }
                evt->isconsumed = 1;
                break;
            default:
                {
                    int isvalid = 0;
                    switch(filter)
                    {
                    case taa_UIINPUT_FILTER_NONE:
                        isvalid = 1;
                        break;
                    case taa_UIINPUT_FILTER_NUMERIC:
                        isvalid = isdigit(ascii) || ascii == '-';
                        break;
                    case taa_UIINPUT_FILTER_PRINT:
                        isvalid = isprint(ascii);
                        break;
                    }
                    if(isvalid)
                    {
                        if(txtlen < txtsize - 1)
                        {
                            char* dst = txt + txtlen + 1;
                            char* src = txt + txtlen;
                            char* end = txt + caret;
                            while(dst > end) { *dst-- = *src--; }
                            *dst = ascii;
                            ++caret;
                            ++txtlen;
                        }
                        evt->isconsumed = 1;
                    }
                }
                break;
            }
        }
        else if(evttype == taa_WINDOW_EVENT_MOUSE_BUTTON1_DOWN)
        {
            const char* stritr = txt;
            const char* strend = stritr + txtlen;
            int x = rect->x;
            int clickx = evt->winevent.mouse.cursorx;
            caret = 0;
            // find the character that matches the cursor position
            while(stritr != strend)
            {
                x += font->characters[(uint8_t) (*stritr)].width;
                if(x > clickx)
                {
                    break;
                }
                ++caret;
                ++stritr;
            }
            if((kb->keys[taa_KEY_LSHIFT] | kb->keys[taa_KEY_RSHIFT]) != 0)
            {
                // if the shift key was down, adjust selection
                selmin = (selmin < caret) ? selmin : caret;
                selmax = (selmax > caret) ? selmax : caret;
            }
            else
            {
                selmin = selmax = caret;
            }
            evt->isconsumed = 1;
        }
        else if(evttype == taa_WINDOW_EVENT_MOUSE_MOVE)
        {
            if(evt->winevent.mouse.button1)
            {
                // if dragging the mouse
                // find the character that matches the cursor position
                const char* s = txt;
                if(font != NULL)
                {
                    int xitr = rect->x - caret;
                    while(*s != '\0')
                    {
                        xitr += font->characters[(uint8_t)(*s)].width;
                        if(xitr > evt->winevent.mouse.cursorx)
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
            evt->isconsumed = 1;
        }
        ++evt;
    }
    assert(txtlen < txtsize);
    *txtlen_out = txtlen;
    nextstate->caret = caret;
    nextstate->selectstart = selmin;
    nextstate->selectlength = selmax - selmin;
    return taa_UI_FLAG_FOCUS;
}

//****************************************************************************
static int taa_uiinput_try_focus(
    taa_uiinput_state* prevstate,
    taa_uiinput_state* nextstate,
    int parent,
    int index,
    taa_ui_type type,
    taa_ui_styleid styleid)
{
    int result = 0;
    if(
        prevstate->focusindex == index &&
        prevstate->focuscontrol.type == type &&
        prevstate->focuscontrol.styleid == styleid)
    {
        if( prevstate->focusparent[0]== -1 ||
            prevstate->focusparent[0]==parent ||
            prevstate->focusparent[1]==parent)
        {
            nextstate->focusparent[0] = parent;
            nextstate->focusparent[1] = parent;
            nextstate->focusindex = index;
            result = 1;
        }
    }
    return result;
}

//****************************************************************************
static unsigned int taa_uiinput_vscroll(
    taa_uiinput_state* prevstate,
    taa_uiinput_state* nextstate,
    int* value)
{
    const taa_mouse_state* mouse = &prevstate->mouse;
    taa_uiinput_event* evt = prevstate->events;
    taa_uiinput_event* evtend = evt + prevstate->numevents;
    taa_ui_rect sliderrect = prevstate->focuscontrol.data.scroll.sliderrect;
    int range = prevstate->focuscontrol.data.scroll.range;
    int dy = 0;
    if(mouse->button1)
    {
        int my = mouse->cursory;
        if(prevstate->isdragging)
        {
            // drag the bar if the mouse was clicked within it
            dy = my - prevstate->dragy;
            nextstate->isdragging = 1;
            nextstate->dragx = mouse->cursorx;
            nextstate->dragy = mouse->cursory;
        }
        else
        {
            // page up or down if mouse was clicked in pane
            // TODO: add repeat timer, to support real paging
            // for now just increment every frame
            // dy += (cy < sliderrect.y) ? -barh : barh;
            if(my < sliderrect.y)
            {
                --dy;
            }
            else if(my > (sliderrect.y + sliderrect.h))
            {
                ++dy;
            }
        }
    }
    while(evt != evtend)
    {
        taa_window_eventtype evttype = evt->winevent.type;
        if(evttype == taa_WINDOW_EVENT_KEY_DOWN)
        {
            switch(evt->winevent.key.keycode)
            {
                case taa_KEY_UP:
                    --dy;
                    evt->isconsumed = 1;
                    break;
                case taa_KEY_DOWN:
                    ++dy;
                    evt->isconsumed = 1;
                    break;
                case taa_KEY_PAGE_UP:
                    dy -= sliderrect.h;
                    evt->isconsumed = 1;
                    break;
                case taa_KEY_PAGE_DOWN:
                    dy += sliderrect.h;
                    evt->isconsumed = 1;
                    break;
                default:
                    break;
            }
        }
        else if(evttype == taa_WINDOW_EVENT_MOUSE_BUTTON1_DOWN)
        {
            if(taa_uiinput_testrect(
                evt->winevent.mouse.cursorx,
                evt->winevent.mouse.cursory,
                &sliderrect))
            {
                // if mouse was pressed within slider, begin scrolling
                nextstate->isdragging = 1;
                nextstate->dragx = evt->winevent.mouse.cursorx;
                nextstate->dragy = evt->winevent.mouse.cursory;
            }
        }
        ++evt;
    }
    if(dy != 0)
    {
        int paneh = prevstate->focuscontrol.data.scroll.sliderpane.h;
        if(paneh > sliderrect.h)
        {
            int val = (*value) + (dy*range)/(paneh - sliderrect.h);
            if(val < 0)
            {
                val = 0;
            }
            if(val > range)
            {
                val = range;
            }
            *value = val;
        }
    }
    return taa_UI_FLAG_FOCUS;
}

#endif // taa_UIINPUT_H_
