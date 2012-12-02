/**
 * @brief     ui layout implementation
 * @author    Thomas Atwood (tatwood.net)
 * @date      2011
 * @copyright unlicense / public domain
 ****************************************************************************/
#ifndef taa_UILAYOUT_H_
#define taa_UILAYOUT_H_

#include <taa/ui.h>
#include <assert.h>
#include <limits.h>

//****************************************************************************
// enums

enum taa_uilayout_mode_e
{
    taa_UILAYOUT_ABS,
    taa_UILAYOUT_COLS,
    taa_UILAYOUT_ROWS
};

//****************************************************************************
// typedefs

typedef enum taa_uilayout_mode_e taa_uilayout_mode;

typedef struct taa_uilayout_cmd_s taa_uilayout_cmd;
typedef struct taa_uilayout_list_s taa_uilayout_list;

//****************************************************************************
// structs

struct taa_uilayout_cmd_s
{
    taa_uilayout_mode mode;
    taa_ui_halign halign;
    taa_ui_valign valign;
    taa_ui_rect rect;
    taa_ui_rect contentrect;
    int spacing;
    int scrollx;
    int scrolly;
    int parentcmd;
    int targetcmd;
    int childbegin;
    int childend;
    int control0;
    int control1;
    int autow;
    int autoh;
    int childrenw;
    int childrenh;
};

struct taa_uilayout_list_s
{
    taa_uilayout_cmd* cmds;
    size_t capacity;
    size_t size;
};

//****************************************************************************
// functions

//****************************************************************************
static void taa_uilayout_intersect(
    const taa_ui_rect* a,
    const taa_ui_rect* b,
    taa_ui_rect* r_out)
{
    int xl = (a->x > b->x) ? a->x : b->x;
    int xr = (a->x+a->w < b->x+b->w) ? a->x+a->w : b->x+b->w;
    int yt = (a->y > b->y) ? a->y : b->y;
    int yb = (a->y+a->h < b->y+b->h) ? a->y+a->h : b->y+b->h;
    r_out->x = xl;
    r_out->y = yt;
    r_out->w = xr - xl;
    r_out->h = yb - yt;
}

//****************************************************************************
static void taa_uilayout_union(
    const taa_ui_rect* a,
    const taa_ui_rect* b,
    taa_ui_rect* r_out)
{
    int xl = (a->x < b->x) ? a->x : b->x;
    int xr = (a->x+a->w > b->x+b->w) ? a->x+a->w : b->x+b->w;
    int yt = (a->y < b->y) ? a->y : b->y;
    int yb = (a->y+a->h > b->y+b->h) ? a->y+a->h : b->y+b->h;
    r_out->x = xl;
    r_out->y = yt;
    r_out->w = xr - xl;
    r_out->h = yb - yt;
}

//****************************************************************************
static int taa_uilayout_auto_height(
    taa_uilayout_cmd* cmds,
    taa_uilayout_cmd* cmd,
    const taa_ui_style* styles,
    const taa_ui_control* controls)
{
    int h = 0;
    if(cmd->control0 >= 0)
    {
        const taa_ui_control* control = controls + cmd->control0;
        const taa_ui_style* style = styles + control->styleid;
        h = style->tborder+style->tpadding+style->bpadding+style->bborder;
        switch(control->type)
        {
        case taa_UI_BUTTON:
            h += style->font->charheight;
            break;
        case taa_UI_LABEL:
            h += style->font->charheight;
            break;
        case taa_UI_NUMBERBOX:
            h += style->font->charheight;
            break;
        case taa_UI_TEXTBOX:
            h += style->font->charheight;
            break;
        case taa_UI_VSCROLLBAR:
            {
                const taa_ui_style* sliderstyle;
                sliderstyle = styles + control->data.scroll.sliderstyleid;
                h +=
                    sliderstyle->tborder +
                    sliderstyle->tpadding +
                    sliderstyle->bpadding +
                    sliderstyle->bborder;
            }
            break;
        default:
            break;
        }
    }
    if(cmd->childbegin != cmd->childend)
    {
        // if it's an auto sized container, it needs to fit its children
        // plus space for borders and padding. if any children are set to
        // fill, the fill space will have no effect on the container size
        taa_uilayout_mode mode = cmd->mode;
        int childitr = cmd->childbegin;
        int childend = cmd->childend;
        int contenth = 0;
        taa_ui_rect childrect;
        childrect.x = INT_MAX;
        childrect.y = INT_MAX;
        childrect.w = INT_MIN;
        childrect.h = INT_MIN;
        while(childitr != childend)
        {
            const taa_uilayout_cmd* child = cmds + childitr;
            assert(child->parentcmd == (ptrdiff_t) (cmd - cmds));
            assert(child->autoh >= 0);
            if(mode == taa_UILAYOUT_ROWS)
            {
                contenth += child->autoh + cmd->spacing;
            }
            else
            {
                taa_ui_rect tmprect;
                tmprect = child->rect;
                tmprect.h = child->autoh;
                taa_uilayout_union(&tmprect,&childrect,&childrect);
                contenth = childrect.y + childrect.h;
            }
            childitr = child->childend;
        }
        if(mode == taa_UILAYOUT_ROWS && contenth > 0)
        {
            // an extra mount of spacing was added to the last row
            // remove it
            contenth -= cmd->spacing;
        }
        h += contenth;
    }
    return h;
}

