/* $Id: colorsel.c,v 1.4 2005/05/19 23:08:32 smoli Exp $
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

/** @weakgroup sgColorSel
 *  @{
 */

#include <libsgui/sgui.h>

/* Defines the widget type and its callbacks
 * ------------------------------------------------------------------------- */
sgWidgetType sgColorSelType =
{
  .name = "sgColorSel", 
  .size = sizeof(sgColorSel),
  .methods =
  {
    .recalc = sgRecalcColorSel,
    .redraw = sgRedrawColorSel,
    .handler = sgHandleColorSelEvent,
    .blit = sgBlitColorSel
  }
};

/* Creates a new colorsel widget by splitting another
 * ------------------------------------------------------------------------- */
sgWidget *sgNewColorSelSplitted(sgWidget *based, sgEdge edge, Uint16 pixels,
                                sgColorSelMode mode, const char *caption)
{
  sgWidget *colorsel;
  SDL_Rect  newrect;
  
  sgSplitWidget(based, &newrect, edge, pixels);

  colorsel = sgNewColorSel(based->parent, newrect.x, newrect.y, newrect.w, newrect.h, 
                           mode, caption);

  return colorsel;
}

/* Creates a new colorsel widget and subtracts it from a group
 * ------------------------------------------------------------------------- */
sgWidget *sgNewColorSelGrouped(sgWidget *group, sgEdge edge, sgAlign align,
                               Uint16 w, Uint16 h, sgColorSelMode mode,
                               const char *caption)
{
  sgWidget *colorsel = sgNewColorSel(group, 0, 0, w, h, mode, caption);
  
  sgSubGroup(group, colorsel, edge, align);
  
  return colorsel;
}

/* Creates a new colorsel widget and adds it to a group
 * ------------------------------------------------------------------------- */
sgWidget *sgNewColorSelAligned(sgWidget *group, sgEdge edge, sgAlign align,
                               Uint16 w, Uint16 h, sgColorSelMode mode,
                               const char *caption)
{
  sgWidget *colorsel = sgNewColorSel(group, 0, 0, w, h, mode, caption);
  
  sgAddGroup(group, colorsel, edge, align);
  
  return colorsel;
}

/* Creates a new color select widget from a rectangle
 * ------------------------------------------------------------------------- */
sgWidget *sgNewColorSelRect(sgWidget *parent, SDL_Rect rect,
                            sgColorSelMode mode, const char *caption)
{
  return sgNewColorSel(parent, rect.x, rect.y, rect.w, rect.h,
                       mode, caption);
}

/* Creates a new color select widget
 * ------------------------------------------------------------------------- */
sgWidget *sgNewColorSel(sgWidget *parent, Sint16 x, Sint16 y, Uint16 w, Uint16 h,
                        sgColorSelMode mode, const char *caption)
{
  sgWidget *colorsel;
  
  /* Create widget structure */
  colorsel = sgNewWidget(&sgColorSelType, parent, x, y, w, h, caption);
  
  sgColorSel(colorsel)->mode = mode;
  sgColorSel(colorsel)->max = 255;
  sgColorSel(colorsel)->pos = 0;
  
  sgColorSel(colorsel)->hsv.h = 255;
  sgColorSel(colorsel)->hsv.s = 255;
  sgColorSel(colorsel)->hsv.v = 255;
  sgColorSel(colorsel)->hsv.a = 255;
  
  switch(mode)
  {
    case SG_COLORSEL_HUE:
      sgColorSel(colorsel)->hsv.h = 0;
      break;
    case SG_COLORSEL_SATURATION:
      sgColorSel(colorsel)->hsv.s = 0;
      break;
    case SG_COLORSEL_VALUE:
      sgColorSel(colorsel)->hsv.v = 0;
      break;
  }
  
  sgRecalcWidget(colorsel);
  
  return colorsel;
}

/* Recalculates internal widget rectangles according to widget->size
 * ------------------------------------------------------------------------- */
