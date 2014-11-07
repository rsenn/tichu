/* $Id: adjust.c,v 1.29 2005/05/21 22:36:09 smoli Exp $
 * ------------------------------------------------------------------------- *
 *                     /                                                     *
 *      ___  ___                                                             *
 *     |___ |   )|   )|        Simple and smooth GUI library :)              *
 *      __/ |__/ |__/ |        Copyright (C) 2003-2005  Roman Senn           *
 *          __/                                                              *
 *                                                                           *
 *  This library is free software; you can redistribute it and/or            *
 *  modify it under the terms of the GNU Library General Public              *
 *  License as published by the Free Software Foundation; either             *
 *  version 2 of the License, or (at your option) any later version.         *
 *                                                                           *
 *  This library is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        *
 *  Library General Public License for more details.                         *
 *                                                                           *
 *  You should have received a copy of the GNU Library General Public        *
 *  License along with this library; if not, write to the Free               *
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA *
 * ------------------------------------------------------------------------- */

/** @weakgroup sgAdjust
 *  @{
 */

#include <libsgui/sgui.h>

/* Defines the widget type and its callbacks
 * ------------------------------------------------------------------------- */
sgWidgetType sgAdjustType =
{
  .name = "sgAdjust", 
  .size = sizeof(sgAdjust),
  .methods =
  {
    .recalc = sgRecalcAdjust,
    .redraw = sgRedrawAdjust,
    .handler = sgHandleAdjustEvent,
    .blit = sgBlitWidget
  }
};

/* Creates a new adjust widget by splitting another
 * ------------------------------------------------------------------------- */
sgWidget *sgNewAdjustSplitted(sgWidget *based, sgEdge edge, Uint16 pixels, 
                              double start, double end, const char *caption)
{
  sgWidget *adjust;
  SDL_Rect  newrect;
  
  sgSplitWidget(based, &newrect, edge, pixels);
  adjust = sgNewAdjust(based->parent, newrect.x, newrect.y, newrect.w, newrect.h, 
                       start, end, caption);

  return adjust;
}

/* Creates a new adjust widget and subtracts it from a group
 * ------------------------------------------------------------------------- */
sgWidget *sgNewAdjustGrouped(sgWidget *group, sgEdge edge, sgAlign align,
                             Uint16 w, Uint16 h, double start, double end, const char *caption)
{
  sgWidget *adjust = sgNewAdjust(group, 0, 0, w, h, start, end, caption);
  
  sgSubGroup(group, adjust, edge, align);
  
  return adjust;
}

/* Creates a new adjust widget and adds it to a group
 * ------------------------------------------------------------------------- */
sgWidget *sgNewAdjustAligned(sgWidget *group, sgEdge edge, sgAlign align,
                             Uint16 w, Uint16 h, double start, double end, 
                             const char *caption)
{
  sgWidget *adjust = sgNewAdjust(group, 0, 0, w, h, start, end, caption);
  
  sgAddGroup(group, adjust, edge, align);
  
  return adjust;
}

/* Creates a new adjustment widget from a rectangle
 * ------------------------------------------------------------------------- */
sgWidget *sgNewAdjustRect(sgWidget *parent, SDL_Rect rect,
                      double start, double end, const char *caption)
{
  return sgNewAdjust(parent, rect.x, rect.y, rect.w, rect.h,
                     start, end, caption);
}

/* Creates a new adjustment widget
 * ------------------------------------------------------------------------- */
sgWidget *sgNewAdjust(sgWidget *parent, Sint16 x, Sint16 y, Uint16 w, Uint16 h,
                      double start, double end, const char *caption)
{
  sgWidget *adjust;
  
  /* Create widget structure */
  adjust = sgNewWidget(&sgAdjustType, parent, x, y, w, h, caption);
  
  sgAdjust(adjust)->delta = start;
  sgAdjust(adjust)->max = end - sgAdjust(adjust)->delta;
//  sgAdjust(adjust)->len = rw;
  
  sgRecalcWidget(adjust);
  
//sgLog("rw: %u", rw);
  return adjust;
}

/* Recalculates internal widget rectangles according to widget->size
 * ------------------------------------------------------------------------- */
