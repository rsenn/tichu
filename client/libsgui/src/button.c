/* $Id: button.c,v 1.23 2005/05/23 02:45:23 smoli Exp $
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

/** @weakgroup sgButton 
 *  @{
 */

#include <libsgui/sgui.h>
#include <libsgui/picts.h>
#include <libsgui/png.h>

/* Defines the widget type and its callbacks
 * ------------------------------------------------------------------------- */
static void sgDeleteButton(sgWidget *button);

sgWidgetType sgButtonType =
{
  .name = "sgButton",
  .size = sizeof(struct sgButton),
  .methods =
  {
    .recalc = sgRecalcButton,
    .redraw = sgRedrawButton,
    .handler = sgHandleButtonEvent,
    .blit = sgBlitWidget,
    .delete = sgDeleteButton
  }
};

/* Cleans up button 
 * -------------------------------------------------------------------------- */
static void sgDeleteButton(sgWidget *button)
{
  if(sgButton(button)->image)
    SDL_FreeSurface(sgButton(button)->image);
}

/* Creates a new button widget by splitting another
 * -------------------------------------------------------------------------- */
sgWidget *sgNewButtonSplitted(sgWidget *based, sgEdge edge, int pixels, 
                              const char *caption)
{
  sgWidget *button;
  SDL_Rect  newrect;
  
  sgSplitWidget(based, &newrect, edge, pixels);
  button = sgNewButton(based->parent, newrect.x, newrect.y, newrect.w, newrect.h, caption);
  
  return button;
}

/* Creates a new button widget and adds it to a group
 * -------------------------------------------------------------------------- */
sgWidget *sgNewButtonGrouped(sgWidget *group, sgEdge edge, sgAlign align,
                             Uint16 w, Uint16 h, const char *caption) 
{
  sgWidget *button = sgNewButton(group, 0, 0, w, h, caption);
  
  sgSubGroup(group, button, edge, align);
  
  return button;
}

/* Creates a new button widget from a rectangle
 * -------------------------------------------------------------------------- */
sgWidget *sgNewButtonRect(sgWidget *parent, SDL_Rect rect, const char *caption) 
{
  return sgNewWidgetRect(&sgButtonType, parent, rect, caption);
}

/* Creates a new button widget
 * -------------------------------------------------------------------------- */
sgWidget *sgNewButton(sgWidget *parent, Sint16 x, Sint16 y, Uint16 w, Uint16 h,
                      const char *caption) 
{
  return sgNewWidget(&sgButtonType, parent, x, y, w, h, caption);
}

/* Recalculates internal button rectangles according to button->size
 * -------------------------------------------------------------------------- */
void sgRecalcButton(sgWidget *button)
{
  sgFont *font;
  Uint16 minh, minw;
  
  /* Get the font we're drawing on the button face */
  font = button->font[SG_FONT_BOLD];
  
  /* Calculate minimal dimensions based on font */
  if(button->caption[0])
  {
    minw = sgTextWidth(font, button->caption) + (button->border << 1) + 2;
    minh = sgFontHeight(font) + 3 + (button->border << 1) + 2;
    
    /* Apply them */
    if(button->rect.h < minh) button->rect.h = minh;
    if(button->rect.w < minw) button->rect.w = minw;
  }
  
  /* Calc rectangle for the button face */
  sgButton(button)->brect.x = 0;
  sgButton(button)->brect.y = 0;
  sgButton(button)->brect.w = button->rect.w;
  sgButton(button)->brect.h = button->rect.h;
  
  sgSubBorder(&sgButton(button)->brect, button->border);
  
  sgButton(button)->caption = sgButton(button)->brect;
  sgSubBorder(&sgButton(button)->caption, 2);

  /* pictogram calculation */
  if(sgButton(button)->pict)
  {
    sgButton(button)->prect.w = sgButton(button)->pict->w;
    sgButton(button)->prect.h = sgButton(button)->pict->h;
    
    sgAlignRect(&sgButton(button)->brect, &sgButton(button)->prect, 
                SG_ALIGN_CENTER|SG_ALIGN_MIDDLE);
  }

  /* image calculation */
  if(sgButton(button)->image)
  {
    SDL_Rect rect;

    sgButton(button)->irect.w = sgButton(button)->image->w;
    sgButton(button)->irect.h = sgButton(button)->image->h;

    sgSplitRect(&sgButton(button)->caption, &rect, sgButton(button)->iedge,
                (sgButton(button)->iedge & (SG_EDGE_LEFT|SG_EDGE_RIGHT)) ?
                sgButton(button)->caption.h : sgButton(button)->caption.w);
    
    sgAlignRect(&rect, &sgButton(button)->irect, sgButton(button)->ialign);
  }
  
  sgSetWidgetStatus(button, SG_REDRAW_NEEDED);
}

/* Redraws button widget
 * -------------------------------------------------------------------------- */
