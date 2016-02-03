/* $Id: group.c,v 1.21 2005/05/21 10:09:34 smoli Exp $
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

/** @weakgroup sgGroup
 *  @{
 */

#include <libsgui/sgui.h>
#include <libsgui/png.h>

/* Defines the widget type and its callbacks
 * ------------------------------------------------------------------------- */
sgWidgetType sgGroupType =
{
  .name = "sgGroup",
  .size = sizeof(struct sgGroup),
  .methods =
  {
    .recalc = sgRecalcGroup,
    .redraw = sgRedrawGroup,
    .handler = sgHandleGroupEvent,
    .blit = sgBlitWidget
  }
};

/* -------------------------------------------------------------------------- *
 * Creates a new group widget using full dialog size                          *
 * -------------------------------------------------------------------------- */
sgWidget *sgNewGroupFull(sgWidget *parent, const char *caption) 
{
  SDL_Rect rect = parent->rect;
  
  if(parent->type == &sgGroupType)
  {
    rect = sgGroup(parent)->splitted;
  }
  else
  {
    rect.x -= parent->rect.x;
    rect.y -= parent->rect.y;
    
    sgSubBorder(&rect, parent->border);
  }
  
  
  return sgNewGroup(parent, rect.x, rect.y,
                    rect.w, rect.h, caption);
}

/* -------------------------------------------------------------------------- */
sgWidget *sgNewGroupAligned(sgWidget *group, sgEdge edge, sgAlign align,
                            Uint16 w, Uint16 h, const char *caption)

{  
  sgWidget *newgroup;
  
  newgroup = sgNewGroup(group, 0, 0, w, h, caption);
  
  sgSubGroup(group, newgroup, edge, align);
  
  return newgroup;
}

/* Creates a new group widget by splitting another
 * -------------------------------------------------------------------------- */
sgWidget *sgNewGroupSplitted(sgWidget *based, sgEdge edge, int pixels,
                              const char *caption)

{  
  sgWidget *group;
  SDL_Rect  newrect;
  
  sgSplitWidget(based, &newrect, edge, pixels);
  group = sgNewGroup(based->parent, newrect.x, newrect.y, newrect.w, newrect.h, caption);
  
  return group;
}

/* -------------------------------------------------------------------------- *
 * Creates a new group widget from a rectangle                                *
 * -------------------------------------------------------------------------- */
sgWidget *sgNewGroupRect(sgWidget *parent, SDL_Rect rect, const char *caption) 
{
  return sgNewGroup(parent, rect.x, rect.y, rect.w, rect.h, caption);
}

/* -------------------------------------------------------------------------- *
 * Creates a new group widget                                                 *
 * -------------------------------------------------------------------------- */