//****************************************************************************
static int taa_uilayout_auto_width(
    taa_uilayout_cmd* cmds,
    taa_uilayout_cmd* cmd,
    const taa_ui_style* styles,
    const taa_ui_control* controls)
{
    int w = 0;
    if(cmd->control0 >= 0)
    {
        const taa_ui_control* control = controls + cmd->control0;
        const taa_ui_style* style = styles + control->styleid;
        w = style->lborder+style->lpadding+style->rpadding+style->rborder;
        switch(control->type)
        {
        case taa_UI_BUTTON:
            w += taa_ui_calc_font_width(
                style->font,
                control->data.text.text,
                control->data.text.textlength);
            break;
        case taa_UI_LABEL:
            w += taa_ui_calc_font_width(
                style->font,
                control->data.text.text,
                control->data.text.textlength);
            break;
        case taa_UI_NUMBERBOX:
            w += style->font->maxcharwidth * control->data.text.textcapacity;
            break;
        case taa_UI_TEXTBOX:
            w += style->font->maxcharwidth * control->data.text.textcapacity;
            break;
        case taa_UI_VSCROLLBAR:
            {
                const taa_ui_style* sliderstyle;
                sliderstyle = styles + control->data.scroll.sliderstyleid;
                w +=
                    sliderstyle->lborder +
                    sliderstyle->lpadding +
                    sliderstyle->rpadding +
                    sliderstyle->rborder;
            }
            break;
        default:
            break;
        }
    }
    if(cmd->childbegin != cmd->childend)
    {
        // if it's an auto sized container, it needs to fit its children plus
        // space for borders and padding. if any children are set to fill, the
        // fill space will have no effect on the container size
        taa_uilayout_mode mode = cmd->mode;
        int childitr = cmd->childbegin;
        int childend = cmd->childend;
        int contentw = 0;
        taa_ui_rect childrect;
        childrect.x = INT_MAX;
        childrect.y = INT_MAX;
        childrect.w = INT_MIN;
        childrect.h = INT_MIN;
        while(childitr != childend)
        {
            const taa_uilayout_cmd* child = cmds + childitr;
            assert(child->parentcmd == (ptrdiff_t) (cmd - cmds));
            assert(child->autow >= 0);
            if(mode == taa_UILAYOUT_COLS)
            {
                contentw += child->autow + cmd->spacing;
            }
            else
            {
                taa_ui_rect tmprect;
                tmprect = child->rect;
                tmprect.w = child->autow;
                taa_uilayout_union(&tmprect,&childrect,&childrect);
                contentw = childrect.x + childrect.w;
            }
            childitr = child->childend;
        }
        if(mode == taa_UILAYOUT_COLS && contentw > 0)
        {
            // an extra mount of spacing was added to the last column
            // remove it
            contentw -= cmd->spacing;
        }
        w += contentw;
    }
    return w;
}