void sgRecalcAdjust(sgWidget *adjust)
{
  int knobsize;
  SDL_Rect rect;
  sgFont   *font;
  Uint16    minh, minw;
  Uint16    textsize;
  Uint16    numsize;
  char      maxnum[11];
  int up = 0;
  
  /* Get the font we're using for the value and caption */
  font = adjust->font[SG_FONT_NORMAL];
  
  /* lame test */
  if(adjust->rect.w > adjust->rect.h)
  { 
    /* Print the maximal value to temp buffer and measure the width 
      to get minimal dimensions */
    sprintf(maxnum, (sgAdjust(adjust)->fmt ? sgAdjust(adjust)->fmt : "%.0lf"),
            sgAdjust(adjust)->max/* - 1*/);
    numsize = ((sgTextWidth(font, maxnum) + (adjust->border)) + adjust->border - 1) / (adjust->border << 1) * (adjust->border << 1);
    textsize = sgTextWidth(font, adjust->caption) + (adjust->border << 1);
    
  }
  else
  {
    up = 1;
    
    textsize = 
    numsize = sgFontHeight(font) + (adjust->border << 1);
  }
  
  textsize = (textsize + 15) & -16;
  
  /* Calc minimal dimensions */
  if(up == 0)
  { 
    minh = sgFontHeight(font) + 2;
    minw = adjust->border * 4 + numsize + minh + sgTextWidth(font, adjust->caption);    
  }
  else
  {
    minw = (adjust->caption) ? sgTextWidth(font, adjust->caption) : sgFontHeight(font) + 2;
    minh = adjust->border * 4 + sgFontHeight(font) + 2 + minw + sgFontHeight(font) + 2;
  }

  /* Apply minimal dimensions */
  if(adjust->rect.h < minh) adjust->rect.h = minh;
  if(adjust->rect.w < minw) adjust->rect.w = minw;
  
  rect.x = 0;
  rect.y = 0;
  rect.w = adjust->rect.w;
  rect.h = adjust->rect.h;

  sgSubBorder(&rect, adjust->border);
  
  /* width & height of the knob */
  knobsize = (up) ? rect.w : rect.h;
  
/*  if(up)
    titlewidth = rect.wh - (sgAdjust(adjust)->len + knobsize + numsize);
  else
    titlewidth = rect.w - (sgAdjust(adjust)->len + knobsize + numsize);*/
    
  /* Rectangle for the caption */
  if(adjust->caption[0])
  {
    sgSplitRect(&rect, &sgAdjust(adjust)->caption,
                (up) ? SG_EDGE_TOP : SG_EDGE_LEFT, textsize);
    
    sgPadRect(&sgAdjust(adjust)->caption, 
              (up) ? SG_EDGE_BOTTOM|SG_EDGE_TOP : SG_EDGE_LEFT|SG_EDGE_RIGHT,
              adjust->border);
  }
  
  
  sgSplitRect(&rect, &sgAdjust(adjust)->value,
              (up) ? SG_EDGE_BOTTOM : SG_EDGE_RIGHT, numsize);
    
  sgPadRect(&sgAdjust(adjust)->value,
            (up) ? SG_EDGE_BOTTOM|SG_EDGE_TOP : SG_EDGE_LEFT|SG_EDGE_RIGHT,
            adjust->border);
  
  /* hidden range rect */
    
  sgAdjust(adjust)->range = rect;

  sgAdjust(adjust)->len = rect.w - knobsize;
  
  /* Rectangles for the ruler and the knob */  
  if(up)
  {    
    sgAdjust(adjust)->ruler = rect;
    
/*    rect.x += knobsize / 4;
    rect.w = knobsize;*/

    sgAdjust(adjust)->knob = sgAdjust(adjust)->ruler;
    sgAdjust(adjust)->knob.h = knobsize;
  } 
  else 
  {
    sgAdjust(adjust)->ruler = rect;
    
    sgAdjust(adjust)->knob = sgAdjust(adjust)->ruler;
    sgAdjust(adjust)->knob.w = knobsize;
  }
  
  sgPadRect(&sgAdjust(adjust)->ruler, SG_EDGE_ALL, knobsize / 4);
  
  sgSetWidgetStatus(adjust, SG_REDRAW_NEEDED);
}

/* Redraws adjust widget
 * ------------------------------------------------------------------------- */