void sgRecalcColorSel(sgWidget *colorsel)
{
  int knobsize;
  SDL_Rect rect;
  sgFont   *font;
  Uint16    minh, minw;
  Uint16    textsize;
  
  /* Get the font we're using for the value and caption */
  font = colorsel->font[SG_FONT_NORMAL];
  
  /* Calc minimal dimensions */
  minh = sgFontHeight(font) + 2;
  minw = colorsel->border * 4 + 32 + minh + sgTextWidth(font, colorsel->caption);

  if(colorsel->caption[0])
    textsize = (sgTextWidth(font, colorsel->caption) + colorsel->border * 3);
  else
    textsize = 0;
  
  /* Apply minimal dimensions */
/*  if(colorsel->rect.h < minh) colorsel->rect.h = minh;
  if(colorsel->rect.w < minw) colorsel->rect.w = minw;*/
  
  rect.x = 0;
  rect.y = 0;
  rect.w = colorsel->rect.w;
  rect.h = colorsel->rect.h;

  /* width of the knob */
  knobsize = rect.h / 2;
  
/*  if(up)
    titlewidth = rect.wh - (sgColorSel(colorsel)->len + knobsize + numsize);
  else
    titlewidth = rect.w - (sgColorSel(colorsel)->len + knobsize + numsize);*/
    
  /* Rectangle for the caption */
  if(textsize)
    sgSplitRect(&rect, &sgColorSel(colorsel)->caption, SG_EDGE_LEFT, textsize);
  
  sgSplitRect(&rect, &sgColorSel(colorsel)->preview, SG_EDGE_RIGHT, rect.h);
  
  sgSubBorder(&sgColorSel(colorsel)->preview, colorsel->border);
  
//  sgPadRect(&sgColorSel(colorsel)->preview, SG_EDGE_LEFT|SG_EDGE_RIGHT, colorsel->border);
  
  /* hidden range rect */
  
  rect.w += 4;
  
  sgColorSel(colorsel)->knob = rect;
  sgColorSel(colorsel)->knob.w = knobsize;

  sgPadRect(&rect, SG_EDGE_LEFT|SG_EDGE_RIGHT, knobsize / 2);
  sgPadRect(&rect, SG_EDGE_TOP|SG_EDGE_BOTTOM, 8);

  sgColorSel(colorsel)->outer = rect;
  sgSubBorder(&rect, 2);
  sgColorSel(colorsel)->inner = rect;
//  sgSubBorder(&rect, 2);
  sgColorSel(colorsel)->range = rect;
  
    
  sgColorSel(colorsel)->len = sgColorSel(colorsel)->range.w;
  sgColorSel(colorsel)->max = 255;
 
  sgColorSel(colorsel)->pos = sgColorSel(colorsel)->pos + 1;
  sgSetColorSelPos(colorsel, sgColorSel(colorsel)->pos - 1);
    
  
  sgSetWidgetStatus(colorsel, SG_REDRAW_NEEDED);
}

/* Redraws colorsel widget
 * ------------------------------------------------------------------------- */
static void sgDrawColorSelGradient(sgWidget *colorsel)
{
  SDL_Rect rect = sgColorSel(colorsel)->range;
  int pos;
  Uint32 *pixels;
  Uint16 y;
  Uint32 add;
  Uint32 offset;
  sgHSV hsv;
  sgColor rgb;

  SDL_LockSurface(colorsel->face.content);
  
  add = (colorsel->face.content->pitch >> 2);
  pixels = colorsel->face.content->pixels;
  pixels += rect.y * add;
  pixels += rect.x;
  
  offset = sgColorSel(colorsel)->pos * 255 / sgColorSel(colorsel)->len;
  
  for(pos = 0; pos < rect.w; pos++)
  {
    SDL_Rect tmprect = rect;
    int value = pos * 255 / rect.w;
    Uint32 color;
    Uint32 *row;

    hsv = sgColorSel(colorsel)->hsv;
  
    switch(sgColorSel(colorsel)->mode)
    {
      case SG_COLORSEL_HUE: hsv.h = value; break;
      case SG_COLORSEL_SATURATION: hsv.s = value; break;
      case SG_COLORSEL_VALUE: hsv.v = value; break;
    }

    rgb = sgHSVToRGB(hsv);
    
    color = (rgb.r << RSHIFT) | (rgb.g << GSHIFT) | 
            (rgb.b << BSHIFT) | AMASK;
    
    if(pos == 0 && sgColorSel(colorsel)->window.x < sgColorSel(colorsel)->range.x)
    {
      tmprect.x = sgColorSel(colorsel)->window.x;
      tmprect.w = sgColorSel(colorsel)->range.x - sgColorSel(colorsel)->window.x;
      
      SDL_FillRect(colorsel->face.content, &tmprect, color);
    }
            
    if(pos == rect.w - 1 && 
       sgColorSel(colorsel)->window.x + sgColorSel(colorsel)->window.w >
       sgColorSel(colorsel)->range.x + sgColorSel(colorsel)->range.w)
    {
      tmprect.x = sgColorSel(colorsel)->range.x + sgColorSel(colorsel)->range.w;
      tmprect.w = (sgColorSel(colorsel)->window.x + sgColorSel(colorsel)->window.w) -
        tmprect.x;
      
      SDL_FillRect(colorsel->face.content, &tmprect, color);
    }
            
    row = &pixels[pos];
    
    for(y = 0; y < rect.h; y++)
    {
      *row = color;
      row += add;
    }
  }
  
  SDL_UnlockSurface(colorsel->face.content);
  
}