//****************************************************************************
static void taa_uilayout_abs(
        taa_uilayout_cmd* cmds,
        taa_ui_halign halign,
        taa_ui_valign valign,
        const taa_ui_rect* contentrect,
        int begin,
        int end)
{
    int itr = begin;
    while(itr != end)
    {
        taa_uilayout_cmd* cmd = cmds + itr;
        if(cmd->rect.w == taa_UI_WIDTH_FILL)
        {
            cmd->rect.w = contentrect->w;
        }
        if(cmd->rect.h == taa_UI_HEIGHT_FILL)
        {
            cmd->rect.h = contentrect->h;
        }
        assert(cmd->rect.w >= 0);
        assert(cmd->rect.h >= 0);
        switch(halign)
        {
        case taa_UI_HALIGN_LEFT:
            cmd->rect.x += contentrect->x;
            break;
        case taa_UI_HALIGN_RIGHT:
            {
                int xoffset = cmd->rect.x + cmd->rect.w;
                cmd->rect.x = contentrect->x + contentrect->w - xoffset;
            }
            break;
        case taa_UI_HALIGN_CENTER:
            {
                int xoffset = cmd->rect.x - (cmd->rect.w/2);
                cmd->rect.x = contentrect->x + (contentrect->w/2) + xoffset;
            }
            break;
        }
        switch(valign)
        {
        case taa_UI_VALIGN_TOP:
            cmd->rect.y += contentrect->y;
            break;
        case taa_UI_VALIGN_BOTTOM:
            {
                int yoffset = cmd->rect.y + cmd->rect.h;
                cmd->rect.y = contentrect->y + contentrect->h - yoffset;
            }
            break;
        case taa_UI_VALIGN_CENTER:
            {
                int yoffset = cmd->rect.y - (cmd->rect.h/2);
                cmd->rect.y = contentrect->y + (contentrect->h/2) + yoffset;
            }
            break;
        }
        itr = cmd->childend;
    }
}

//****************************************************************************
static void taa_uilayout_cols(
        taa_uilayout_cmd* cmds,
        taa_ui_valign valign,
        int spacing,
        const taa_ui_rect* contentrect,
        int begin,
        int end)
{
    int itr = begin;
    int fixedw = 0;
    int fillw = 0;
    int numfill = 0;
    int x = 0;
    // first pass, determine available fill space and number of fill controls
    while(itr != end)
    {
        taa_uilayout_cmd* cmd = cmds + itr;
        if(cmd->rect.w == taa_UI_WIDTH_FILL)
        {
            ++numfill;
        }
        else
        {
            fixedw += cmd->rect.w;
        }
        fixedw += spacing;
        itr = cmd->childend;
    }
    // remove the right-most spacing so that it only exists between columns
    fixedw -= spacing;
    // divide available fill width by number of controls that want to fill
    fillw = (contentrect->w > fixedw) ? contentrect->w - fixedw : 0;
    if(numfill > 1)
    {
        fillw /= numfill;
    }
    // second pass, set fill widths and calculate coordinates
    x = contentrect->x;
    itr = begin;
    while(itr != end)
    {
        taa_uilayout_cmd* cmd = cmds + itr;
        if(cmd->rect.w == taa_UI_WIDTH_FILL)
        {
            cmd->rect.w = fillw;
        }
        if(cmd->rect.h == taa_UI_HEIGHT_FILL)
        {
            cmd->rect.h = contentrect->h;
        }
        assert(cmd->rect.w >= 0);
        assert(cmd->rect.h >= 0);
        cmd->rect.x = x;
        x += cmd->rect.w + spacing;
        switch(valign)
        {
        case taa_UI_VALIGN_TOP:
            cmd->rect.y += contentrect->y;
            break;
        case taa_UI_VALIGN_BOTTOM:
            {
                int yoffset = cmd->rect.y + cmd->rect.h;
                cmd->rect.y = contentrect->y + contentrect->h - yoffset;
            }
            break;
        case taa_UI_VALIGN_CENTER:
            {
                int yoffset = cmd->rect.y - (cmd->rect.h/2);
                cmd->rect.y = contentrect->y + (contentrect->h/2) + yoffset;
            }
            break;
        }
        itr = cmd->childend;
    }
}