void sgRedrawAdjust(sgWidget *adjust) 
{
  SDL_Rect numrect;
  
  int up = 0;
  if(adjust->rect.h > adjust->rect.w) up = 1;
  
  if(sgRedrawWidgetBorder(adjust))
  {
    if(sgHasWidgetFocus(adjust))
    {
      sgDrawWidgetBorder(adjust, &sgAdjust(adjust)->ruler);
      SDL_FillRect(adjust->face.border, &sgAdjust(adjust)->knob, 0);
    }
  }
  
  /* Draw adjustment frames */
  if(sgRedrawWidgetFrame(adjust))
  {
/*    sgDrawFrame(adjust->face.frame, &sgAdjust(adjust)->range,
                SG_DRAW_INVERSE|SG_DRAW_CONTRAST|SG_DRAW_FILL);*/
    
    sgDrawFrame(adjust->face.frame, &sgAdjust(adjust)->ruler,
                sgAdjust(adjust)->rdraw|SG_DRAW_FILL|SG_DRAW_INVERSE);
    
    sgDrawFrame(adjust->face.frame, &sgAdjust(adjust)->knob,
                sgAdjust(adjust)->pushed|SG_DRAW_FILL);
    
/*    rect = sgAdjust(adjust)->knob;
    sgSubBorder(&rect, 2);
    
    sgDrawGradient(adjust->face.frame, &rect,
                   (sgAdjust(adjust)->pushed ? SG_DRAW_INVERSE|SG_DRAW_SOFT : 0)|SG_DRAW_FILL);*/
  }
  
  if(sgRedrawWidgetContent(adjust))
  {
    char value[64];
    
    /* Draw widget title */
    if(adjust->caption[0])
      sgDrawTextOutline(adjust->font[SG_FONT_NORMAL],
                        adjust->face.content, &sgAdjust(adjust)->caption,
                        (up) ? SG_ALIGN_CENTER|SG_ALIGN_TOP : SG_ALIGN_CENTER|SG_ALIGN_MIDDLE, 
                        adjust->caption);
    
    /* Redraw value */
    numrect = sgAdjust(adjust)->value;
    numrect.y -= 1;
    numrect.h += 2;
    numrect.x -= 1;
    numrect.w += 2;
    SDL_FillRect(adjust->face.content, &numrect, 0);

    snprintf(value, sizeof(value), (sgAdjust(adjust)->fmt ? sgAdjust(adjust)->fmt : "%.0lf"), sgAdjust(adjust)->pos + sgAdjust(adjust)->delta);
    
    sgDrawTextOutline(adjust->font[SG_FONT_NORMAL], 
                      adjust->face.content, &sgAdjust(adjust)->value,
                      (up) ? SG_ALIGN_CENTER|SG_ALIGN_TOP : SG_ALIGN_CENTER|SG_ALIGN_MIDDLE, 
                      value);
  }
}

/* Handles an incoming event for the adjustment widget
 * -------------------------------------------------------------------------- */