void sgRedrawColorSel(sgWidget *colorsel) 
{
  if(sgRedrawWidgetBorder(colorsel))
  {
    if(sgHasWidgetFocus(colorsel))
    {
      sgDrawWidgetBorder(colorsel, &sgColorSel(colorsel)->outer);
      SDL_FillRect(colorsel->face.border, &sgColorSel(colorsel)->knob, 0);
      SDL_FillRect(colorsel->face.border, &sgColorSel(colorsel)->preview, 0);
    }
    else
    {
      SDL_Rect rect;
      
      sgDrawFrame(colorsel->face.border, &sgColorSel(colorsel)->inner,
                  SG_DRAW_INVERSE|SG_DRAW_FILL);
      
      rect = sgColorSel(colorsel)->preview;
      
      sgSubBorder(&rect, 2);
      
/*      sgDrawFrame(colorsel->face.border, &sgColorSel(colorsel)->preview,
                  SG_DRAW_INVERSE|SG_DRAW_FILL);*/
      sgDrawFrame(colorsel->face.border, &sgColorSel(colorsel)->window,
                  SG_DRAW_INVERSE|SG_DRAW_FILL);
    }
    
  }
  
  /* Draw color select frames */
  if(sgRedrawWidgetFrame(colorsel))
  {
    sgDrawFrame(colorsel->face.frame, &sgColorSel(colorsel)->outer,
                sgColorSel(colorsel)->rdraw);
    
    sgDrawFrame(colorsel->face.frame, &sgColorSel(colorsel)->knob,
                sgColorSel(colorsel)->pushed|SG_DRAW_FILL);
    
    sgDrawFrame(colorsel->face.frame, &sgColorSel(colorsel)->preview, 0);
    
    sgDrawFrame(colorsel->face.frame, &sgColorSel(colorsel)->window,
                (sgColorSel(colorsel)->pushed & ~SG_DRAW_INVFILL)|SG_DRAW_CLEAR|SG_DRAW_INVERSE);
  }
  
  if(sgRedrawWidgetContent(colorsel))
  {
    Uint32 color;
    sgColor rgb;
    sgHSV hsv;
    
    /* Draw widget caption */
    if(colorsel->caption[0])
      sgDrawTextOutline(colorsel->font[SG_FONT_NORMAL],
                        colorsel->face.content, &sgColorSel(colorsel)->caption,
                        SG_ALIGN_CENTER|SG_ALIGN_MIDDLE, 
                        colorsel->caption);
    
    hsv = sgColorSel(colorsel)->hsv;
  
    switch(sgColorSel(colorsel)->mode)
    {
      case SG_COLORSEL_HUE: hsv.h = sgColorSel(colorsel)->pos; break;
      case SG_COLORSEL_SATURATION: hsv.s = sgColorSel(colorsel)->pos; break;
      case SG_COLORSEL_VALUE: hsv.v = sgColorSel(colorsel)->pos; break;
    }

    rgb = sgHSVToRGB(hsv);
    
    color = (rgb.r << RSHIFT) | (rgb.g << GSHIFT) | 
            (rgb.b << BSHIFT) | AMASK;
    
    SDL_FillRect(colorsel->face.content, &sgColorSel(colorsel)->preview, color);
    /* Redraw preview */
/*    numrect = sgColorSel(colorsel)->preview;
    numrect.y -= 1;
    numrect.h += 2;
    numrect.x -= 1;
    numrect.w += 2;
    SDL_FillRect(colorsel->face.content, &numrect, 0);*/
    sgDrawColorSelGradient(colorsel);
  }
}

