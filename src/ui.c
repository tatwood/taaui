/**
 * @brief     ui simulation implementation
 * @author    Thomas Atwood (tatwood.net)
 * @date      2011
 * @copyright unlicense / public domain
 ****************************************************************************/
#include "uiinput.h"
#include "uilayout.h"
#include <taa/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//****************************************************************************
typedef struct taa_ui_stack_s taa_ui_stack;

struct taa_ui_stack_s
{
    int control;
    int layoutcmd;
    int scrollx;
    int scrolly;
};

struct taa_ui_s
{
    const taa_ui_style* stylesheet;
    taa_ui_stack* stack;
    taa_ui_control* ctrlbuffer;
    taa_uilayout_list layout;
    // state maintained across 2 frames
    taa_uiinput_state nextstate;
    taa_uiinput_state prevstate;
    int viewwidth;
    int viewheight;
    int ctrlcursor;
    unsigned int idcounter;
    // control buffers
    size_t numstyles;
    size_t stackdepth;
    size_t textoffset;
    size_t stackcapacity;
    size_t ctrlcapacity;
    size_t textcapacity;
    char* textbuffer;
    taa_ui_controllist ctrllist;
};

//****************************************************************************
static int taa_ui_push_id_control(
    taa_ui* ui,
    taa_ui_type type,
    taa_ui_styleid styleid,
    unsigned int flags,
    unsigned int id)
{
    int index = -1;
    if(ui->ctrlcursor >= 0)
    {
        taa_ui_control* ctrl;
        index = ui->ctrlcursor;
        ctrl = ui->ctrlbuffer + index;
        ctrl->type = type;
        ctrl->styleid = styleid;
        ctrl->flags = flags;
        ctrl->data.type = taa_UI_DATA_ID;
        ctrl->data.id.id = id;
        --ui->ctrlcursor;
    }
    else
    {
        taa_LOG_WARN("exceeded ui control limit");
    }
    return index;
}

//****************************************************************************
static int taa_ui_push_range_control(
    taa_ui* ui,
    taa_ui_type type,
    taa_ui_styleid panestyleid,
    taa_ui_styleid sliderstyleid,
    unsigned int flags,
    int range,
    int value)
{
    int index = -1;
    if(ui->ctrlcursor >= 0)
    {
        taa_ui_control* ctrl;
        index = ui->ctrlcursor;
        ctrl = ui->ctrlbuffer + index;
        ctrl->type = type;
        ctrl->styleid = panestyleid;
        ctrl->flags = flags;
        ctrl->data.type = taa_UI_DATA_SCROLL;
        ctrl->data.scroll.sliderstyleid = sliderstyleid;
        ctrl->data.scroll.range = range;
        ctrl->data.scroll.value = value;
        --ui->ctrlcursor;
    }
    else
    {
        taa_LOG_WARN("exceeded ui control limit");
    }
    return index;
}

//****************************************************************************
static int taa_ui_push_text_control(
    taa_ui* ui,
    taa_ui_type type,
    taa_ui_styleid styleid,
    unsigned int flags,
    const char* txt,
    size_t txtlen,
    size_t txtcapacity)
{
    int index = -1;
    if(ui->ctrlcursor >= 0)
    {
        taa_ui_control* ctrl;
        char* dsttxt;
        size_t maxtxt;
        maxtxt = ui->textcapacity - ui->textoffset;
        if(txtlen > maxtxt)
        {
            txtlen = maxtxt;
            taa_LOG_WARN("exceeded ui text limit");
        }
        dsttxt = ui->textbuffer + ui->textoffset;
        memcpy(dsttxt, txt, txtlen);
        index = ui->ctrlcursor;
        ctrl = ui->ctrlbuffer + index;
        ctrl->type = type;
        ctrl->styleid = styleid;
        ctrl->flags = flags;
        ctrl->data.type = taa_UI_DATA_TEXT;
        ctrl->data.text.text = dsttxt;
        ctrl->data.text.textlength = txtlen;
        ctrl->data.text.textcapacity = txtcapacity;
        ui->textoffset += txtlen;
        --ui->ctrlcursor;
    }
    else
    {
        taa_LOG_WARN("exceeded ui control limit");
    }
    return index;
}

