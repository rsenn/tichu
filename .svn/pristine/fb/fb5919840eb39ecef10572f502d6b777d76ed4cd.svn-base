/* $Id: widget.c,v 1.37 2005/05/22 02:44:34 smoli Exp $
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

/** @weakgroup sgWidget
  * @{
  */
#include <stdlib.h>
#include <string.h>
#include <SDL.h>

#include <libsgui/sgui.h>
#include <libsgui/list.h>
#include <libsgui/common.h>

sgList sgWidgetList;

/* Creates a new widget based on a rectangle
 * -------------------------------------------------------------------------- */
sgWidget *sgNewWidgetRect(sgWidgetType *type, sgWidget *parent, SDL_Rect rect,
                          const char *caption)
{
  return sgNewWidget(type, parent, rect.x, rect.y, rect.w, rect.h, caption);
}

/* Creates a new widget
 * -------------------------------------------------------------------------- */
sgWidget *sgNewWidget(sgWidgetType *type, sgWidget *parent,
                      Sint16 x, Sint16 y, Uint16 w, Uint16 h, 
                      const char *caption)
{
  sgWidget *widget;
  int i;

  /* Allocate and clear widget structure */
  if((widget = malloc(sizeof(sgWidget) + type->size)) == NULL)
    return NULL;

  memset(widget, 0, sizeof(sgWidget) + type->size);
  widget->data.ptr = &widget[1];
  
  /* Link it to the parents widget list or the global */  
  sgAddListHead((parent ? &parent->list : &sgWidgetList), &widget->node, widget);
  
  /* Set some initial values */
  widget->type = type;
  widget->parent = parent;

  if(parent)
  {
    if(parent->type == &sgDialogType)
      widget->dialog = parent;
    else
      widget->dialog = parent->dialog;
  }
  
  /* Set caption */
  sgStringCopy(widget->caption, caption);
  
  /* Initialise callbacks */
  widget->methods = type->methods;

  /* Inherit stuff from the parent */
  if(parent)
  {
    widget->alpha = parent->alpha;
    widget->border = parent->border;
    widget->proc = parent->proc;

    widget->palette.frame = parent->palette.frame;
    widget->palette.border = parent->palette.border;
    widget->palette.content = parent->palette.content;

    for(i = 0; i < sizeof(widget->font) / sizeof(widget->font[0]); i++)
      widget->font[i] = parent->font[i];
  }
  
  /* Initialise surface pointers */
/*  widget->face.frame = NULL;
  widget->face.border = NULL;
  widget->face.content = NULL;*/

  /* Set widget rectangle */
  sgSetWidgetPos(widget, x, y, SG_ALIGN_LEFT|SG_ALIGN_TOP);
  sgSetWidgetSize(widget, w, h);
  
  return widget;
}

/* -------------------------------------------------------------------------- *
 * Free widget ressources (doesn't recurse)                                   *
 * -------------------------------------------------------------------------- */
void sgFreeWidget(sgWidget *widget) 
{
  sgFreeWidgetFace(widget);
  sgFreeWidgetBorder(widget);
  sgFreeWidgetContent(widget);
  
  if(widget->methods.delete)
    widget->methods.delete(widget);
 
  /* Free all palettes whose pointer doesn't match the parent one */
  if(widget->parent == NULL ||
     widget->parent->palette.frame != widget->palette.frame)
      free(widget->palette.frame);

  if(widget->parent == NULL ||
     widget->parent->palette.border != widget->palette.border)
      free(widget->palette.border);
  
  if(widget->parent == NULL || 
     widget->parent->palette.content != widget->palette.content)
      free(widget->palette.content);
  
  sgDeleteList((widget->parent ? &widget->parent->list : &sgWidgetList),
               &widget->node);
}

/* -------------------------------------------------------------------------- *
 * Free widget ressources and the widget struct itself (recurses)             *
 * -------------------------------------------------------------------------- */
void sgDeleteWidget(sgWidget *widget)
{
  sgWidget *child;
  sgWidget *next;
  
  sgFreeWidget(widget);
  
  /* recurse into children */
  sgForeachSafe(&widget->list, child, next)
    sgDeleteWidget(child);
  
  free(widget);
}

/* Sets the widget status
 * -------------------------------------------------------------------------- */