//****************************************************************************
static void taa_uilayout_rows(
        taa_uilayout_cmd* cmds,
        taa_ui_halign halign,
        int spacing,
        const taa_ui_rect* contentrect,
        int begin,
        int end)
{
    int itr = begin;
    int fixedh = 0;
    int fillh = 0;
    int numfill = 0;
    int y = 0;
    // first pass, determine available fill space and number of fill controls
    while(itr != end)
    {
        taa_uilayout_cmd* cmd = cmds + itr;
        if(cmd->rect.h == taa_UI_HEIGHT_FILL)
        {
            ++numfill;
        }
        else
        {
            fixedh += cmd->rect.h;
        }
        fixedh += spacing;
        itr = cmd->childend;
    }
    // remove the bottom-most spacing so that it only exists between rows
    fixedh -= spacing;
    // divide available fill height by number of controls that want to fill
    fillh = (contentrect->h > fixedh) ? contentrect->h - fixedh : 0;
    if(numfill > 1)
    {
        fillh /= numfill;
    }
    // second pass, set fill widths and calculate coordinates
    y = contentrect->y;
    itr = begin;
    while(itr != end)
    {
        taa_uilayout_cmd* cmd = cmds + itr;
        if(cmd->rect.w == taa_UI_WIDTH_FILL)
        {
            cmd->rect.w = contentrect->w;
        }
        if(cmd->rect.h == taa_UI_HEIGHT_FILL)
        {
            cmd->rect.h = fillh;
        }
        assert(cmd->rect.w >= 0);
        assert(cmd->rect.h >= 0);
        switch(halign)
        {
        case taa_UI_HALIGN_LEFT:
            cmd->rect.x += contentrect->x;
            break;
        case taa_UI_HALIGN_RIGHT:
            {
                int xoffset = cmd->rect.x + cmd->rect.w;
                cmd->rect.x = contentrect->x + contentrect->w - xoffset;
            }
            break;
        case taa_UI_HALIGN_CENTER:
            {
                int xoffset = cmd->rect.x - (cmd->rect.w/2);
                cmd->rect.x = contentrect->x + (contentrect->w/2) + xoffset;
            }
            break;
        }
        cmd->rect.y = y;
        y += cmd->rect.h + spacing;
        itr = cmd->childend;
    }
}

//****************************************************************************
static void taa_uilayout_pass0(
    taa_uilayout_cmd* cmds,
    const taa_ui_style* styles,
    const taa_ui_control* controls,
    taa_ui_halign halign,
    taa_ui_valign valign,
    int begin,
    int end)
{
    // recursive bottom-up calculation of fixed sized control dimensions
    int itr = begin;
    while(itr != end)
    {
        taa_uilayout_cmd* cmd = cmds + itr;
        int childbegin = cmd->childbegin;
        int childend = cmd->childend;
        // recursively process all children first
        if(childbegin != childend)
        {
            taa_uilayout_pass0(
                cmds,
                styles,
                controls,
                cmd->halign,
                cmd->valign,
                childbegin,
                childend);
        }
        // after children have been sized, compute width of current control
        switch(cmd->rect.w)
        {
        case taa_UI_WIDTH_AUTO:
            cmd->autow = taa_uilayout_auto_width(cmds,cmd,styles,controls);
            cmd->rect.w = cmd->autow;
            break;
        case taa_UI_WIDTH_DEFAULT:
            if(cmd->control0 >= 0)
            {
                const taa_ui_control* control = controls + cmd->control0;
                const taa_ui_style* style = styles + control->styleid;
                cmd->rect.w = style->defaultw;
            }
            else
            {
                cmd->rect.w = 0;
            }
            cmd->autow = cmd->rect.w;
            break;
        case taa_UI_WIDTH_FILL:
            cmd->autow = taa_uilayout_auto_width(cmds,cmd,styles,controls);
            break;
        default:
            cmd->autow = cmd->rect.w;
            break;
        }
        // compute height of control
        switch(cmd->rect.h)
        {
        case taa_UI_HEIGHT_AUTO:
            cmd->autoh = taa_uilayout_auto_height(cmds,cmd,styles,controls);
            cmd->rect.h = cmd->autoh;
            break;
        case taa_UI_HEIGHT_DEFAULT:
            if(cmd->control0 >= 0)
            {
                const taa_ui_control* control = controls + cmd->control0;
                const taa_ui_style* style = styles + control->styleid;
                cmd->rect.h = style->defaulth;
            }
            else
            {
                cmd->rect.h = 0;
            }
            cmd->autoh = cmd->rect.h;
            break;
        case taa_UI_HEIGHT_FILL:
            cmd->autoh = taa_uilayout_auto_height(cmds,cmd,styles,controls);
            break;
        default:
            cmd->autow = cmd->rect.w;
            break;
        }
        itr = childend;
        assert((itr == end) || (cmds[itr].parentcmd == cmd->parentcmd));
    }
}