//****************************************************************************
static int32_t taa_ui_push_stack(
    taa_ui* ui,
    int32_t control,
    int32_t layoutcmd,
    int32_t scrollx,
    int32_t scrolly)
{
    uint32_t index = ui->stackdepth + 1;
    if(index < ui->stackcapacity)
    {
        taa_ui_stack* s = ui->stack + index;
        s->control = control;
        s->layoutcmd = layoutcmd;
        s->scrollx = scrollx;
        s->scrolly = scrolly;
    }
    ++ui->stackdepth;
    return index;
}

//****************************************************************************
static void taa_ui_pop_stack(
    taa_ui* ui)
{
    --ui->stackdepth;
    assert(ui->stackdepth >= 0);
}

//****************************************************************************
void taa_ui_begin(
    taa_ui* ui,
    int vieww,
    int viewh,
    const taa_keyboard_state* kb,
    const taa_mouse_state* mouse,
    const taa_window_event* winevents,
    int numevents)
{
    taa_ui_stack* s = ui->stack;
    ui->viewwidth = vieww;
    ui->viewheight = viewh;
    ui->stackdepth = 0;
    ui->ctrlcursor = ui->ctrlcapacity - 1;
    ui->textoffset = 0;
    s->control = -1;
    s->layoutcmd = -1;
    taa_uiinput_begin(
        &ui->prevstate,
        &ui->nextstate,
        kb,
        mouse,
        winevents,
        numevents);
    taa_uilayout_begin(&ui->layout);
}

//****************************************************************************
taa_ui_handle taa_ui_button(
    taa_ui* ui,
    taa_ui_styleid styleid,
    unsigned int flags,
    const taa_ui_rect* rect,
    const char* txt,
    unsigned int* flags_out)
{
    const taa_ui_stack* p = ui->stack + ui->stackdepth;
    const taa_ui_style* style = ui->stylesheet + styleid;
    uint32_t txtlen = strlen(txt);
    int32_t control;
    int32_t layoutcmd;
    if(taa_uiinput_try_focus(
        &ui->prevstate,
        &ui->nextstate,
        p->control,
        ui->ctrlcursor,
        taa_UI_BUTTON,
        styleid))
    {
        flags |= taa_uiinput_button(&ui->prevstate);
    }
    else
    {
        flags &= ~taa_UI_FLAG_FOCUS;
    }
    control = taa_ui_push_text_control(
        ui,
        taa_UI_BUTTON,
        styleid,
        flags,
        txt,
        txtlen,
        txtlen);
    layoutcmd = taa_uilayout_push(
        &ui->layout,
        taa_UILAYOUT_ABS,
        style->halign,
        style->valign,
        0,
        p->layoutcmd,
        -1,
        control,
        0,
        0,
        rect);
    taa_uilayout_pop(&ui->layout, layoutcmd, -1);
    if(flags_out != NULL)
    {
        *flags_out = flags;
    }
    return layoutcmd;
}