int sgHandleAdjustEvent(sgWidget *adjust, SDL_Event *event) 
{
  int up = 0;
  if(adjust->rect.h > adjust->rect.w) up = 1;
  
  if(event->type == SDL_MOUSEMOTION)
  {
    if(!sgAdjust(adjust)->cursor && sgMatchRect(&sgAdjust(adjust)->knob, event->motion.x, event->motion.y))
    {
      sgSetDialogCursor(adjust->dialog, SG_CURSOR_RESIZE_H);
      sgSetWidgetStatus(adjust->dialog, SG_REDRAW_MOUSE);
      sgAdjust(adjust)->cursor = 1;      
      return 1;
    }
    
    if(sgAdjust(adjust)->cursor && !sgAdjust(adjust)->pushed &&
       !sgMatchRect(&sgAdjust(adjust)->knob, event->motion.x, event->motion.y))
    {
      sgSetDialogCursor(adjust->dialog, SG_CURSOR_DEFAULT);
      sgSetWidgetStatus(adjust->dialog, SG_REDRAW_MOUSE);
      sgAdjust(adjust)->cursor = 0;
      return 1;
    }
  }
  
  /* Adjust must have focus to generate events */
  if(sgHasWidgetFocus(adjust))
  {
    /* Button was pushed and we aren't adjusting */
    if(sgEventButton(event, SDL_BUTTON_LEFT, SG_PRESSED) && !(sgAdjust(adjust)->pushed & SG_DRAW_INVFILL))
    {
      /* When the click hits the knob then begin scroll-adjusting */
      if(sgMatchRect(&sgAdjust(adjust)->knob, event->button.x, event->button.y))
      {
        sgAdjust(adjust)->pushed |= SG_DRAW_INVFILL;
        
        sgAdjust(adjust)->offset = 
          (up ? (event->button.y - sgAdjust(adjust)->knob.y) : 
                (event->button.x - sgAdjust(adjust)->knob.x));
        
        sgSetWidgetStatus(adjust, SG_REDRAW_FRAME);
        return 1;
      }
      
      /* ignore clicks outside range */
      if(up && event->button.y > adjust->rect.h)
        return 0;
      
      if(!up && event->button.x > adjust->rect.w)
        return 0;
      
      /* When the click hits the range, then set the position */
      if(sgMatchRect(&sgAdjust(adjust)->ruler, event->button.x, event->button.y))
      {
        int pos;
        double value;

        if(up)
        { 
          pos = event->button.y - sgAdjust(adjust)->ruler.y - sgAdjust(adjust)->ruler.h;
          value = sgAdjust(adjust)->max - (double)pos * sgAdjust(adjust)->max / (double)sgAdjust(adjust)->ruler.h;
         }
        else
        {
          pos = event->button.x - sgAdjust(adjust)->ruler.x;
          value = (double)pos * sgAdjust(adjust)->max / (double)sgAdjust(adjust)->ruler.w;
        }

        sgAdjust(adjust)->rdraw &= ~SG_DRAW_HIGH;
        sgAdjust(adjust)->pushed |= SG_DRAW_HIGH;
        
        return sgSetAdjustValue(adjust, value + sgAdjust(adjust)->delta);
      }
    }
    
    /* Button was released and we are adjusting */
    if(sgEventButton(event, SDL_BUTTON_LEFT, SG_RELEASED) && (sgAdjust(adjust)->pushed & SG_DRAW_INVFILL))
    {
      if(!sgMatchRect(&sgAdjust(adjust)->knob, event->button.x, event->button.y))
      {
        sgSetDialogCursor(adjust->dialog, SG_CURSOR_DEFAULT);
        sgSetWidgetStatus(adjust->dialog, SG_REDRAW_MOUSE);
      }
      
      sgAdjust(adjust)->pushed &= ~SG_DRAW_INVFILL;
      sgSetWidgetStatus(adjust, SG_REDRAW_FRAME);
      return 1;
    }
    
    /* If we are adjusting and the mouse is moving... */
    if(event->type == SDL_MOUSEMOTION && (sgAdjust(adjust)->pushed & SG_DRAW_INVFILL))
    {
      int x = (Sint16)event->motion.x;
      int y = (Sint16)event->motion.y;
      
      if(sgAdjust(adjust)->pushed)
      {
        int pos;
        double value;
        
        
        
        /* ignore motion outside range */
        if(up)
        {
          if(y > adjust->rect.h)
            return 0;
          
          pos = sgLimit(y - sgAdjust(adjust)->range.y, sgAdjust(adjust)->len);
          value = pos * sgAdjust(adjust)->max / sgAdjust(adjust)->len;
        }
        else
        {
          if(x > adjust->rect.w)
            return 0;

          x -= sgAdjust(adjust)->offset;
          x -= sgAdjust(adjust)->range.x;

          pos = sgLimit(x, sgAdjust(adjust)->len);
          
          value = pos * sgAdjust(adjust)->max / sgAdjust(adjust)->len;
        }
        
        return sgSetAdjustValue(adjust, value + sgAdjust(adjust)->delta);
      } 
    } 
    
    if(sgEventButton(event, SDL_BUTTON_WHEELUP, SG_PRESSED) ||
       sgEventKey(event, SDLK_LEFT, SG_PRESSED))
    {
      if(sgAdjust(adjust)->pos > 0)
        return sgSetAdjustValue(adjust, sgAdjust(adjust)->pos - (sgAdjust(adjust)->max / sgAdjust(adjust)->len) + sgAdjust(adjust)->delta);
    }
    
    if(sgEventButton(event, SDL_BUTTON_WHEELDOWN, SG_PRESSED) ||
       sgEventKey(event, SDLK_RIGHT, SG_PRESSED))
    {
      if(sgAdjust(adjust)->pos < sgAdjust(adjust)->max) 
        return sgSetAdjustValue(adjust, sgAdjust(adjust)->pos + (sgAdjust(adjust)->max / sgAdjust(adjust)->len) + sgAdjust(adjust)->delta);
    }
  }

  if(event->type == SDL_MOUSEMOTION)
  {
    sgHandleWidgetHilite(adjust, sgAdjust(adjust)->ruler, 
                         &sgAdjust(adjust)->rdraw, 
                         event->motion.x, event->motion.y);
    
    sgHandleWidgetHilite(adjust, sgAdjust(adjust)->knob, 
                         &sgAdjust(adjust)->pushed, 
                         event->motion.x, event->motion.y);
    
    if(sgAdjust(adjust)->pushed & SG_DRAW_HIGH)
      sgAdjust(adjust)->rdraw &= ~SG_DRAW_HIGH;
  }
  
  /* Cancel adjusting if we're not focused */
  if((adjust->status & SG_FOCUS) == 0 && sgAdjust(adjust)->pushed)
  {
    sgAdjust(adjust)->pushed = 0;
    sgSetWidgetStatus(adjust, SG_REDRAW_FRAME);
    return 1;
  }

  return 0;
}