/* Handles an incoming event for the color select widget
 * -------------------------------------------------------------------------- */
int sgHandleColorSelEvent(sgWidget *colorsel, SDL_Event *event) 
{
  int up = 0;
  if(colorsel->rect.h > colorsel->rect.w) up = 1;
  
  if(event->type == SDL_MOUSEMOTION)
  {
    if(!sgColorSel(colorsel)->cursor && sgMatchRect(&sgColorSel(colorsel)->knob, event->motion.x, event->motion.y))
    {
      sgSetDialogCursor(colorsel->dialog, SG_CURSOR_RESIZE_H);
      sgSetWidgetStatus(colorsel->dialog, SG_REDRAW_MOUSE);
      sgColorSel(colorsel)->cursor = 1;      
//      return 1;
    }
    
    if(sgColorSel(colorsel)->cursor && !sgColorSel(colorsel)->pushed &&
       !sgMatchRect(&sgColorSel(colorsel)->knob, event->motion.x, event->motion.y))
    {
      sgSetDialogCursor(colorsel->dialog, SG_CURSOR_DEFAULT);
      sgSetWidgetStatus(colorsel->dialog, SG_REDRAW_MOUSE);
      sgColorSel(colorsel)->cursor = 0;
//      return 1;
    }
  }
  
  /* ColorSel must have focus to generate events */
  if(sgHasWidgetFocus(colorsel))
  {
    /* Button was pushed and we aren't colorseling */
    if(sgEventButton(event, SDL_BUTTON_LEFT, SG_PRESSED) && !(sgColorSel(colorsel)->pushed & SG_DRAW_INVFILL))
    {
      /* When the click hits the knob then begin scrolling */
      if(sgMatchRect(&sgColorSel(colorsel)->knob, event->button.x, event->button.y))
      {
        sgColorSel(colorsel)->pushed |= SG_DRAW_INVFILL;
        
        sgColorSel(colorsel)->offset = 
          (up ? (event->button.y - sgColorSel(colorsel)->knob.y) : 
                (event->button.x - sgColorSel(colorsel)->knob.x));
        
        sgSetWidgetStatus(colorsel, SG_REDRAW_FRAME);
        return 1;
      }
      
      /* ignore clicks outside range */
      if(up && event->button.y > colorsel->rect.h)
        return 0;
      
      if(!up && event->button.x > colorsel->rect.w)
        return 0;
      
      /* When the click hits the range, then set the position */
      if(sgMatchRect(&sgColorSel(colorsel)->outer, event->button.x, event->button.y))
      {
        int pos;
        int preview;

        if(up)
        { 
          pos = event->button.y - sgColorSel(colorsel)->outer.y - sgColorSel(colorsel)->outer.h;
          preview = sgColorSel(colorsel)->max - pos * sgColorSel(colorsel)->max / sgColorSel(colorsel)->outer.h;
         }
        else
        {
          pos = event->button.x - sgColorSel(colorsel)->outer.x;
          preview = pos * sgColorSel(colorsel)->max / sgColorSel(colorsel)->outer.w;
        }

        sgColorSel(colorsel)->rdraw &= ~SG_DRAW_HIGH;
        sgColorSel(colorsel)->pushed |= SG_DRAW_HIGH;
        
        return sgSetColorSelPos(colorsel, preview);
      }
    }
    
    /* Button was released and we are adjusting */
    if(sgEventButton(event, SDL_BUTTON_LEFT, SG_RELEASED) && (sgColorSel(colorsel)->pushed & SG_DRAW_INVFILL))
    {
      if(!sgMatchRect(&sgColorSel(colorsel)->knob, event->button.x, event->button.y))
      {
        sgSetDialogCursor(colorsel->dialog, SG_CURSOR_DEFAULT);
        sgSetWidgetStatus(colorsel->dialog, SG_REDRAW_MOUSE);
      }
      
      sgColorSel(colorsel)->pushed &= ~SG_DRAW_INVFILL;
      sgSetWidgetStatus(colorsel, SG_REDRAW_FRAME);
      return 1;
    }
    
    /* If we are colorseling and the mouse is moving... */
    if(event->type == SDL_MOUSEMOTION && (sgColorSel(colorsel)->pushed & SG_DRAW_INVFILL))
    {
      int x = (Sint16)event->motion.x;
      
      if(sgColorSel(colorsel)->pushed)
      {
        int pos;
        int preview;
        
        
        
        /* ignore motion outside range */
        if(x > colorsel->rect.w)
          return 0;
        
        x -= sgColorSel(colorsel)->offset;
        x -= sgColorSel(colorsel)->range.x;
        
        pos = sgLimit(x, sgColorSel(colorsel)->len);
        
        preview = pos * sgColorSel(colorsel)->max / sgColorSel(colorsel)->len;
        
        return sgSetColorSelPos(colorsel, preview);
      } 
    } 
    
    if(sgEventButton(event, SDL_BUTTON_WHEELUP, SG_PRESSED) ||
       sgEventKey(event, SDLK_LEFT, SG_PRESSED))
    {
      if(sgColorSel(colorsel)->pos > 0)
        return sgSetColorSelPos(colorsel, sgColorSel(colorsel)->pos - 1);
    }
    
    if(sgEventButton(event, SDL_BUTTON_WHEELDOWN, SG_PRESSED) ||
       sgEventKey(event, SDLK_RIGHT, SG_PRESSED))
    {
      if(sgColorSel(colorsel)->pos < sgColorSel(colorsel)->max) 
        return sgSetColorSelPos(colorsel, sgColorSel(colorsel)->pos + 1);
    }
  }

  if(event->type == SDL_MOUSEMOTION)
  {
    sgHandleWidgetHilite(colorsel, sgColorSel(colorsel)->outer, 
                         &sgColorSel(colorsel)->rdraw, 
                         event->motion.x, event->motion.y);
    
    sgHandleWidgetHilite(colorsel, sgColorSel(colorsel)->knob, 
                         &sgColorSel(colorsel)->pushed, 
                         event->motion.x, event->motion.y);
    
    if(sgColorSel(colorsel)->pushed & SG_DRAW_HIGH)
      sgColorSel(colorsel)->rdraw &= ~SG_DRAW_HIGH;
  }
  
  /* Cancel adjusting if we're not focused */
  if((colorsel->status & SG_FOCUS) == 0 && sgColorSel(colorsel)->pushed)
  {
    sgColorSel(colorsel)->pushed = 0;
    sgSetWidgetStatus(colorsel, SG_REDRAW_FRAME);
    return 1;
  }

  return 0;
}