void sgSetWidgetStatus(sgWidget *widget, int flags)
{
  if(flags == SG_REDRAW_SCREEN)
  {
    while(widget->parent)
      widget = widget->parent;
    
    widget->status |= SG_REDRAW_SCREEN;
  }
  else
  {
    /* SG_REDRAW_SCREEN must be set recursively until the root */
    if(flags & SG_REDRAW_NEEDED)
      sgSetWidgetStatus(widget, SG_REDRAW_SCREEN);
    
    if((widget->status & flags) != flags)
      widget->status |= flags;
  }
}

/* Unsets the widget status
 * -------------------------------------------------------------------------- */
void sgClearWidgetStatus(sgWidget *widget, int flags)
{
  if((widget->status & flags))
  {
    widget->status &= ~flags;
  }
}

/* Recalculates widget dimensions
 * -------------------------------------------------------------------------- */
void sgRecalcWidget(sgWidget *widget)
{
  if(widget->methods.recalc)
    widget->methods.recalc(widget);
}

/* Sets the widget procedure
 * -------------------------------------------------------------------------- */
void sgSetWidgetProc(sgWidget *widget, sgWidgetProc *proc)
{
  widget->proc = proc;
}

/* Sets the widget position 
 * -------------------------------------------------------------------------- */
void sgSetWidgetPos(sgWidget *widget, Sint16 x, Sint16 y, sgAlign align)
{
  SDL_Rect rect = widget->rect;
  
  sgAlignPos(&rect, x, y, align);
  
  /* Has the position changed? */
  if(rect.x != widget->rect.x || rect.y != widget->rect.y)
  {
    widget->rect.x = rect.x;
    widget->rect.y = rect.y;

    widget->area = widget->rect;

    sgSetWidgetStatus(widget, SG_REDRAW_SCREEN);
  }
}

/* Sets the widget size
 * -------------------------------------------------------------------------- */
void sgSetWidgetSize(sgWidget *widget, Uint16 w, Uint16 h)
{
  /* Has the size changed? */
  if(w != widget->rect.w || h != widget->rect.h)
  {
    widget->rect.w = w;
    widget->rect.h = h;

    widget->area = widget->rect;

    sgFreeWidgetFace(widget);
    
    sgRecalcWidget(widget);
  }  
}

/* Sets the widget rectangle
 * -------------------------------------------------------------------------- */
void sgSetWidgetRect(sgWidget *widget, SDL_Rect rect)
{
  /* Set the position first */
  sgSetWidgetPos(widget, rect.x, rect.y, SG_ALIGN_LEFT|SG_ALIGN_TOP);
  
  /* Set the dimensions then */
  sgSetWidgetSize(widget, rect.w, rect.h);
}

/* Sets the widget border thickness
 * -------------------------------------------------------------------------- */
void sgSetWidgetBorder(sgWidget *widget, int border)
{
  if(widget->border != border)
  {
    widget->border = border;
    
    sgRecalcWidget(widget);
  }
}
  
/* -------------------------------------------------------------------------- *
 * Split widget                                                               *
 * -------------------------------------------------------------------------- */
void sgSplitWidget(sgWidget *widget, SDL_Rect *newrect, sgEdge edge, int pixels)
{
  SDL_Rect rect;
  
  rect = widget->rect;
  
  sgSplitRect(&rect, newrect, edge, pixels);
  sgSetWidgetRect(widget, rect);
  
  sgRecalcWidget(widget);
}

/* -------------------------------------------------------------------------- *
 * Creates widget border surface (8-bit indexed, color-keyed, source alpha)   *
 * -------------------------------------------------------------------------- */
void sgCreateWidgetBorder(sgWidget *widget) 
{
  /* Free old surface if present */
  if(widget->face.border)
    SDL_FreeSurface(widget->face.border);
  
  /* Create a new surface */
  widget->face.border = 
    SDL_CreateRGBSurface(SDL_SWSURFACE, widget->rect.w, widget->rect.h, 
                         8, 0, 0, 0, 0);
  if(widget->face.border)
  {
    /* Set the colorkey */
    SDL_SetColorKey(widget->face.border, SDL_SRCCOLORKEY|SDL_RLEACCEL, 0);
    
    /* Set source alpha */
    SDL_SetAlpha(widget->face.border, SDL_SRCALPHA|SDL_RLEACCEL, widget->alpha);
    
    SDL_SetPalette(widget->face.border, SDL_LOGPAL, (SDL_Color *)widget->palette.border, 0, 256);
    /* Clear the surface */
//    sgClearWidgetBorder(widget);
  }
  
  /* request setting the palette */
  widget->status |= SG_REDRAW_PALETTE;
}