//****************************************************************************
void taa_ui_create(
    size_t stacksize,
    size_t maxcontrols,
    size_t maxtext,
    taa_ui_style* stylesheet,
    size_t numstyles,
    taa_ui** ui_out)
{
    void* buf;
    taa_ui* ui;
    taa_ui_stack* stack;
    taa_ui_control* ctrl;
    taa_uilayout_cmd* layout;
    char* text;
    // calculate structure offsets
    buf = NULL;
    ui = (taa_ui*) buf;
    buf = ui + 1;
    stack = (taa_ui_stack*) taa_ALIGN_PTR(buf, 8);
    buf = stack + stacksize;
    ctrl = (taa_ui_control*) taa_ALIGN_PTR(buf, 8);
    buf = ctrl + maxcontrols;
    layout = (taa_uilayout_cmd*) taa_ALIGN_PTR(buf, 8);
    buf = layout + maxcontrols;
    text = (char*) taa_ALIGN_PTR(buf, 8);
    buf = text + maxtext;
    // allocate buffer and adjust pointers
    buf = malloc((size_t) buf);
    ui = (taa_ui*) (((ptrdiff_t) ui) + ((ptrdiff_t) buf));
    stack = (taa_ui_stack*) (((ptrdiff_t) stack) + ((ptrdiff_t) buf));
    ctrl = (taa_ui_control*) (((ptrdiff_t) ctrl) + ((ptrdiff_t) buf));
    layout = (taa_uilayout_cmd*) (((ptrdiff_t) layout) + ((ptrdiff_t) buf));
    text = (char*)  (((ptrdiff_t) text) + ((ptrdiff_t) buf));
    // initialize structures
    memset(ui, 0, sizeof(*ui));
    ui->stylesheet = stylesheet;
    ui->stack = stack;
    ui->ctrlbuffer = ctrl;
    ui->textbuffer = text;
    ui->numstyles = numstyles;
    ui->stackcapacity = stacksize;
    ui->ctrlcapacity = maxcontrols;
    ui->textcapacity = maxtext;
    ui->layout.cmds = layout;
    ui->layout.capacity = maxcontrols;
    // set out parameter
    *ui_out = ui;
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
    taa_ui_rect screenrect = { 0, 0, ui->viewwidth, ui->viewheight };
    taa_ui_controllist* cl = &ui->ctrllist;
    size_t firstcontrol = ui->ctrlcursor + 1;
    size_t numcontrols = ui->ctrlcapacity - firstcontrol;
    assert(ui->stackdepth == 0);
    taa_uilayout_end(&ui->layout, ui->stylesheet,ui->ctrlbuffer,&screenrect);
    taa_uiinput_end(
        &ui->prevstate,
        &ui->nextstate,
        ui->ctrlbuffer,
        firstcontrol,
        numcontrols);
    cl->controls = ui->ctrlbuffer + firstcontrol;
    cl->numcontrols = numcontrols;
    cl->caret = ui->prevstate.caret;
    cl->selectstart = ui->prevstate.selectstart;
    cl->selectlength = ui->prevstate.selectlength;
    cl->viewwidth = ui->viewwidth;
    cl->viewheight = ui->viewheight;
    return cl;
}

//****************************************************************************
unsigned int taa_ui_generate_id(
    taa_ui* ui)
{
    return ++ui->idcounter;
}

//****************************************************************************
taa_ui_handle taa_ui_label(
    taa_ui* ui,
    taa_ui_styleid styleid,
    const taa_ui_rect* rect,
    const char* txt)
{
    const taa_ui_stack* p = ui->stack + ui->stackdepth;
    const taa_ui_style* style = ui->stylesheet + styleid;
    uint32_t txtlen = strlen(txt);
    int32_t control;
    int32_t layoutcmd;
    control = taa_ui_push_text_control(
        ui,
        taa_UI_LABEL,
        styleid,
        taa_UI_FLAG_DISABLED,
        txt,
        txtlen,
        txtlen);
    layoutcmd = taa_uilayout_push(
        &ui->layout,
        taa_UILAYOUT_ABS,
        style->halign,
        style->valign,
        0,
        p->layoutcmd,
        -1,
        control,
        0,
        0,
        rect);
    taa_uilayout_pop(&ui->layout, layoutcmd, -1);
    return layoutcmd;
}