/* Sets the format string which is used for the formatting of the value
 * ------------------------------------------------------------------------- */
int sgSetAdjustFormat(sgWidget *adjust, const char *fmt)
{
  sgAdjust(adjust)->fmt = fmt;
  
  sgRecalcWidget(adjust);
  
  return 1;
}
  
/* Sets the position of the adjustment knob
 * ------------------------------------------------------------------------- */
int sgSetAdjustValue(sgWidget *adjust, double value)
{
  int x, y;
  int up = 0;
  if(adjust->rect.h > adjust->rect.w) up = 1;

  SDL_GetMouseState(&x, &y);
  x -= adjust->rect.x;
  y -= adjust->rect.y;
  
    if(!sgAdjust(adjust)->cursor && sgMatchRect(&sgAdjust(adjust)->knob, x, y))
    {
      sgSetDialogCursor(adjust->dialog, SG_CURSOR_RESIZE_H);
      sgSetWidgetStatus(adjust->dialog, SG_REDRAW_MOUSE);
      sgAdjust(adjust)->cursor = 1;      
    }
    
    if(sgAdjust(adjust)->cursor && !sgAdjust(adjust)->pushed &&
       !sgMatchRect(&sgAdjust(adjust)->knob, x, y))
    {
      sgSetDialogCursor(adjust->dialog, SG_CURSOR_DEFAULT);
      sgSetWidgetStatus(adjust->dialog, SG_REDRAW_MOUSE);
      sgAdjust(adjust)->cursor = 0;
    }
    
  
  if(value - sgAdjust(adjust)->delta != sgAdjust(adjust)->pos)
  {
    int pos;
    
    sgAdjust(adjust)->pos = value - sgAdjust(adjust)->delta;

    pos = (value - sgAdjust(adjust)->delta) * sgAdjust(adjust)->len / sgAdjust(adjust)->max;
    
    if(up)
      sgAdjust(adjust)->knob.y =
      sgAdjust(adjust)->ruler.y + pos - 
      sgAdjust(adjust)->knob.h / 4;
    else
      sgAdjust(adjust)->knob.x = 
      sgAdjust(adjust)->ruler.x + pos -
      sgAdjust(adjust)->knob.w / 4;
    
    sgReportWidgetEvent(adjust, SG_EVENT_CHANGE);
    
    sgSetWidgetStatus(adjust, SG_REDRAW_FRAME|SG_REDRAW_CONTENT|SG_REDRAW_BORDER);
    return 1;
  }
  
  return 0;
}


/* Gets the position of the adjustment knob
 * ------------------------------------------------------------------------- */
double sgGetAdjustValue(sgWidget *adjust, double *pos)
{
  if(pos)
    *pos = sgAdjust(adjust)->pos + sgAdjust(adjust)->delta;
  
  return sgAdjust(adjust)->pos + sgAdjust(adjust)->delta;
}

/** @} */