/* -------------------------------------------------------------------------- *
 * Converts widget border surface to display format                           *
 * -------------------------------------------------------------------------- */
void sgConvertWidgetBorder(sgWidget *widget)
{
  if(widget->face.border)
  {
    SDL_Surface *tmp = SDL_DisplayFormat(widget->face.border);

    if(tmp)
    {
      SDL_FreeSurface(widget->face.border);
      widget->face.border = tmp;
    }
  }
}

/* -------------------------------------------------------------------------- *
 * Creates widget frame surface (8-bit indexed, color-keyed, opaque)          *
 * -------------------------------------------------------------------------- */
void sgCreateWidgetFrame(sgWidget *widget)
{
  /* Free old surface if present */
  if(widget->face.frame)
    SDL_FreeSurface(widget->face.frame);
  
  /* Create a surface */
  widget->face.frame = 
    SDL_CreateRGBSurface(SDL_SWSURFACE, widget->rect.w, widget->rect.h, 8,
                         0, 0, 0, 0);

  if(widget->face.frame)
  {
    /* Set the colorkey */
    SDL_SetColorKey(widget->face.frame, SDL_SRCCOLORKEY|SDL_RLEACCEL, 0);
    
    SDL_SetPalette(widget->face.frame, SDL_LOGPAL,
                   (SDL_Color *)widget->palette.frame, 0, 256);
    
    if(widget->status & SG_DISABLED)
      SDL_SetAlpha(widget->face.frame, SDL_SRCALPHA, widget->alpha);
  }
  
  /* request setting the palette */
  widget->status |= SG_REDRAW_PALETTE;
}

/* -------------------------------------------------------------------------- *
 * Converts widget frame surface to display format                            *
 * -------------------------------------------------------------------------- */
void sgConvertWidgetFrame(sgWidget *widget)
{
  if(widget->face.frame)
  {
    SDL_Surface *tmp = SDL_DisplayFormat(widget->face.frame);

    if(tmp)
    {
      SDL_FreeSurface(widget->face.frame);
      widget->face.frame = tmp;
    }
  }
}

/* -------------------------------------------------------------------------- *
 * Creates widget content surface (32-bit, alpha layered)                     *
 * -------------------------------------------------------------------------- */
void sgCreateWidgetContent(sgWidget *widget)
{
  /* Free old surface if present */
  if(widget->face.content)
    SDL_FreeSurface(widget->face.content);
  
  /* Create a 32-bit surface */
  widget->face.content = SDL_CreateRGBSurface(SDL_SWSURFACE,
                                              widget->rect.w, widget->rect.h, 32,
                                              RMASK, GMASK, BMASK, AMASK);
  if(widget->status & SG_DISABLED)
    SDL_SetAlpha(widget->face.content, SDL_SRCALPHA, widget->alpha);
  
  if(widget->face.content)
    SDL_FillRect(widget->face.content, NULL, 0);
}

/* -------------------------------------------------------------------------- *
 * Converts widget content surface to display format                          *
 * -------------------------------------------------------------------------- */
void sgConvertWidgetContent(sgWidget *widget)
{
  if(widget->face.content)
  {
    SDL_Surface *tmp = SDL_DisplayFormatAlpha(widget->face.content);

    if(tmp)
    {
      SDL_FreeSurface(widget->face.content);
      widget->face.content = tmp;
    }
  }
}

/* -------------------------------------------------------------------------- *
 * Create widget surfaces                                                     *
 * -------------------------------------------------------------------------- */
void sgCreateWidgetFace(sgWidget *widget)
{
  sgCreateWidgetContent(widget);
  sgCreateWidgetFrame(widget);
  sgCreateWidgetBorder(widget);
}

/* -------------------------------------------------------------------------- *
 * Free widget surfaces                                                       *
 * -------------------------------------------------------------------------- */
void sgFreeWidgetFace(sgWidget *widget)
{
  sgFreeWidgetContent(widget);
  sgFreeWidgetFrame(widget);
  sgFreeWidgetBorder(widget);
}
  
/* -------------------------------------------------------------------------- *
 * Clears all widget surfaces                                                 *
 * -------------------------------------------------------------------------- */
void sgClearWidgetFace(sgWidget *widget)
{
  sgClearWidgetContent(widget);
  sgClearWidgetFrame(widget);
  sgClearWidgetBorder(widget);
}

/* -------------------------------------------------------------------------- *
 * Creates border surface if it doesn't exist and draws a gradient to it      *
 * -------------------------------------------------------------------------- */
