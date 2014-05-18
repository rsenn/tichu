/* $Id: toggle.c,v 1.17 2005/05/18 22:45:56 smoli Exp $
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

/** @weakgroup sgToggle 
 *  @{
 */

#include <libsgui/sgui.h>

/* Defines the widget type and its callbacks
 * ------------------------------------------------------------------------- */
sgWidgetType sgToggleType =
{
  .name = "sgToggle",
  .size = sizeof(struct sgToggle),
  .methods =
  {
    .recalc = sgRecalcToggle,
    .redraw = sgRedrawToggle,
    .handler = sgHandleToggleEvent,
    .blit = sgBlitWidget
  }
};

/* Creates a new toggle widget by splitting another
 * -------------------------------------------------------------------------- */
sgWidget *sgNewToggleSplitted(sgWidget *based, sgEdge edge, int pixels, 
                              const char *caption)
{
  sgWidget *toggle;
  SDL_Rect  newrect;
  
  sgSplitWidget(based, &newrect, edge, pixels);
  toggle = sgNewToggle(based->parent, newrect.x, newrect.y, newrect.w, newrect.h, caption);
  
  return toggle;
}

/* Creates a new toggle widget and adds it to a group
 * -------------------------------------------------------------------------- */
sgWidget *sgNewToggleGrouped(sgWidget *group, sgEdge edge, sgAlign align,
                             Uint16 w, Uint16 h, const char *caption) 
{
  sgWidget *toggle = sgNewToggle(group, 0, 0, w, h, caption);
  
  sgSubGroup(group, toggle, edge, align);
  
  return toggle;
}

/* Creates a new toggle widget from a rectangle
 * -------------------------------------------------------------------------- */
sgWidget *sgNewToggleRect(sgWidget *parent, SDL_Rect rect, const char *caption) 
{
  return sgNewWidgetRect(&sgToggleType, parent, rect, caption);
}

/* Creates a new toggle widget
 * -------------------------------------------------------------------------- */
sgWidget *sgNewToggle(sgWidget *parent, Sint16 x, Sint16 y, Uint16 w, Uint16 h,
                      const char *caption) 
{
  return sgNewWidget(&sgToggleType, parent, x, y, w, h, caption);
}

/* Recalculates internal toggle rectangles according to toggle->size
 * -------------------------------------------------------------------------- */
void sgRecalcToggle(sgWidget *toggle)
{
  sgFont *font;
  Uint16 minh, minw;
  
  /* Get the font we're drawing on the toggle face */
  font = toggle->font[SG_FONT_NORMAL];
  
  /* Calculate minimal dimensions based on font */
  minw = sgTextWidth(font, toggle->caption) + (toggle->border << 1) + 2;
  minh = sgFontHeight(font) + 3 + 2;
  
  /* Apply them */
  if(toggle->rect.h < minh) toggle->rect.h = minh;
  if(toggle->rect.w < minw) toggle->rect.w = minw;
  
  /* Calc rectangle for the toggle face */
  sgToggle(toggle)->caption.x = 0;
  sgToggle(toggle)->caption.y = 0;
  sgToggle(toggle)->caption.w = toggle->rect.w;
  sgToggle(toggle)->caption.h = toggle->rect.h;

  sgSplitRect(&sgToggle(toggle)->caption, &sgToggle(toggle)->rect, SG_EDGE_LEFT, toggle->rect.h);
  
  sgPadRect(&sgToggle(toggle)->caption, SG_EDGE_LEFT|SG_EDGE_RIGHT, 4);
  
  sgSubBorder(&sgToggle(toggle)->rect, toggle->border);
  
  sgSetWidgetStatus(toggle, SG_REDRAW_NEEDED);
}

/* Redraws toggle widget
 * -------------------------------------------------------------------------- */