/* Sets the position of the color select knob
 * ------------------------------------------------------------------------- */
int sgSetColorSelPos(sgWidget *colorsel, int newpos)
{
  int up = 0;
  if(colorsel->rect.h > colorsel->rect.w) up = 1;

  if(newpos != sgColorSel(colorsel)->pos)
  {
    int pos;
    
    sgColorSel(colorsel)->pos = newpos;

    switch(sgColorSel(colorsel)->mode)
    {
      case SG_COLORSEL_HUE:
        sgColorSel(colorsel)->hsv.h = newpos;
        break;
      case SG_COLORSEL_SATURATION:
        sgColorSel(colorsel)->hsv.s = newpos;
        break;
      case SG_COLORSEL_VALUE:
        sgColorSel(colorsel)->hsv.v = newpos;
        break;
    }
  
    pos = newpos * sgColorSel(colorsel)->len / sgColorSel(colorsel)->max;
    
    sgColorSel(colorsel)->knob.x =
      sgColorSel(colorsel)->range.x + pos -
      sgColorSel(colorsel)->knob.w / 2;
    
    sgColorSel(colorsel)->window = sgColorSel(colorsel)->knob;
    sgPadRect(&sgColorSel(colorsel)->window, SG_EDGE_LEFT|SG_EDGE_RIGHT, 4);
    
    sgColorSel(colorsel)->window.y = sgColorSel(colorsel)->range.y;
    sgColorSel(colorsel)->window.h = sgColorSel(colorsel)->range.h;
  
    sgAddBorder(&sgColorSel(colorsel)->window, 2);
  
    sgReportWidgetEvent(colorsel, SG_EVENT_CHANGE);
    
    sgSetWidgetStatus(colorsel, SG_REDRAW_FRAME|SG_REDRAW_CONTENT|SG_REDRAW_BORDER);
    return 1;
  }
  
  return 0;
}