void sgDrawWidgetBorder(sgWidget *widget, SDL_Rect *cutout)
{
  sgDrawFrame(widget->face.border, &widget->face.border->clip_rect,
              SG_DRAW_FILL);
  
  if(cutout)
    SDL_FillRect(widget->face.border, cutout, 0);
}
  
/* -------------------------------------------------------------------------- *
 * Disable widget                                                             *
 * -------------------------------------------------------------------------- */
void sgDisableWidget(sgWidget *widget) 
{
  if((widget->status & SG_DISABLED) == 0)
  {
/*    sgWidget *child;*/
    
    widget->status |= SG_DISABLED|SG_REDRAW_FRAME|SG_REDRAW_CONTENT;
/*    
    sgForeach(&widget->list, child)
      sgDisableWidget(child);*/
  }
}

/* -------------------------------------------------------------------------- *
 * Hide and disable widget                                                    *
 * -------------------------------------------------------------------------- */
void sgHideWidget(sgWidget *widget) 
{
  if((widget->status & SG_HIDDEN) == 0)
  {
    widget->status |= SG_DISABLED|SG_HIDDEN|SG_REDRAW_SCREEN;
  }
}  

/* -------------------------------------------------------------------------- *
 * Show and re-enable widget                                                  *
 * -------------------------------------------------------------------------- */
void sgShowWidget(sgWidget *widget) 
{
  if(widget->status & SG_HIDDEN) 
  {
    widget->status &= ~(SG_HIDDEN|SG_DISABLED);
    widget->status |= SG_REDRAW_SCREEN|SG_REDRAW_PALETTE;
  }
}

/* -------------------------------------------------------------------------- *
 * Re-enable widget                                                           *
 * -------------------------------------------------------------------------- */
void sgEnableWidget(sgWidget *widget) 
{
  if(widget->status & SG_DISABLED)
  {
    widget->status &= ~SG_DISABLED;
    widget->status |= SG_REDRAW_FRAME|SG_REDRAW_CONTENT;
  }
}

/* -------------------------------------------------------------------------- *
 * Generate widget palette from color                                         *
 * -------------------------------------------------------------------------- */
void sgSetWidgetFrameColor(sgWidget *widget, sgColor color)
{
  if(widget->list.size == 0 && 
     (widget->parent == NULL || widget->palette.frame != widget->parent->palette.frame))
    free(widget->palette.frame);
  
  widget->palette.frame = sgCreatePalette(color);
  sgSetWidgetStatus(widget, SG_REDRAW_PALETTE|SG_REDRAW_FRAME);
}

void sgSetWidgetBorderColor(sgWidget *widget, sgColor color)
{
  if(widget->list.size == 0 && 
     (widget->parent == NULL || widget->palette.border != widget->parent->palette.border))
    free(widget->palette.border);
  
  widget->palette.border = sgCreatePalette(color);
  sgSetWidgetStatus(widget, SG_REDRAW_PALETTE|SG_REDRAW_BORDER);
}

void sgSetWidgetContentColor(sgWidget *widget, sgColor color)
{
  if(widget->list.size == 0 && 
     (widget->parent == NULL || widget->palette.content != widget->parent->palette.content))
    free(widget->palette.content);
  
  widget->palette.content = sgCreatePalette(color);
  sgSetWidgetStatus(widget, SG_REDRAW_PALETTE|SG_REDRAW_CONTENT);
}

void sgSetWidgetColor(sgWidget *widget, sgColor color)
{
  sgSetWidgetFrameColor(widget, color);
  sgSetWidgetBorderColor(widget, color);
  sgSetWidgetContentColor(widget, color);
}

/* -------------------------------------------------------------------------- *
 * Generate widget palette from RGB value                                     *
 * -------------------------------------------------------------------------- */
void sgSetWidgetFrameRGB(sgWidget *widget, Uint8 r, Uint8 g, Uint8 b)
{
  sgColor color; color.r = r; color.g = g; color.b = b;
  sgSetWidgetFrameColor(widget, color);
}
  
void sgSetWidgetBorderRGB(sgWidget *widget, Uint8 r, Uint8 g, Uint8 b)
{
  sgColor color; color.r = r; color.g = g; color.b = b;
  sgSetWidgetBorderColor(widget, color);
}
  
void sgSetWidgetContentRGB(sgWidget *widget, Uint8 r, Uint8 g, Uint8 b)
{
  sgColor color; color.r = r; color.g = g; color.b = b;
  sgSetWidgetContentColor(widget, color);
}