//****************************************************************************
static void taa_uilayout_pass1(
    taa_uilayout_cmd* cmds,
    const taa_ui_style* styles,
    taa_ui_control* controls,
    taa_uilayout_mode mode,
    taa_ui_halign halign,
    taa_ui_valign valign,
    int spacing,
    int scrollx,
    int scrolly,
    const taa_ui_rect* parentrect,
    const taa_ui_rect* parentclip,
    taa_ui_rect* parentarea,
    int begin,
    int end)
{
    // recursive top-down calculation of filled control dimensions, screen
    // space positions, and clip rectangles
    int itr;
    // calculate size and layout of all the controls at this level first
    switch(mode)
    {
    case taa_UILAYOUT_ABS:
        taa_uilayout_abs(cmds, halign, valign, parentrect, begin, end);
        break;
    case taa_UILAYOUT_COLS:
        taa_uilayout_cols(cmds, valign, spacing, parentrect, begin, end);
        break;
    case taa_UILAYOUT_ROWS:
        taa_uilayout_rows(cmds, halign, spacing, parentrect, begin, end);
        break;
    }
    // export layout to controls and recurse into children
    itr = begin;
    while(itr != end)
    {
        taa_uilayout_cmd* cmd = cmds + itr;
        int childbegin = cmd->childbegin;
        int childend = cmd->childend;
        taa_ui_rect cliprect;  // rect this command is clipped against
        taa_ui_rect contentrect; // used for child positioning and clipping
        taa_ui_rect contentclip; // rect to clip child commands against
        taa_ui_rect childarea; // measures area occupied by children
        // adjust for scroll
        if(cmd->control0 >= 0)
        {
            taa_ui_control* control = controls + cmd->control0;
            const taa_ui_style* style = styles + control->styleid;
            contentrect = cmd->rect;
            contentrect.x += style->lborder + style->lpadding;
            contentrect.y += style->rborder + style->rpadding;
            contentrect.w -= style->lborder + style->lpadding;
            contentrect.w -= style->rborder + style->rpadding;
            contentrect.h -= style->tborder + style->tpadding;
            contentrect.h -= style->bborder + style->bpadding;
            if(contentrect.w < 0)
            {
                contentrect.w = 0;
            }
            if(contentrect.h < 0)
            {
                contentrect.h = 0;
            }
            taa_uilayout_intersect(&cmd->rect, parentclip, &cliprect);
            // export layout to control
            control->rect = cmd->rect;
            control->cliprect = cliprect;
            if(control->type == taa_UI_VSCROLLBAR && cmd->targetcmd >= 0)
            {
                // if it's a vertical scrollbar, the slider rect and range
                // need to be calculated
                taa_uilayout_cmd* targetcmd = cmds + cmd->targetcmd;
                const taa_ui_style* ss;
                taa_ui_rect sliderrect;
                int range; // maximum scroll value
                int value; // current scroll value
                int slidermax; // maximum height of slider
                int slidermin; // minimum height of slider
                int sliderh; // height of slider
                int sliderrange; // amount that slider can move
                ss = styles + control->data.scroll.sliderstyleid;
                range = targetcmd->childrenh - targetcmd->contentrect.h;
                slidermax = contentrect.h;
                slidermin = ss->tborder+ss->tpadding+ss->bpadding+ss->bborder;
                sliderh = (range>0) ? (slidermax*slidermax)/range : slidermax;
                sliderh = (sliderh >= slidermin) ? sliderh : slidermin;
                sliderh = (sliderh <= slidermax) ? sliderh : slidermax;
                sliderrange = slidermax - sliderh;
                sliderrect = contentrect;
                sliderrect.h = sliderh;
                value = control->data.scroll.value;
                if(sliderh > 0 && sliderrange > 0 && range > slidermax)
                {
                    // if scrollable
                    sliderrect.y += (value*sliderrange)/range;
                }
                control->data.scroll.range = range;
                control->data.scroll.sliderrect = sliderrect;
                control->data.scroll.sliderpane = contentrect;
            }
            if(cmd->control1 >= 0)
            {
                // if two controls are associated with this layout command,
                // export the rectangles to the second control as well
                control = controls + cmd->control1;
                control->rect = cmd->rect;
                control->cliprect = cliprect;
            }
        }
        else
        {
            contentrect = cmd->rect;
            taa_uilayout_intersect(&cmd->rect, parentclip, &cliprect);
        }
        // export content size to cmd
        cmd->contentrect = contentrect;
        // after current control is calculated, descend into children
        childarea.x = INT_MAX;
        childarea.y = INT_MAX;
        childarea.w = INT_MIN;
        childarea.h = INT_MIN;
        if(childbegin != childend)
        {
            taa_uilayout_intersect(&contentrect, &cliprect, &contentclip);
            contentrect.x -= scrollx;
            contentrect.y -= scrolly;
            taa_uilayout_pass1(
                cmds,
                styles,
                controls,
                cmd->mode,
                cmd->halign,
                cmd->valign,
                cmd->spacing,
                cmd->scrollx,
                cmd->scrolly,
                &contentrect,
                &contentclip,
                &childarea,
                childbegin,
                childend);
        }
        cmd->childrenw = (childarea.w >= 0) ? childarea.w : 0;
        cmd->childrenh = (childarea.h >= 0) ? childarea.h : 0;
        if(cmd->control0 >= 0)
        {
            // if a control command, propogate area to parent
            taa_uilayout_union(parentarea, &cmd->rect, parentarea);
        }
        else
        {
            // if a pure layout command, propogate child area to parent
            taa_uilayout_union(parentarea, &childarea, parentarea);            
        }
        itr = childend;
        assert((itr == end) || (cmds[itr].parentcmd == cmd->parentcmd));
    }
}