sgWidget *sgNewGroup(sgWidget *parent, Sint16 x, Sint16 y, Uint16 w, Uint16 h,
                     const char *caption) 
{
  sgWidget *group;
  
  /* Create group structure */
  group = sgNewWidget(&sgGroupType, parent, x, y, w, h, caption);

  if(parent->type == &sgTabType)
    sgAddTabGroup(parent, group);
  
  sgSetGroupAlign(group, SG_ALIGN_CENTER|SG_ALIGN_MIDDLE);
  
  return group;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void sgRecalcGroup(sgWidget *group)
{
  sgFont *font;
  Uint16 minh, minw;
  
  /* Get the font we're drawing to the group title */
  font = group->font[SG_FONT_BOLD];
  
  /* Calculate minimal dimensions based on font */
  minw = sgTextWidth(font, group->caption) + (group->border << 1) + 2;
  minh = sgFontHeight(font) + 3 + (group->border << 1) + 2;
  
  /* Apply them */
  if(group->rect.h < minh) group->rect.h = minh;
  if(group->rect.w < minw) group->rect.w = minw;
  
  /* Calc rectangle for the outer group frame */
  sgGroup(group)->outer.x = 0;
  sgGroup(group)->outer.y = 0;
  sgGroup(group)->outer.w = group->rect.w;
  sgGroup(group)->outer.h = group->rect.h;
  
  sgSubBorder(&sgGroup(group)->outer, group->border);
  
  /* Calc rectangle for the inner group frame */
  sgGroup(group)->inner = sgGroup(group)->outer;
  sgSubBorder(&sgGroup(group)->inner, 2);
  
  /* Split off the caption rectangle */
  if(group->caption[0])
    sgSplitRect(&sgGroup(group)->inner, 
                &sgGroup(group)->caption, SG_EDGE_TOP, sgFontHeight(font));
  
  /* Calc rectangle for the group body frame */
  sgGroup(group)->body = sgGroup(group)->inner;
  sgSubBorder(&sgGroup(group)->body, 2);
  
  /* Body rectangle is untouched, but this one 
     will get splitted when widgets are added: */
  sgGroup(group)->splitted = sgGroup(group)->body;
  
  if(sgGroup(group)->width)
  {
    sgGroup(group)->tab = sgGroup(group)->outer;
    
    sgGroup(group)->tab.h = sgGroup(group)->inner.y - 
                            sgGroup(group)->outer.y - 2;

    sgGroup(group)->tab.x += sgGroup(group)->offset;
    sgGroup(group)->tab.w = sgGroup(group)->width;
    
    
    sgGroup(group)->caption = sgGroup(group)->tab;
  }
  
  
  
  sgSetWidgetStatus(group, SG_REDRAW_NEEDED);
}

/* -------------------------------------------------------------------------- *
 * Draws group look                                                           *
 * -------------------------------------------------------------------------- */
void sgRedrawGroup(sgWidget *group) 
{ 
  if(sgRedrawWidgetFrame(group))
  {
    if(sgGroup(group)->width)
    {
      SDL_Rect rect;
      
      rect = sgGroup(group)->outer;
      rect.h -= sgGroup(group)->inner.y - sgGroup(group)->outer.y;
      rect.y = sgGroup(group)->inner.y;
      
      sgDrawFrame(group->face.frame, &sgGroup(group)->outer,
                  SG_DRAW_NORMAL);
      sgDrawFrame(group->face.frame, &rect,
                  SG_DRAW_NORMAL);
      
      rect.y = sgGroup(group)->tab.y;
      rect.h = sgGroup(group)->tab.h + 2;
      
      rect.x = 0;
      rect.w = sgGroup(group)->tab.x;
      
      if(sgGroup(group)->tab.x)
      {
        SDL_FillRect(group->face.frame, &rect, 0);
        
        rect.x += rect.w;
        
        sgBlendVLine(group->face.frame, rect.x, rect.y, rect.y + rect.h);
        sgBlendVLine(group->face.frame, rect.x + 1, rect.y + 1, rect.y + rect.h + 1);
      }
      
      rect.x += sgGroup(group)->tab.w;
      rect.w = group->rect.w;
      
      if(rect.w)
      {
        SDL_FillRect(group->face.frame, &rect, 0);
        
        sgBlendVLine(group->face.frame, rect.x - 2, rect.y, rect.y + rect.h + 1);
        sgBlendVLine(group->face.frame, rect.x - 1, rect.y, rect.y + rect.h);
      }
      
      rect.x -= 2;
      
      /* Draw the tab content */
      rect = sgGroup(group)->tab;
      
      sgSubBorder(&rect, 2);
      rect.h += 6;

      sgDrawFrame(group->face.frame, &rect,
                  (group->node.prev ? 0 : SG_DRAW_INVERSE)|SG_DRAW_FILL);
      
      
/*      rect = sgGroup(group)->tab;
      
      sgSubBorder(&rect, 2);
      rect.h += 4;
      
      rect.y += rect.h;
      rect.h = 2;
      
      SDL_FillRect(group->face.frame, &rect, 0);*/
      
    }
    else
    {
      sgDrawFrame(group->face.frame, &sgGroup(group)->outer,
                  SG_DRAW_NORMAL|SG_DRAW_FILL);

      SDL_FillRect(group->face.frame, &sgGroup(group)->inner, 0);
      
      
      sgDrawFrame(group->face.frame, &sgGroup(group)->inner, 
                  SG_DRAW_INNER|SG_DRAW_INVERSE);
    }
  }

  if(sgRedrawWidgetBorder(group))
  {
    sgWidget *child;
          
    if(sgGroup(group)->width)
    {
      SDL_Rect rect;
      Uint8 i1, i2;
      
/*      rect = sgGroup(group)->inner;
      sgAddBorder(&rect, 8);
      
      sgDrawFrame(group->face.border, &rect, SG_DRAW_FILL);*/
   
      rect = sgGroup(group)->inner;
/*      rect.h += rect.y - sgGroup(group)->tab.y;
      rect.y = sgGroup(group)->tab.y;*/

      sgDrawFrame(group->face.border, &rect,
                  SG_DRAW_INVERSE|SG_DRAW_FILL);
      
      
      rect.y = sgGroup(group)->tab.y;
      rect.h = sgGroup(group)->tab.h + 2;
      
      rect.x = 0;
      rect.w = sgGroup(group)->tab.x;
      
      if(sgGroup(group)->tab.x)
      {
        SDL_FillRect(group->face.border, &rect, 0);
        
        rect.x += rect.w;
      }
      
      rect.x += sgGroup(group)->tab.w;
      rect.w = group->rect.w;
      
      if(rect.w)
      {
        SDL_FillRect(group->face.border, &rect, 0);
      }
      
      rect = sgGroup(group)->tab;
      
      sgSubBorder(&rect, 2);
      rect.h += 4;
      
      i1 = sgGetPixel8(group->face.border, rect.x, rect.y + rect.h + 4);
      i2 = sgGetPixel8(group->face.border, rect.x + rect.w - 1, rect.y + rect.h + 4);
      
      sgDrawHLine(group->face.border, rect.x, rect.x + rect.w, rect.y + rect.h, i1, i2);
      sgDrawHLine(group->face.border, rect.x, rect.x + rect.w, rect.y + rect.h + 1, i1, i2);
    }
    else
    {
      if(sgHasWidgetFocus(group))
        sgDrawWidgetBorder(group, &sgGroup(group)->outer);

/*      if(!(group->parent && group->parent->parent && group->parent->parent->type == &sgTabType))*/
        sgDrawFrame(group->face.border, &sgGroup(group)->inner, SG_DRAW_FILL|SG_DRAW_INVERSE);
      
    }
    
    sgForeach(&group->list, child)
    {
      SDL_Rect rect;
      
      if(child->type != &sgGroupType && 
         child->type != &sgImageType)
        continue;
      
      if(child->type == &sgGroupType)
        rect = sgGroup(child)->body;

      if(child->type == &sgImageType)
        rect = sgImage(child)->body;

      rect.x += child->rect.x;
      rect.y += child->rect.y;
      
      SDL_FillRect(group->face.border, &rect, 0);                     
    }
  }

  if(sgRedrawWidgetContent(group))
  {
    if(group->caption[0])
      sgDrawTextOutline(group->font[SG_FONT_BOLD],
                        group->face.content, &sgGroup(group)->caption,
                        SG_ALIGN_CENTER|SG_ALIGN_MIDDLE, group->caption);
  }

}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int sgHandleGroupEvent(sgWidget *group, SDL_Event *event)
{
  Sint16 x, y;
  Uint8 state;
  
  state = sgGetWidgetMouse(group, &x, &y);
  
/*  x -= sgGroup(group)->body.x;
  y -= sgGroup(group)->body.y;*/
  
  switch(event->type)
  {
    case SDL_MOUSEMOTION:
    {
      /* If we have focus ourselves, then check children focus */
      if(sgHasWidgetFocus(group) && state == 0)
      {
        sgWidget *widget;
        
        sgForeach(&group->list, widget)
        {
          if(widget->status & SG_DISABLED)
            continue;

          if(sgMatchRect(&widget->area, event->motion.x, event->motion.y))
            break;
        }

        /* Mouse is over a widget but not the one in group->focus */
        if(widget && sgGroup(group)->focus != widget)
        {
          if(sgGroup(group)->focus && (sgGroup(group)->focus->status & SG_GRAB))
            return 0;
          
          if(sgGroup(group)->focus)
          {
            sgClearWidgetStatus(sgGroup(group)->focus, SG_FOCUS);
            sgSetWidgetStatus(sgGroup(group)->focus, SG_REDRAW_BORDER);
          }
          

          sgSetWidgetStatus(widget, SG_FOCUS|SG_REDRAW_NEEDED);
          
          sgGroup(group)->focus = widget;

          sgSetWidgetStatus(group, SG_REDRAW_SCREEN);
        }
      }
    }    
  }
  
  /* Tab-Button was pushed and the mouse-button has been released */
  if(sgGroup(group)->pushed && sgEventButton(event, SDL_BUTTON_LEFT, SG_RELEASED))
  {
    if(sgMatchRect(&sgGroup(group)->tab, event->button.x, event->button.y))
      sgReportWidgetEvent(group, SG_EVENT_CLICK);
    
    sgReportWidgetEvent(group, SG_BUTTON_UP);
    sgGroup(group)->pushed = 0;
    return 1;
  }
  
  if(!sgGroup(group)->pushed && sgEventButton(event, SDL_BUTTON_LEFT, SG_PRESSED))
  {
    if(sgMatchRect(&sgGroup(group)->tab, event->button.x, event->button.y))
    {
      sgReportWidgetEvent(group, SG_BUTTON_DOWN);
      sgGroup(group)->pushed = 1;
      return 1;
    }
  }
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Adds a widget to a group by splitting its dimensions from the 'splitted'   *
 * rectangle, which has initially the dimensions of the body                  *
 * -------------------------------------------------------------------------- */
void sgSubGroup(sgWidget *group, sgWidget *sub, sgEdge edge, sgAlign align)
{
  SDL_Rect newrect;
  SDL_Rect subrect;
  
  /* Split a rect for the new group at a given position */
  sgSplitRect(&sgGroup(group)->splitted, &newrect, edge, 
              ((edge == SG_EDGE_TOP) || (edge == SG_EDGE_BOTTOM)) ? sub->rect.h : sub->rect.w);
 
  /* Align the widget inside the new rectangle */
  subrect = sub->rect;
  sgAlignRect(&newrect, &subrect, align);
  
  sgSetWidgetRect(sub, subrect);
  
  /* Set the group */
  sub->parent = group;
}

/* -------------------------------------------------------------------------- *
 * Adds a widget to a group by adding its dimensions to the 'aligned'         *
 * rectangle, which has initially zero dimensions and is aligned inside the   *
 * body                                                                       *
 * -------------------------------------------------------------------------- */
void sgAddGroup(sgWidget *group, sgWidget *add, sgEdge edge, sgAlign align)
{
  SDL_Rect newrect;
  SDL_Rect addrect;
  SDL_Rect rect;
  Sint16 dx;
  Sint16 dy;
  sgWidget *widget;
  
  /* Enhance the aligned rectangle at the given position */
  newrect = sgGroup(group)->aligned;
  
  switch(edge)
  {
    case SG_EDGE_TOP:
      newrect.y -= add->rect.h;
    case SG_EDGE_BOTTOM:
      newrect.h += add->rect.h;
      break;
    case SG_EDGE_LEFT:
      newrect.x -= add->rect.w;
    case SG_EDGE_RIGHT:
      newrect.w += add->rect.w;
      break;
    default:
      break;
  }
  
  /* Calculate the added rectangle */
  addrect = newrect;
  
  switch(edge)
  {
    case SG_EDGE_BOTTOM:
      addrect.y += sgGroup(group)->aligned.h;
    case SG_EDGE_TOP:
      addrect.h = add->rect.h;
      break;
    case SG_EDGE_RIGHT:
      addrect.x += sgGroup(group)->aligned.w;
    case SG_EDGE_LEFT:
      addrect.w = add->rect.w;
      break;
    default:
      break;
  }
  
  /* Align the widget inside the new rectangle */
  rect = add->rect;
  sgAlignRect(&addrect, &rect, align);
  
  sgSetWidgetRect(add, rect);
  
  /* Set the group */
  add->parent = group;
  
  sgAlignRect(&sgGroup(group)->body, &newrect, sgGroup(group)->align);
  
  dx = newrect.x - sgGroup(group)->aligned.x;
  dy = newrect.y - sgGroup(group)->aligned.y;
  
  sgForeach(&group->list, widget)
  {
    widget->rect.x += dx;
    widget->rect.y += dy;
  }
  
  sgGroup(group)->aligned = newrect;
}

/* -------------------------------------------------------------------------- *
 * Sets the alignment                                                         *
 * -------------------------------------------------------------------------- */
void sgSetGroupAlign(sgWidget *group, sgAlign align)
{
  sgGroup(group)->align = align;
}
  
/* -------------------------------------------------------------------------- *
 * Sets the image to display inside the widget                                *
 * -------------------------------------------------------------------------- */
int sgSetGroupImage(sgWidget *group, SDL_Surface *surface, sgAlign align)
{
  if(sgGroup(group)->image)
    SDL_FreeSurface(sgGroup(group)->image);

  sgGroup(group)->align = align;
  sgGroup(group)->image = (surface->format->Amask ?
                           SDL_DisplayFormatAlpha(surface) :
                           SDL_DisplayFormat(surface));

  if(sgGroup(group)->image)
  {
    sgSetWidgetStatus(group, SG_REDRAW_CONTENT);
    return 1;
  }
  
  return 0;
}

/* -------------------------------------------------------------------------- *
 * Loads an image from a file                                                 *
 * -------------------------------------------------------------------------- */
int sgLoadGroupImage(sgWidget *group, const char *file, sgAlign align)
{
  SDL_Surface *surface = sgLoadPngFile(file);

  if(surface)
  {
    sgSetGroupImage(group, surface, align);
    SDL_FreeSurface(surface);
    return 0;
  }
  
  return -1;
}

/* -------------------------------------------------------------------------- *
 * Modifies the caption rectangle so the group can be used in a tab widget    *
 * -------------------------------------------------------------------------- */
Uint16 sgSetGroupTab(sgWidget *group, Uint16 offset)
{
  sgGroup(group)->offset = offset;
  sgGroup(group)->width = sgTextWidth(group->font[SG_FONT_BOLD], 
                                      group->caption) + (group->border << 4);
  return sgGroup(group)->width;
}

/* -------------------------------------------------------------------------- *
 * Blits the group, but only the caption unless it has focus                  *
 * -------------------------------------------------------------------------- */
int sgBlitGroupTab(sgWidget *group, SDL_Surface *surface, Sint16 x, Sint16 y)
{
  SDL_Rect drect;
  SDL_Rect srect;
  sgWidget *child;
  
  if(group->parent->type != &sgTabType)
    return sgBlitWidget(group, surface, x, y);
  
  /* Do not blit hidden widgets */
  if(group->status & SG_HIDDEN)
    return 0;
  
  /* Redraw groups faces if needed */
  sgRedrawWidget(group);
  
  drect = group->rect;
  drect.x += x;
  drect.y += y;

  srect = group->rect;
  srect.x = 0;
  srect.y = 0;
  
  if(group->node.prev)
    srect.h = sgGroup(group)->caption.h + group->border + 2;
  
  /* Blit the surfaces to screen */
  if(group->face.frame)
    SDL_BlitSurface(group->face.frame, &srect, surface, &drect);
  
  if(group->face.border)
    SDL_BlitSurface(group->face.border, &srect, surface, &drect);
    
  if(group->face.content)
    SDL_BlitSurface(group->face.content, &srect, surface, &drect);

  /* Recurse into children only when active */
  if(group->node.prev == NULL)
    sgForeach(&group->list, child)
      child->methods.blit(child, surface, drect.x, drect.y);

  /* Reset redraw flags */
  sgClearWidgetStatus(group, SG_REDRAW_NEEDED);
  
  return 1;
}

/** @} */