void sgSetWidgetRGB(sgWidget *widget, Uint8 r, Uint8 g, Uint8 b)
{
  sgSetWidgetFrameRGB(widget, r, g, b);
  sgSetWidgetBorderRGB(widget, r, g, b);
  sgSetWidgetContentRGB(widget, r, g, b);  
}

/* -------------------------------------------------------------------------- *
 * Set widget font                                                            *
 * -------------------------------------------------------------------------- */
void sgSetWidgetFont(sgWidget *widget, int type, sgFont *font)
{
  widget->font[type] = font;
  
  sgRecalcWidget(widget);
}

/* -------------------------------------------------------------------------- *
 * Load widget font                                                           *
 * -------------------------------------------------------------------------- */
int sgLoadWidgetFont(sgWidget *widget, int type, const char *file)
{
  sgFont *font;
  
  if((font = sgLoadFontFile(file)))
  {
    sgSetWidgetFont(widget, type, font);
    return 0;
  }
  
  sgLog("Could not load the font file '%s'", file);
  return -1;
}

/* -------------------------------------------------------------------------- *
 * Set all widget fonts                                                       *
 * -------------------------------------------------------------------------- */
void sgSetWidgetFonts(sgWidget *widget, sgFont *normal, sgFont *bold, sgFont *fixed)
{
  if(normal)
    widget->font[SG_FONT_NORMAL] = normal;
  
  if(bold)
    widget->font[SG_FONT_BOLD] = bold;
  
  if(fixed)
    widget->font[SG_FONT_FIXED] = fixed;
  
  sgRecalcWidget(widget);
}

/* -------------------------------------------------------------------------- *
 * Load all widget fonts                                                      *
 * -------------------------------------------------------------------------- */