void sgRedrawToggle(sgWidget *toggle) 
{
  /* Draw widget border */
  if(sgRedrawWidgetBorder(toggle))
  {
    if(toggle->status & SG_FOCUS)
      sgDrawWidgetBorder(toggle, &sgToggle(toggle)->rect);
    else
      sgClearWidgetBorder(toggle);
  }
    
  /* Draw filled frame for the toggle face */
  if(sgRedrawWidgetFrame(toggle))
  {
    sgDrawFrame(toggle->face.frame, &sgToggle(toggle)->rect,
                sgToggle(toggle)->draw|SG_DRAW_FILL);
  }
  
  /* Draw text onto toggle face */
  if(sgRedrawWidgetContent(toggle))
  {
    sgDrawTextOutline(toggle->font[SG_FONT_NORMAL],
                      toggle->face.content, &sgToggle(toggle)->caption,
                      SG_ALIGN_LEFT|SG_ALIGN_MIDDLE, toggle->caption);
  }
}

/* Handles an incoming event for the toggle widget
 * -------------------------------------------------------------------------- */
int sgHandleToggleEvent(sgWidget *toggle, SDL_Event *event) 
{
  /* Toggle must have focus to generate events */
  if(toggle->status & SG_FOCUS)
  {    
    /* If either the mouse toggle or the return key was released
       we mark the toggle as released and trigger the CLICK event */
    if((sgToggle(toggle)->draw & SG_DRAW_INVFILL) && 
       (sgEventButton(event, SDL_BUTTON_LEFT, SG_RELEASED) ||
        sgEventKey(event, SDLK_RETURN, SG_RELEASED)))
    {
      /* When the event wasn't keyboard generated check also wether the
         mouse is inside of toggle coordinates */
      if(event->type == SDL_KEYUP ||
         sgMatchRect(&sgToggle(toggle)->rect, event->button.x, event->button.y))
      {
        sgReportWidgetEvent(toggle, SG_EVENT_CLICK);
        
        sgToggle(toggle)->draw ^= SG_DRAW_INVERSE;
      }

      sgReportWidgetEvent(toggle, SG_BUTTON_UP);
        sgSetWidgetStatus(toggle, SG_REDRAW_FRAME);
      
      sgToggle(toggle)->draw &= ~SG_DRAW_INVFILL;
      
      return 1;
    }
    
    /* If either the mouse toggle or the return key was 
       pushed then we mark the toggle widget as pushed */
    if((sgToggle(toggle)->draw & SG_DRAW_INVFILL) == 0 &&
       (sgEventButton(event, SDL_BUTTON_LEFT, SG_PRESSED) ||
        sgEventKey(event, SDLK_RETURN, SG_PRESSED)))
    {
      /* When the event wasn't keyboard generated check also wether the
         mouse is inside of toggle coordinates */
      if(event->type == SDL_KEYDOWN ||
         sgMatchRect(&sgToggle(toggle)->rect, event->button.x, event->button.y))
      {
        sgReportWidgetEvent(toggle, SG_BUTTON_DOWN);

        sgToggle(toggle)->draw |= SG_DRAW_INVFILL;
        sgSetWidgetStatus(toggle, SG_REDRAW_FRAME);
        
        return 1;
      }
    } 
  }

  if(event->type == SDL_MOUSEMOTION)
  {
    sgHandleWidgetHilite(toggle, sgToggle(toggle)->rect, 
                         &sgToggle(toggle)->draw, 
                         event->motion.x, event->motion.y);
  }
  
  return 0;
}

/* Sets the state of a toggle
 * -------------------------------------------------------------------------- */
int sgSetToggleState(sgWidget *toggle, int enabled)
{
  /* If the toggle state changes we'll set the redraw flag */
  if(enabled != sgGetToggleState(toggle))
  {
    if(sgToggle(toggle)->draw & SG_DRAW_INVERSE)
      sgToggle(toggle)->draw &= ~SG_DRAW_INVERSE;
    else
      sgToggle(toggle)->draw |= SG_DRAW_INVERSE;
    
    sgSetWidgetStatus(toggle, SG_REDRAW_FRAME);
    return 1;
  }
  
  return 0;
}

/** @} */