//****************************************************************************
static int taa_uilayout_push(
    taa_uilayout_list* list,
    taa_uilayout_mode mode,
    taa_ui_halign halign,
    taa_ui_valign valign,
    int spacing,
    int parent,
    int targetcmd,
    int control,
    int scrollx,
    int scrolly,
    const taa_ui_rect* rect)
{
    int index = -1;
    if(list->size < list->capacity)
    {
        taa_uilayout_cmd* cmd;
        index = (int) list->size;
        cmd = list->cmds + index;
        cmd->mode = mode;
        cmd->halign = halign;
        cmd->valign = valign;
        cmd->spacing = spacing;
        cmd->parentcmd = parent;
        cmd->targetcmd = targetcmd;
        cmd->childbegin = index + 1;
        cmd->childend = index + 1;
        cmd->control0 = control;
        cmd->control1 = -1;
        cmd->scrollx = scrollx;
        cmd->scrolly = scrolly;
        cmd->rect = *rect;
        ++list->size;
    }
    return index;
}

//****************************************************************************
static void taa_uilayout_pop(
    taa_uilayout_list* list,
    int cmdindex,
    int control)
{
    taa_uilayout_cmd* cmd = list->cmds + cmdindex;
    assert(((size_t) cmdindex) < list->size);
    cmd->control1 = control;
    cmd->childend = list->size;
}

//****************************************************************************
static void taa_uilayout_begin(
    taa_uilayout_list* list)
{
    list->size = 0;
}

//****************************************************************************
static void taa_uilayout_end(
    taa_uilayout_list* list,
    const taa_ui_style* styles,
    taa_ui_control* controls,
    const taa_ui_rect* screenrect)
{
    taa_ui_rect uiarea;
    // fixed and auto layout sizes must be calculated before fill sizes. fill
    // sizes in column and row modes are affected by the amount of available
    // space after fixed space has been allocated. fixed and auto sized space
    // must be calculated from the bottom up, as auto sized containers will
    // depend on the size of their children
    taa_uilayout_pass0(
        list->cmds,
        styles,
        controls,
        taa_UI_HALIGN_LEFT,
        taa_UI_VALIGN_TOP,
        0,
        list->size);
    // fill sizes are calculated after all the fixed and auto sized space has
    // been calculated. fill space is calculated from the top down, as child
    // controls must fit the available space within their containers
    uiarea.x = INT_MAX;
    uiarea.y = INT_MAX;
    uiarea.w = INT_MIN;
    uiarea.h = INT_MIN;
    taa_uilayout_pass1(
        list->cmds,
        styles,
        controls,
        taa_UILAYOUT_ABS,
        taa_UI_HALIGN_LEFT,
        taa_UI_VALIGN_TOP,
        0,
        0,
        0,
        screenrect,
        screenrect,
        &uiarea,
        0,
        list->size);
}

#endif // taa_UILAYOUT_H_