//****************************************************************************
taa_ui_handle taa_ui_numberbox(
    taa_ui* ui,
    taa_ui_styleid styleid,
    unsigned int flags,
    const taa_ui_rect* rect,
    int min,
    int max,
    int* value,
    unsigned int* flags_out)
{
    const taa_ui_stack* p = ui->stack + ui->stackdepth;
    const taa_ui_style* style = ui->stylesheet + styleid;
    char txt[12];
    int val;
    int control;
    int layoutcmd;
    size_t txtlen;
    size_t txtsize;
    size_t i;
    val = *value;
    // if the value is zero, keep the text empty until after input is
    // evaluated. this prevents an additional trailing zero from being added
    // to the actual input number
    if(val != 0)
    {
        sprintf(txt, "%u", val);
    }
    else
    {
        txt[0] = '\0';
    }
    i = abs(max);
    // determine the maximum number of characters the textbox may contain
    if(((unsigned int) abs(min)) > i)
    {
        i = abs(min);
    }
    txtsize = 1;
    while(i > 0)
    {
        ++txtsize;
        // technically should be 10, but using 8 for speed. the only effects
        // of this inaccuracy, is that the numberbox may be slightly wider
        i /= 8; 
    }
    assert(txtsize <= sizeof(txt));
    if(taa_uiinput_try_focus(
        &ui->prevstate,
        &ui->nextstate,
        p->control,
        ui->ctrlcursor,
        taa_UI_NUMBERBOX,
        styleid))
    {
        flags |= taa_uiinput_text(
            &ui->prevstate,
            &ui->nextstate,
            taa_UIINPUT_FILTER_NUMERIC,
            style->font,
            txt,
            txtsize,
            &txtlen);
        val = atoi(txt);
        if(val < min || val > max)
        {
            val = *value;
            sprintf(txt, "%u", val);
        }
        else
        {
            *value = val;
        }
    }
    else
    {
        flags &= ~taa_UI_FLAG_FOCUS;
        txtlen = strlen(txt);
    }
    // if the value is still zero, fix the text so the number "0" displays
    if(val == 0)
    {
        txt[0] = '0';
        txtlen = 1;
    }
    control = taa_ui_push_text_control(
        ui,
        taa_UI_NUMBERBOX,
        styleid,
        flags,
        txt,
        txtlen,
        txtsize - 1);
    layoutcmd = taa_uilayout_push(
        &ui->layout,
        taa_UILAYOUT_ABS,
        style->halign,
        style->valign,
        0,
        p->layoutcmd,
        -1,
        control,
        0,
        0,
        rect);
    taa_uilayout_pop(&ui->layout, layoutcmd, -1);
    if(flags_out != NULL)
    {
        *flags_out = flags;
    }
    return layoutcmd;
}

//****************************************************************************
void taa_ui_pop_cols(
    taa_ui* ui)
{
    const taa_ui_stack* s = ui->stack + ui->stackdepth;
    taa_uilayout_pop(&ui->layout, s->layoutcmd, -1);
    taa_ui_pop_stack(ui);
}

//****************************************************************************
taa_ui_handle taa_ui_pop_container(
    taa_ui* ui,
    int* scrollx_out,
    int* scrolly_out,
    unsigned int* flags_out)
{
    const taa_ui_stack* s = ui->stack + ui->stackdepth;
    const taa_ui_stack* p = s - 1;
    taa_ui_control* endctrl;
    unsigned int flags;
    int focusindex;
    int beginindex;
    int endindex;
    int layoutcmd;
    int scrollx;
    int scrolly;
    beginindex = ui->ctrlcursor;
    endindex = s->control;
    endctrl = ui->ctrlbuffer + endindex;
    flags = endctrl->flags;
    focusindex = ui->prevstate.focusindex;
    layoutcmd = s->layoutcmd;
    scrollx = s->scrollx;
    scrolly = s->scrolly;
    // do not attempt to grab focus for the container until the pop
    // container function. this allows children the opportunity to grab it
    // first
    if(taa_uiinput_try_focus(
            &ui->prevstate,
            &ui->nextstate,
            p->control,
            ui->ctrlcursor,
            taa_UI_CONTAINER_BEGIN,
            endctrl->styleid))
    {
        // treat the focus input as if the container is a button
        flags |= taa_uiinput_button(&ui->prevstate);
    }
    else if(focusindex >= beginindex && focusindex <= endindex)
    {
        // if one of the children is focused, propogate the flag up
        flags |= taa_UI_FLAG_FOCUS;
        if(ui->nextstate.focusparent[1] == endindex)
        {
            // if this is the immediate parent of the focused control, fix the
            // focus parent range and attempt to scroll the focused child
            // into view if it is clipped
            taa_ui_rect* cliprect = &ui->prevstate.focuscontrol.cliprect;
            taa_ui_rect* focusrect = &ui->prevstate.focuscontrol.rect;
            int clipx;
            int childx;
            int clipy;
            int childy;
            ui->nextstate.focusparent[0] = beginindex;
            clipx = cliprect->x;
            childx = focusrect->x;
            if(clipx > childx)
            {
                scrollx -= clipx - childx;
            }
            clipx += cliprect->w;
            childx += focusrect->w;
            if(clipx < childx)
            {
                scrollx += childx - clipx;
            }
            clipy = cliprect->y;
            childy = focusrect->y;
            if(clipy > childy)
            {
                scrolly -= clipy - childy;
            }
            clipy += cliprect->h;
            childy += focusrect->h;
            if(clipy < childy)
            {
                scrolly += childy - clipy;
            }
            // keep layout consistent
            ui->layout.cmds[layoutcmd].scrollx = scrollx;
            ui->layout.cmds[layoutcmd].scrolly = scrolly;
        }
    }
    // make sure the flags are consistent for both begin and end control
    endctrl->flags = flags;
    // because the control buffer is filled in reverse order, the container
    // begin control is added in the pop container function
    beginindex = taa_ui_push_id_control(
        ui,
        taa_UI_CONTAINER_BEGIN,
        endctrl->styleid,
        flags,
        endctrl->data.id.id);
    taa_uilayout_pop(
        &ui->layout,
        layoutcmd,
        beginindex);
    taa_ui_pop_stack(ui);
    if(scrollx_out != NULL)
    {
        *scrollx_out = scrollx;
    }
    if(scrolly_out != NULL)
    {
        *scrolly_out = scrolly;
    }
    if(flags_out != NULL)
    {
        *flags_out = flags;
    }
    return layoutcmd;
}