void sgRedrawButton(sgWidget *button) 
{
  /* Draw widget border */
  if(sgRedrawWidgetBorder(button))
  {
    if(sgHasWidgetFocus(button))
      sgDrawWidgetBorder(button, &sgButton(button)->brect);
  }
    
  /* Draw filled frame for the button face */
  if(sgRedrawWidgetFrame(button))
  {
    sgDrawFrame(button->face.frame, &sgButton(button)->brect,
                sgButton(button)->draw|SG_DRAW_FILL);
    
    if(sgButton(button)->pict)
      sgPutPict(button->face.frame, &sgButton(button)->prect, 
                sgButton(button)->pict, sgButton(button)->draw);
  }
  
  /* Draw text onto button face */
  if(sgRedrawWidgetContent(button))
  {
    sgDrawTextOutline(button->font[SG_FONT_BOLD],
                      button->face.content, &sgButton(button)->caption,
                      SG_ALIGN_CENTER|SG_ALIGN_MIDDLE, button->caption);
    
    if(sgButton(button)->image)
      sgCopy(sgButton(button)->image, NULL, 
             button->face.content, &sgButton(button)->irect);
  }
}

/* Handles an incoming event for the button widget
 * -------------------------------------------------------------------------- */
int sgHandleButtonEvent(sgWidget *button, SDL_Event *event) 
{
  /* Button must have focus to generate events */
  if(sgHasWidgetFocus(button))
  {    
    /* If either the mouse button or the return key was released
       we mark the button as released and trigger the CLICK event */
    if(sgGetButtonState(button) && (sgEventButton(event, SDL_BUTTON_LEFT, SG_RELEASED) ||
                                    sgEventKey(event, SDLK_RETURN, SG_RELEASED)))
    {
      /* When the event wasn't keyboard generated check also wether the
         mouse is inside of button coordinates */
      if(event->type == SDL_KEYUP ||
         sgMatchRect(&sgButton(button)->brect, event->button.x, event->button.y))
      {
//        sgLog("button click event");
        sgReportWidgetEvent(button, SG_EVENT_CLICK);
      }

      sgReportWidgetEvent(button, SG_BUTTON_UP);
      
      return sgSetButtonState(button, SG_RELEASED);
    }
    
    /* If either the mouse button or the return key was 
       draw then we mark the button widget as draw */
    if(!sgGetButtonState(button) && (sgEventButton(event, SDL_BUTTON_LEFT, SG_PRESSED) ||
                                     sgEventKey(event, SDLK_RETURN, SG_PRESSED)))
    {
      /* When the event wasn't keyboard generated check also wether the
         mouse is inside of button coordinates */
      if(event->type == SDL_KEYDOWN ||
         sgMatchRect(&sgButton(button)->brect, event->button.x, event->button.y))
      {
        sgReportWidgetEvent(button, SG_BUTTON_DOWN);

        return sgSetButtonState(button, SG_PRESSED);
      }
    }
    
  }

  /* Check for mouse motion */
  if(event->type == SDL_MOUSEMOTION)
  {
    /* When motion was inside button rectangle then we hilite it */
    if(sgMatchRect(&sgButton(button)->brect, event->motion.x, event->motion.y))
    {
      if((sgButton(button)->draw & SG_DRAW_HIGH) == 0)
      {
        sgButton(button)->draw |= SG_DRAW_HIGH;
        sgSetWidgetStatus(button, SG_REDRAW_FRAME);
      }
    }
    else
    {
      if((sgButton(button)->draw & SG_DRAW_HIGH))
      {
        sgButton(button)->draw &= ~SG_DRAW_HIGH;
        sgSetWidgetStatus(button, SG_REDRAW_FRAME);
      }
    }
  }
  
  return 0;
}

/* Sets the state of a button
 * -------------------------------------------------------------------------- */
int sgSetButtonState(sgWidget *button, int draw)
{
  /* If the button state changes we'll set the redraw flag */
  if((draw & SG_DRAW_INVERSE) != sgGetButtonState(button))
  {
    if(sgButton(button)->draw & SG_DRAW_INVERSE)
      sgButton(button)->draw &= ~SG_DRAW_INVERSE;
    else
      sgButton(button)->draw |= SG_DRAW_INVERSE;
    
    sgSetWidgetStatus(button, SG_REDRAW_FRAME);
    return 1;
  }
  
  return 0;
}

/* Sets the pictogram for a button */
void sgSetButtonPict(sgWidget *button, sgPict *pict)
{
  sgButton(button)->pict = pict;
  
  sgRecalcWidget(button);
}

/* Loads the image for a button */
int sgLoadButtonImage(sgWidget *button, const char *file, sgEdge edge, sgAlign align)
{
  if((sgButton(button)->image = sgLoadPngFile(file)) == NULL)
    return -1;
  
  sgButton(button)->iedge = edge;
  sgButton(button)->ialign = align;
  
  sgRecalcWidget(button);
  
  return 0;
}

/** @} */