int sgLoadWidgetFonts(sgWidget *widget, const char *normal, 
                       const char *bold, const char *fixed)
{
  if(sgLoadWidgetFont(widget, SG_FONT_NORMAL, normal))
    return -1;

  if(sgLoadWidgetFont(widget, SG_FONT_BOLD, bold))
    return -1;

  if(sgLoadWidgetFont(widget, SG_FONT_FIXED, fixed))
    return -1;  
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Sets the caption of a widget (va_list)                                     *
 * -------------------------------------------------------------------------- */
void sgvSetWidgetCaption(sgWidget *widget, const char *caption, va_list args)
{
  vsnprintf(widget->caption, 255, caption, args);
  widget->caption[sizeof(widget->caption) - 1] = '\0';
  
  sgSetWidgetStatus(widget, SG_REDRAW_CONTENT);
}

/* -------------------------------------------------------------------------- *
 * Sets the widget caption (vararg)                                           *
 * -------------------------------------------------------------------------- */
void sgSetWidgetCaption(sgWidget *widget, const char *caption, ...)
{
  va_list args;
  va_start(args, caption);
  sgvSetWidgetCaption(widget, caption, args);
  va_end(args);
}

/* -------------------------------------------------------------------------- *
 * Gets widget caption                                                        *
 * -------------------------------------------------------------------------- */
char *sgGetWidgetCaption(sgWidget *widget)
{
  return widget->caption;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int sgReportWidgetEvent(sgWidget *widget, sgEvent event)
{
  if(widget->proc)
    return widget->proc(widget, event);
  
  return 0;
}  
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int sgHandleWidgetEvent(sgWidget *widget, SDL_Event *event)
{
  sgWidget *child;
  
  if(widget->methods.handler)
    if(widget->methods.handler(widget, event))
      return 1;
  
  /* Do not handle children of disabled widgets:
     its up to the disabled widget itself if it will
     handle any events */
  if(widget->status & SG_DISABLED)
    return 0;
  
  sgForeach(&widget->list, child)
  {
    SDL_Event revent;
    revent = *event;
    
    /* On a mouse button or motion event make the position relative to the child */
    if(revent.type == SDL_MOUSEBUTTONDOWN || 
       revent.type == SDL_MOUSEBUTTONUP)
    {
      revent.button.x -= child->rect.x;
      revent.button.y -= child->rect.y;
    }
    
    if(revent.type == SDL_MOUSEMOTION)
    {
      revent.motion.x -= child->rect.x;
      revent.motion.y -= child->rect.y;
    }
    
    
    if(sgHandleWidgetEvent(child, &revent))
      return 1;
  }

  return 0;
}

/* -------------------------------------------------------------------------- *
 * Renders a widget to its surfaces                                           *
 * -------------------------------------------------------------------------- */
void sgRedrawWidget(sgWidget *widget)
{
  /* Create new border surface if needed */
  if((widget->status & SG_REDRAW_BORDER))
    sgCreateWidgetBorder(widget);
  
  /* Create new frame surface if needed */
  if((widget->status & SG_REDRAW_FRAME))
    sgCreateWidgetFrame(widget);
  
  /* Create new content surface if needed */
  if((widget->status & SG_REDRAW_CONTENT))
    sgCreateWidgetContent(widget);

  /* When redraw is needed and there is a callback: do it :D */
  if(widget->status & (SG_REDRAW_NEEDED))
  {
    if(widget->methods.redraw)
      widget->methods.redraw(widget);    
  }
  
  /* Convert surfaces to display format if any of them changed */
  
  if((widget->status & SG_REDRAW_BORDER))
    sgConvertWidgetBorder(widget);
  
  if((widget->status & SG_REDRAW_FRAME))
    sgConvertWidgetFrame(widget);
  
  if((widget->status & SG_REDRAW_CONTENT))
    sgConvertWidgetContent(widget);
}

/* -------------------------------------------------------------------------- *
 * Blits widget to a surface                                                  *
 * -------------------------------------------------------------------------- */
int sgBlitWidget(sgWidget *widget, SDL_Surface *surface, Sint16 x, Sint16 y)
{
  SDL_Rect rect;
  sgWidget *child;
  
  if(widget->parent == NULL)
  {
    if((widget->status & (SG_REDRAW_BORDER|SG_REDRAW_FRAME|
                          SG_REDRAW_CONTENT|SG_REDRAW_SCREEN)) == 0)
      return 0;
  }

  
  
  /* Do not blit hidden widgets */
  if(widget->status & SG_HIDDEN)
    return 0;
  
  /* Redraw widgets faces if needed */
  sgRedrawWidget(widget);
  
  rect = widget->rect;
  rect.x += x;
  rect.y += y;
  
  /* Blit the surfaces to screen */
  if(widget->face.frame)
    SDL_BlitSurface(widget->face.frame, NULL, surface, &rect);
  
  if(widget->face.border)
   {
  
      
    SDL_BlitSurface(widget->face.border, NULL, surface, &rect);
  }
  
  
  if(widget->face.content)
    SDL_BlitSurface(widget->face.content, NULL, surface, &rect);
  
  /* Recurse into children */
  sgForeachUp(&widget->list, child)
    if(child->methods.blit)
      child->methods.blit(child, surface, rect.x, rect.y);
  

  /* Reset redraw flags */
  sgClearWidgetStatus(widget, SG_REDRAW_NEEDED);
  
  return 1;
}


/* -------------------------------------------------------------------------- *
 * A wrapper to SDL_GetMouseState which returns a position relative to the    *
 * widget                                                                     *
 * -------------------------------------------------------------------------- */
Uint8 sgGetWidgetMouse(sgWidget *widget, Sint16 *x, Sint16 *y)
{
  Uint8 state;
  int xpos, ypos;
  sgWidget *w;
  
  state = SDL_GetMouseState(&xpos, &ypos);
  
  for(w = widget; w; w = w->parent)
  {
    xpos -= w->rect.x;
    ypos -= w->rect.y;
  }
  
  *x = xpos;
  *y = ypos;
  
  return state;
}

/* -------------------------------------------------------------------------- *
 * Indicates whether the widget and all its parents have focus                *
 * -------------------------------------------------------------------------- */
int sgHasWidgetFocus(sgWidget *widget)
{
  while(widget)
  {
    if((widget->status & SG_FOCUS) == 0)
      return 0;
    
    widget = widget->parent;
  }
  
  return 1;
}  

/* Handles motion highlite
 * -------------------------------------------------------------------------- */
void sgHandleWidgetHilite(sgWidget *widget, SDL_Rect rect, sgDraw *draw,
                          Sint16 x, Sint16 y)
{
  if(sgMatchRect(&rect, x, y))
  {
    if((*draw & SG_DRAW_HIGH) == 0)
    {
      *draw |= SG_DRAW_HIGH;
      sgSetWidgetStatus(widget, SG_REDRAW_FRAME);
    }
  }
  else
  {
    if((*draw & SG_DRAW_HIGH))
    {
      *draw &= ~SG_DRAW_HIGH;
      sgSetWidgetStatus(widget, SG_REDRAW_FRAME);
    }
  }
}

/** @} */