//****************************************************************************
void taa_ui_pop_rect(
    taa_ui* ui)
{
    const taa_ui_stack* s = ui->stack + ui->stackdepth;
    taa_uilayout_pop(&ui->layout, s->layoutcmd, -1);
    taa_ui_pop_stack(ui);
}

//****************************************************************************
void taa_ui_pop_rows(
    taa_ui* ui)
{
    const taa_ui_stack* s = ui->stack + ui->stackdepth;
    taa_uilayout_pop(&ui->layout, s->layoutcmd, -1);
    taa_ui_pop_stack(ui);
}

//****************************************************************************
void taa_ui_push_cols(
    taa_ui* ui,
    taa_ui_valign valign,
    int spacing,
    const taa_ui_rect* rect)
{
    const taa_ui_stack* p = ui->stack + ui->stackdepth;
    int layoutcmd;
    layoutcmd = taa_uilayout_push(
        &ui->layout,
        taa_UILAYOUT_COLS,
        taa_UI_HALIGN_LEFT,
        valign,
        spacing,
        p->layoutcmd,
        -1,
        -1,
        0,
        0,
        rect);
    taa_ui_push_stack(ui, p->control, layoutcmd, 0, 0);
}

//****************************************************************************
void taa_ui_push_container(
    taa_ui* ui,
    taa_ui_styleid styleid,
    unsigned int flags,
    const taa_ui_rect* rect,
    int scrollx,
    int scrolly,
    unsigned int id)
{
    const taa_ui_stack* p = ui->stack + ui->stackdepth;
    const taa_ui_style* style = ui->stylesheet + styleid;
    int endcontrol;
    int layoutcmd;
    // because the control buffer is filled in reverse order, the container
    // end control is added in the push container function
    endcontrol = taa_ui_push_id_control(
        ui,
        taa_UI_CONTAINER_END,
        styleid,
        flags,
        id);
    layoutcmd = taa_uilayout_push(
        &ui->layout,
        taa_UILAYOUT_ABS,
        style->halign,
        style->valign,
        0,
        p->layoutcmd,
        -1,
        endcontrol,
        scrollx,
        scrolly,
        rect);
    taa_ui_push_stack(ui, endcontrol, layoutcmd, scrollx, scrolly);
}

//****************************************************************************
void taa_ui_push_rect(
    taa_ui* ui,
    taa_ui_halign halign,
    taa_ui_valign valign,
    const taa_ui_rect* rect)
{
    const taa_ui_stack* p = ui->stack + ui->stackdepth;
    int layoutcmd;
    layoutcmd = taa_uilayout_push(
        &ui->layout,
        taa_UILAYOUT_ABS,
        halign,
        valign,
        0,
        p->layoutcmd,
        -1,
        -1,
        0,
        0,
        rect);
    taa_ui_push_stack(ui, p->control, layoutcmd, 0, 0);
}