/* Gets the position of the color select knob
 * ------------------------------------------------------------------------- */
int sgGetColorSelPos(sgWidget *colorsel, int *pos)
{
  if(pos)
    *pos = sgColorSel(colorsel)->pos;
  
  return sgColorSel(colorsel)->pos;
}

/* Set the hue value of the color select widget */
void sgSetColorSelHue(sgWidget *colorsel, Uint8 hue)
{
  sgColorSel(colorsel)->hsv.h = hue;
  
  if(sgColorSel(colorsel)->mode == SG_COLORSEL_HUE)
    sgSetColorSelPos(colorsel, hue);
  
  sgSetWidgetStatus(colorsel, SG_REDRAW_CONTENT);
}

/* Set the saturation value of the color select widget */
void sgSetColorSelSaturation(sgWidget *colorsel, Uint8 saturation)
{
  sgColorSel(colorsel)->hsv.s = saturation;
  
  if(sgColorSel(colorsel)->mode == SG_COLORSEL_SATURATION)
    sgSetColorSelPos(colorsel, saturation);

  sgSetWidgetStatus(colorsel, SG_REDRAW_CONTENT);
}

/* Set the value of the color select widget */
void sgSetColorSelValue(sgWidget *colorsel, Uint8 value)
{
  sgColorSel(colorsel)->hsv.v = value;
  
  if(sgColorSel(colorsel)->mode == SG_COLORSEL_VALUE)
    sgSetColorSelPos(colorsel, value);

  sgSetWidgetStatus(colorsel, SG_REDRAW_CONTENT);
}

/* Get the hue value of the color select widget */
Uint8 sgGetColorSelHue(sgWidget *colorsel)
{
  return sgColorSel(colorsel)->hsv.h;
}

/* Get the saturation value of the color select widget */
Uint8 sgGetColorSelSaturation(sgWidget *colorsel)
{
  return sgColorSel(colorsel)->hsv.s;
}

/* Get the value of the color select widget */
Uint8 sgGetColorSelValue(sgWidget *colorsel)
{
  return sgColorSel(colorsel)->hsv.v;
}

/* Get the color of the color select widget */
sgColor sgGetColorSelRGB(sgWidget *colorsel)
{
  return sgHSVToRGB(sgColorSel(colorsel)->hsv);
}

/* Set the color of the color select widget */
void sgSetColorSelRGB(sgWidget *colorsel, sgColor rgb)
{
  sgHSV hsv = sgRGBToHSV(rgb);
  
  sgSetColorSelHSV(colorsel, hsv);
}

/* Get the color of the color select widget */
sgHSV sgGetColorSelHSV(sgWidget *colorsel)
{
  return sgColorSel(colorsel)->hsv;
}

/* Set the color of the color select widget */
void sgSetColorSelHSV(sgWidget *colorsel, sgHSV hsv)
{
  sgColorSel(colorsel)->hsv = hsv;
  
  switch(sgColorSel(colorsel)->mode)
  {
    case SG_COLORSEL_HUE:
      sgSetColorSelHue(colorsel, hsv.h);
      break;
    case SG_COLORSEL_SATURATION:
      sgSetColorSelSaturation(colorsel, hsv.s);
      break;
    case SG_COLORSEL_VALUE:
      sgSetColorSelValue(colorsel, hsv.v);
      break;
  }
}

int sgBlitColorSel(sgWidget *colorsel, SDL_Surface *surface, Sint16 x, Sint16 y)
{
  SDL_Rect rect;
  
  /* Do not blit hidden widgets */
  if(colorsel->status & SG_HIDDEN)
    return 0;
  
  /* Redraw widgets faces if needed */
  sgRedrawWidget(colorsel);
  
  rect = colorsel->rect;
  rect.x += x;
  rect.y += y;
  
  /* Blit the surfaces to screen */
  if(colorsel->face.content)
    SDL_BlitSurface(colorsel->face.content, NULL, surface, &rect);
  
  if(colorsel->face.border)
    SDL_BlitSurface(colorsel->face.border, NULL, surface, &rect);
  
  if(colorsel->face.frame)
    SDL_BlitSurface(colorsel->face.frame, NULL, surface, &rect);

  /* Reset redraw flags */
  sgClearWidgetStatus(colorsel, SG_REDRAW_NEEDED);
  
  return 1;
}

/** @} */