//****************************************************************************
void taa_ui_push_rows(
    taa_ui* ui,
    taa_ui_halign halign,
    int spacing,
    const taa_ui_rect* rect)
{
    const taa_ui_stack* p = ui->stack + ui->stackdepth;
    int layoutcmd;
    layoutcmd = taa_uilayout_push(
        &ui->layout,
        taa_UILAYOUT_ROWS,
        halign,
        taa_UI_VALIGN_TOP,
        spacing,
        p->layoutcmd,
        -1,
        -1,
        0,
        0,
        rect);
    taa_ui_push_stack(ui, p->control, layoutcmd, 0, 0);
}

//****************************************************************************
taa_ui_handle taa_ui_textbox(
    taa_ui* ui,
    taa_ui_styleid styleid,
    unsigned int flags,
    const taa_ui_rect* rect,
    char* txt,
    size_t txtsize,
    unsigned int* flags_out)
{
    const taa_ui_stack* p = ui->stack + ui->stackdepth;
    const taa_ui_style* style = ui->stylesheet + styleid;
    int control;
    int layoutcmd;
    size_t txtlen;
    if(taa_uiinput_try_focus(
        &ui->prevstate,
        &ui->nextstate,
        p->control,
        ui->ctrlcursor,
        taa_UI_TEXTBOX,
        styleid))
    {
        flags |= taa_uiinput_text(
            &ui->prevstate,
            &ui->nextstate,
            taa_UIINPUT_FILTER_PRINT,
            style->font,
            txt,
            txtsize,
            &txtlen);
    }
    else
    {
        flags &= ~taa_UI_FLAG_FOCUS;
        txtlen = strlen(txt);
    }
    control = taa_ui_push_text_control(
        ui,
        taa_UI_TEXTBOX,
        styleid,
        flags,
        txt,
        txtlen,
        txtsize - 1);
    layoutcmd = taa_uilayout_push(
        &ui->layout,
        taa_UILAYOUT_ABS,
        style->halign,
        style->valign,
        0,
        p->layoutcmd,
        -1,
        control,
        0,
        0,
        rect);
    taa_uilayout_pop(&ui->layout, layoutcmd, -1);
    if(flags_out != NULL)
    {
        *flags_out = flags;
    }
    return layoutcmd;
}

//****************************************************************************
taa_ui_handle taa_ui_vscrollbar(
    taa_ui* ui,
    taa_ui_styleid panestyleid,
    taa_ui_styleid sliderstyleid,
    unsigned int flags,
    const taa_ui_rect* rect,
    taa_ui_handle target,
    int* value,
    unsigned int* flags_out)
{
    const taa_ui_stack* p = ui->stack + ui->stackdepth;
    const taa_ui_style* style = ui->stylesheet + panestyleid;
    int control;
    int layoutcmd;
    flags &= taa_UI_FLAG_DISABLED;
    if((flags & taa_UI_FLAG_DISABLED) == 0)
    {
        if(taa_uiinput_try_focus(
            &ui->prevstate,
            &ui->nextstate,
            p->control,
            ui->ctrlcursor,
            taa_UI_VSCROLLBAR,
            panestyleid))
        {
            flags |= taa_uiinput_vscroll(&ui->prevstate,&ui->nextstate,value);
        }
    }
    control = taa_ui_push_range_control(
        ui,
        taa_UI_VSCROLLBAR,
        panestyleid,
        sliderstyleid,
        flags,
        0, // range has to be auto calculated later, when layout is known
        *value);
    layoutcmd = taa_uilayout_push(
        &ui->layout,
        taa_UILAYOUT_ABS,
        style->halign,
        style->valign,
        0,
        p->layoutcmd,
        (int) target,
        control,
        0,
        0,
        rect);
    taa_uilayout_pop(&ui->layout, layoutcmd, -1);
    if(flags_out != NULL)
    {
        *flags_out = flags;
    }
    return layoutcmd;
}
