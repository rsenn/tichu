/* $Id: dropdown.c,v 1.16 2005/05/23 02:12:17 smoli Exp $
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

/** @weakgroup sgDropdown
 *  @{
 */
 
#include <libsgui/sgui.h>

/** widget type definition and its callbacks */
sgWidgetType sgDropdownType = 
{
  .name = "sgDropdown",
  .size = sizeof(sgDropdown),
  .methods =
  {
    .recalc = sgRecalcDropdown,
    .redraw = sgRedrawDropdown,
    .handler = sgHandleDropdownEvent,
    .blit = sgBlitWidget,
    .delete = sgClearDropdown
  }
};

/* Creates a new dropdown widget by splitting another
 * -------------------------------------------------------------------------- */
sgWidget *sgNewDropdownSplitted(sgWidget *based, sgEdge edge, int pixels)
{
  sgWidget *dropdown;
  SDL_Rect  newrect;
  
  sgSplitWidget(based, &newrect, edge, pixels);
  dropdown = sgNewDropdown(based->parent, newrect.x, newrect.y, 
                           newrect.w, newrect.h);
  
/*  if(edge == SG_EDGE_LEFT || edge == SG_EDGE_TOP)
  {
    sgDeleteList(&based->parent->list, &dropdown->node);
    sgAddListBefore(&based->parent->list, &dropdown->node, &based->node, dropdown);
  }*/
  
  return dropdown;
}

/* -------------------------------------------------------------------------- */
sgWidget *sgNewDropdownGrouped(sgWidget *group, 
                              sgEdge edge, sgAlign align, Uint16 w, Uint16 h)
{
  sgWidget *dropdown = sgNewDropdown(group, 0, 0, w, h);
  
  sgSubGroup(group, dropdown, edge, align);
  
  return dropdown;
}

/* -------------------------------------------------------------------------- */
sgWidget *sgNewDropdown(sgWidget *parent, Sint16 x, Sint16 y, Uint16 w, Uint16 h)
{
  sgWidget *dropdown;
  
  if((dropdown = sgNewWidget(&sgDropdownType, parent, x, y, w, h, NULL)))
    sgUndeployDropdown(dropdown);
  
  return dropdown;
}

/* -------------------------------------------------------------------------- */
void sgFreeDropdown(sgWidget *dropdown)
{
  sgDropdownItem *item;
  sgDropdownItem *next;

  sgForeachSafe(&sgDropdown(dropdown)->items, item, next)
  {
    if(item->surface)
      SDL_FreeSurface(item->surface);

    sgDeleteList(&sgDropdown(dropdown)->items, &item->node);
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void sgRecalcDropdown(sgWidget *dropdown)
{
  sgFont *font;
  Uint16 minh, minw;
  SDL_Rect rect;
  
  /* Get the font we're drawing to the dropdown caption */
  font = dropdown->font[SG_FONT_FIXED];
  
  /* Calculate minimal dimensions based on font */
  minw = sgTextWidth(font, dropdown->caption) + (dropdown->border << 1) + 2;
  minh = sgFontHeight(font) + 3 + (dropdown->border << 1) + 2;
  
  /* Apply them */
  if(dropdown->rect.h < minh) dropdown->rect.h = minh;
  if(dropdown->rect.w < minw) dropdown->rect.w = minw;
  
  /* Calc rectangle for the outer dropdown frame */
  sgDropdown(dropdown)->outer.x = 0;
  sgDropdown(dropdown)->outer.y = 0;
  sgDropdown(dropdown)->outer.w = dropdown->rect.w;
  sgDropdown(dropdown)->outer.h = dropdown->rect.h;
  
  sgSubBorder(&sgDropdown(dropdown)->outer, dropdown->border);
  
  /* Caption rect must fit the font and a pict (16x16) + frame */
  minh = (sgFontHeight(font) > 16 ? sgFontHeight(font) : 16) + 8;
  
  rect = sgDropdown(dropdown)->outer;
  
  sgSubBorder(&rect, 2);
  
  sgDropdown(dropdown)->list = rect;
  
  /* Split off the scrollbar rectangle */
  sgSplitRect(&rect, &sgDropdown(dropdown)->sbar_space, SG_EDGE_RIGHT, 20);

  rect.w += 1;
  
  /* Split off the button rectangle */
  sgSplitRect(&sgDropdown(dropdown)->sbar_space,
              &sgDropdown(dropdown)->button, SG_EDGE_TOP, minh - 4);
  
  sgDropdown(dropdown)->sbar_space.y--;
  sgDropdown(dropdown)->sbar_space.h++;
  
  /* setup scrollbar rectangles */
  sgSplitTop(&sgDropdown(dropdown)->sbar_space, &sgDropdown(dropdown)->sbar_up, 20);
  sgSplitBottom(&sgDropdown(dropdown)->sbar_space, &sgDropdown(dropdown)->sbar_down, 20);
  
  
  /* Split off the caption rectangle */
  sgSplitRect(&sgDropdown(dropdown)->list, &sgDropdown(dropdown)->caption, SG_EDGE_TOP, minh - 4);

  sgDropdown(dropdown)->caption.w -= minh - 4;
  
/*  rect.y--;  
  rect.h++;
  rect.h -= 2;
  rect.w += 2;
  sgDropdown(dropdown)->list = rect;*/
  
  
  /* Calc rectangle for the dropdown body frame */
  sgDropdown(dropdown)->body = sgDropdown(dropdown)->list;
  sgSubBorder(&sgDropdown(dropdown)->body, 2);
  
  sgDropdown(dropdown)->sbar_space.y--;
  sgDropdown(dropdown)->sbar_space.h += 2;
  sgDropdown(dropdown)->sbar_bar = sgDropdown(dropdown)->sbar_space;
  
  sgPadRect(&sgDropdown(dropdown)->body, SG_EDGE_LEFT|SG_EDGE_RIGHT, 4);
  sgPadRect(&sgDropdown(dropdown)->body, SG_EDGE_TOP|SG_EDGE_BOTTOM, 2);

  /* item dimensions */
  sgDropdown(dropdown)->item = sgDropdown(dropdown)->body;
  sgDropdown(dropdown)->item.h = sgFontHeight(dropdown->font[SG_FONT_FIXED]) + 4;
  
  /* max. items displayed at a time */
  sgDropdown(dropdown)->rows = (dropdown->rect.h - sgDropdown(dropdown)->caption.h) / sgDropdown(dropdown)->item.h;
  
  sgDropdown(dropdown)->cols = (sgDropdown(dropdown)->body.w - 8) / 
    sgTextWidth(dropdown->font[SG_FONT_FIXED], " ");

  if(sgDropdown(dropdown)->items.size > sgDropdown(dropdown)->rows)
  {
    sgDropdown(dropdown)->body.w -= 20;
    sgDropdown(dropdown)->list.w -= 20;
    sgDropdown(dropdown)->item.w -= 20;
  }
  
  sgScrollDropdown(dropdown, sgDropdown(dropdown)->scroll, 1, 1);
  
  sgToggleDropdown(dropdown);
  sgToggleDropdown(dropdown);
  
  sgSetWidgetStatus(dropdown, SG_REDRAW_NEEDED);
  
}

/* -------------------------------------------------------------------------- *
 * Redraws dropdown look                                                      *
 * -------------------------------------------------------------------------- */
void sgRedrawDropdown(sgWidget *dropdown)
{
  sgDropdownItem *item;
  SDL_Rect itemrect;
  SDL_Rect tmprect;
  int i;
  int iselect;
  
  /* Redraw borders */
  if(sgRedrawWidgetBorder(dropdown))
  {
    tmprect.x = 0;
    tmprect.y = 0;
    tmprect.w = dropdown->rect.w;
    tmprect.h = dropdown->rect.h;
    
    if(sgDropdown(dropdown)->down == 0 || sgDropdown(dropdown)->items.size <= sgDropdown(dropdown)->rows)
      tmprect.h = sgDropdown(dropdown)->caption.h + (dropdown->border << 1) + 4;
    
    
    if(sgDropdown(dropdown)->down && sgDropdown(dropdown)->items.size <= sgDropdown(dropdown)->rows)
      tmprect.h += sgDropdown(dropdown)->items.size * sgDropdown(dropdown)->item.h;
      
    if(sgHasWidgetFocus(dropdown))
      sgDrawFrame(dropdown->face.border, &tmprect, SG_DRAW_FILL);

    sgSubBorder(&tmprect, dropdown->border);    
    
    
    SDL_FillRect(dropdown->face.border, &tmprect, 0);

    if(!sgHasWidgetFocus(dropdown))
      sgDrawFrame(dropdown->face.border, &sgDropdown(dropdown)->caption,
                  SG_DRAW_FILL|SG_DRAW_INVERSE);
    
    
/*    if(sgDropdown(dropdown)->down)
      SDL_FillRect(dropdown->face.border, &sgDropdown(dropdown)->list, 0);*/
    
    if(sgDropdown(dropdown)->down)
    {
      /* Draw dropdown items */
      item = sgDropdownGetItemByNum(dropdown, sgDropdown(dropdown)->scroll);
      itemrect = sgDropdown(dropdown)->body;
      
    /* Clear textarea + some extra pixels below */
    itemrect.x = sgDropdown(dropdown)->list.x;
    itemrect.w = sgDropdown(dropdown)->list.w;
    
    tmprect = itemrect;
    tmprect.h += tmprect.y - sgDropdown(dropdown)->list.y;
    tmprect.y = sgDropdown(dropdown)->list.y;
    
    /* Draw until beyond textrect */
    itemrect.h = sgDropdown(dropdown)->item.h;
    itemrect.y -= 5;
    
    for(i = 0; i < sgDropdown(dropdown)->rows && item; item = (sgDropdownItem *)item->node.next, i++)
    {
      if(sgDropdown(dropdown)->focus == i)
       {
         SDL_Rect tmprect = itemrect;
         
         tmprect.h++;

         sgDrawFrame(dropdown->face.border, &tmprect,
                     (sgDropdown(dropdown)->select == i ? SG_DRAW_INVERSE : 0)|SG_DRAW_FILL);
       }
      
      itemrect.y += itemrect.h;
    }
    }
    
  }
  
  /* Redraw frames */
  if(sgRedrawWidgetFrame(dropdown))
  {
    /* Draw outer frame */
    tmprect = sgDropdown(dropdown)->outer;
    
    if(sgDropdown(dropdown)->down == 0)
    {
      tmprect.h = sgDropdown(dropdown)->caption.h + 4;
    }
    else
    {
      /* Resize body when all the items fit */
      if(sgDropdown(dropdown)->items.size <= sgDropdown(dropdown)->rows)
        tmprect.h = sgDropdown(dropdown)->caption.h + 4 + sgDropdown(dropdown)->item.h * sgDropdown(dropdown)->items.size;
    }
    
    
    sgDrawFrame(dropdown->face.frame, &tmprect, SG_DRAW_NORMAL|SG_DRAW_FILL);
    
    
    /* Draw list frame  */
    if(sgDropdown(dropdown)->down)
    {
        SDL_Rect list;

        list = sgDropdown(dropdown)->list;
        
        
        if(sgDropdown(dropdown)->items.size <= sgDropdown(dropdown)->rows)
          list.h = sgDropdown(dropdown)->item.h * sgDropdown(dropdown)->items.size;
        
      iselect = sgDropdown(dropdown)->select - sgDropdown(dropdown)->scroll;
      
      tmprect = list;
      list.y--;
      list.h++;
      
      sgDrawFrame(dropdown->face.frame, &list,
                  SG_DRAW_INVERSE|SG_DRAW_FILL);
      
      if(iselect >= 0 || iselect <= sgDropdown(dropdown)->rows)
      {
        SDL_Rect upper;
        SDL_Rect lower;
        
        /* Draw upper list frame */
        if(iselect > 0)
        {
          upper = list;
          upper.y--;
          upper.h = sgDropdown(dropdown)->item.h * iselect + (sgDropdown(dropdown)->body.y - upper.y) - 4;
        
          sgDrawFrame(dropdown->face.frame, &upper, SG_DRAW_FILL);
        }
        
      
        /* Draw lower list frame */
        if(sgDropdown(dropdown)->select - sgDropdown(dropdown)->scroll < sgDropdown(dropdown)->rows)
        {
          Uint16 offset;
          
          lower = list;
          
          /* Resize body when all the items fit */
/*          if(sgDropdown(dropdown)->items.size <= sgDropdown(dropdown)->rows)
        tmprect.h = sgDropdown(dropdown)->caption.h + 4 + sgDropdown(dropdown)->item.h * sgDropdown(dropdown)->items.size;*/

          offset = sgDropdown(dropdown)->item.h * (iselect + 1) + (sgDropdown(dropdown)->body.y - lower.y) - 5;
          
          lower.y += offset;
          lower.h -= offset;

          sgDrawFrame(dropdown->face.frame, &lower, SG_DRAW_FILL);
        }
      }
      
      if(sgDropdown(dropdown)->items.size > sgDropdown(dropdown)->rows)
      {
        /* Draw up button/arrow */
        sgDrawFrame(dropdown->face.frame, &sgDropdown(dropdown)->sbar_up,
                    sgDropdown(dropdown)->spushed0|SG_DRAW_FILL);
      
        sgPutPict(dropdown->face.frame, &sgDropdown(dropdown)->sbar_up,
                  &sgArrowUp, sgDropdown(dropdown)->spushed0|SG_DRAW_FILL);
        
        /* Draw down button/arrow */
        sgDrawFrame(dropdown->face.frame, &sgDropdown(dropdown)->sbar_down,
                    sgDropdown(dropdown)->spushed1|SG_DRAW_FILL);
      
        sgPutPict(dropdown->face.frame, &sgDropdown(dropdown)->sbar_down,
                  &sgArrowDown, sgDropdown(dropdown)->spushed1);
        
        /* Draw scrollbar space frame */
        sgDrawFrame(dropdown->face.frame, &sgDropdown(dropdown)->sbar_space, 
                    SG_DRAW_INVERSE|SG_DRAW_FILL);
        
        /* Draw scrollbar bar frame */
        sgDrawFrame(dropdown->face.frame, &sgDropdown(dropdown)->sbar_bar,
                    sgDropdown(dropdown)->spushed|SG_DRAW_FILL);
      }
    }
    
    /* Draw caption frame  */
    sgDrawFrame(dropdown->face.frame, &sgDropdown(dropdown)->caption,
                SG_DRAW_INVERSE|SG_DRAW_FILL);
    
    /* Draw dropdown button */
    sgDrawFrame(dropdown->face.frame, &sgDropdown(dropdown)->button,
                sgDropdown(dropdown)->pushed|SG_DRAW_FILL);
    
    sgPutPict(dropdown->face.frame, &sgDropdown(dropdown)->button,
              ((sgDropdown(dropdown)->down) ? &sgArrowUndrop : &sgArrowDrop),
              sgDropdown(dropdown)->pushed);

  }
  
  /* Redraw contents */
  if(sgRedrawWidgetContent(dropdown))
  {
    SDL_Rect list = sgDropdown(dropdown)->list;
        
    if(sgDropdown(dropdown)->items.size <= sgDropdown(dropdown)->rows)
      list.h = sgDropdown(dropdown)->item.h * sgDropdown(dropdown)->items.size;
    
    /* Draw dropdown caption */
    tmprect = sgDropdown(dropdown)->caption;
    
    sgPadRect(&tmprect, SG_EDGE_LEFT|SG_EDGE_RIGHT, 4);
    
    tmprect.y++;
    
    sgDrawTextOutline(dropdown->font[SG_FONT_FIXED],
                      dropdown->face.content, &tmprect,
                      SG_ALIGN_LEFT|SG_ALIGN_MIDDLE, dropdown->caption);
    
    if(sgDropdown(dropdown)->down)
    {
      /* Draw dropdown items */
      item = sgDropdownGetItemByNum(dropdown, sgDropdown(dropdown)->scroll);
      itemrect = sgDropdown(dropdown)->body;
      
      /* Clear textarea + some extra pixels below */
      itemrect.y -= 3;
      itemrect.x -= 2;
      itemrect.w += 4;
      itemrect.h = dropdown->rect.h - (itemrect.y - dropdown->rect.y);
      
      SDL_FillRect(dropdown->face.content, &itemrect, 0);
      
      /* Draw until beyond textrect */
      itemrect.h = sgDropdown(dropdown)->item.h;
      
      for(i = 0; i < sgDropdown(dropdown)->rows && item; item = (sgDropdownItem *)item->node.next, i++)
      {
        /* Draw the text */
        sgDrawTextOutline(dropdown->font[SG_FONT_FIXED],
                          dropdown->face.content, &itemrect, 
                          SG_ALIGN_LEFT|SG_ALIGN_MIDDLE, item->caption);
        
        itemrect.y += itemrect.h;
      }
      
      /* Cut overlapping font of last displayed item */
      itemrect = list;
      itemrect.y += itemrect.h - 2;
      itemrect.h = dropdown->rect.h - itemrect.y;
    
      SDL_FillRect(dropdown->face.content, &itemrect, 0);
    }
    
  }
  
/*  tmprect = dropdown->area;
  tmprect.x = 0;
  tmprect.y = 0;
  SDL_FillRect(dropdown->face.content, &tmprect, RMASK|AMASK);*/
}
  
/* -------------------------------------------------------------------------- *
 * Handles dropdown events                                                     *
 * -------------------------------------------------------------------------- */
int sgHandleDropdownEvent(sgWidget *dropdown, SDL_Event *event) 
{   
  /* Dropdown must have focus to generate events */
  if(sgHasWidgetFocus(dropdown))
  {
    if(sgDropdown(dropdown)->down)
    {
      /* Mousewheel down */
      if(sgEventButton(event, SDL_BUTTON_WHEELUP, SG_PRESSED))
      {
        return sgScrollDropdown(dropdown, sgDropdown(dropdown)->scroll - 1, 0, 1);
      } 
      /* Mousewheel up */
      else if(sgEventButton(event, SDL_BUTTON_WHEELDOWN, SG_PRESSED))
      {
        return sgScrollDropdown(dropdown, sgDropdown(dropdown)->scroll + 1, 0, 1);
      }  
    }
    
    if(sgEventButton(event, SDL_BUTTON_LEFT, SG_PRESSED))
    {
        /* Check for click inside the content rectangle */
        if(sgDropdown(dropdown)->down && sgMatchRect(&sgDropdown(dropdown)->list, event->button.x, event->button.y))
        {
          int select = (event->button.y - sgDropdown(dropdown)->body.y) / sgDropdown(dropdown)->item.h;
          
          /* No items, so none can be select */
          if(sgDropdown(dropdown)->items.size == 0) 
          {
            sgDropdown(dropdown)->select = -1;
            sgReportWidgetEvent(dropdown, SG_SEL_CHANGE);
            
            return 0;
          }
          
          /* Clicked beyond last item? */
          if(select + (int)sgDropdown(dropdown)->scroll >= (int)sgDropdown(dropdown)->items.size) 
          {
            sgSelectDropdownItem(dropdown, 
                                 sgDropdown(dropdown)->items.size - 
                                 sgDropdown(dropdown)->scroll - 1);
            
            sgSetWidgetStatus(dropdown, SG_REDRAW_FRAME);
            
            return 1;
          }
          
          /* Has select item changed? */
          if(select + sgDropdown(dropdown)->scroll != sgDropdown(dropdown)->select) 
          {
            sgReportWidgetEvent(dropdown, SG_SEL_CHANGE);
            
            sgSelectDropdownItem(dropdown, select + sgDropdown(dropdown)->scroll);
            
            sgSetWidgetStatus(dropdown, SG_REDRAW_FRAME);
            
            return 1;
          }
          else 
          {
            /* Selected item has been clicked second time */
            sgReportWidgetEvent(dropdown, SG_SEL_CLICK);
            
            return 1;
          }
        }
        
        
        /* Check for click on the drop arrow */
        if((sgDropdown(dropdown)->pushed & SG_DRAW_INVERSE) == 0 &&
           sgMatchRect(&sgDropdown(dropdown)->button, event->button.x, event->button.y))
        {
          sgDropdown(dropdown)->pushed |= SG_DRAW_INVERSE|SG_DRAW_HIGH;
          sgSetWidgetStatus(dropdown, SG_REDRAW_FRAME);
          return 1;
        }
        
        /* Check for click on the up arrow */
        if((sgDropdown(dropdown)->spushed0 & SG_DRAW_INVERSE) == 0 &&
           sgMatchRect(&sgDropdown(dropdown)->sbar_up, event->button.x, event->button.y))
        {
          sgDropdown(dropdown)->spushed0 |= SG_DRAW_INVERSE|SG_DRAW_HIGH;
          sgSetWidgetStatus(dropdown, SG_REDRAW_FRAME);
          return 1;
        }
        
        /* Check for click on the down arrow */
        if((sgDropdown(dropdown)->spushed1 & SG_DRAW_INVERSE) == 0 &&
           sgMatchRect(&sgDropdown(dropdown)->sbar_down, event->button.x, event->button.y))
        {
          sgDropdown(dropdown)->spushed1 |= SG_DRAW_INVERSE|SG_DRAW_HIGH;
          sgSetWidgetStatus(dropdown, SG_REDRAW_FRAME);
          
          return 1;
        }
        
        /* If we're not scrolling and there are more items that 
           can be displayed once we're entering scrolling mode */
        if((sgDropdown(dropdown)->spushed & SG_DRAW_INVFILL) == 0 &&
           sgDropdown(dropdown)->items.size > sgDropdown(dropdown)->rows && 
           sgMatchRect(&sgDropdown(dropdown)->sbar_bar, event->button.x, event->button.y))
        {
          sgDropdown(dropdown)->clicky = event->button.y;
          sgDropdown(dropdown)->spushed |= SG_DRAW_INVFILL|SG_DRAW_HIGH;
          sgSetWidgetStatus(dropdown, SG_REDRAW_FRAME);

          return 1;
        }
    }    
    
    /* Mouse button was released while widget focused */
    if(sgEventButton(event, SDL_BUTTON_LEFT, SG_RELEASED))
    {
      /* Drop arrow was pushed */
      if(sgDropdown(dropdown)->pushed & SG_DRAW_INVERSE)
      {
        sgDropdown(dropdown)->pushed &= ~SG_DRAW_INVERSE;
        
        sgToggleDropdown(dropdown);
        return 1;
      }
      
      if(sgDropdown(dropdown)->down)
      {
        /* Up arrow was pushed */
        if(sgDropdown(dropdown)->spushed0 & SG_DRAW_INVERSE) 
        {
          sgDropdown(dropdown)->spushed0 &= ~SG_DRAW_INVERSE;
          sgSetWidgetStatus(dropdown, SG_REDRAW_FRAME);
          
          if(sgDropdown(dropdown)->scroll > 0) 
            return sgScrollDropdown(dropdown, sgDropdown(dropdown)->scroll - 1, 0, 1);
          
          return 1;
        }
        
        /* Down arrow was pushed */
        if(sgDropdown(dropdown)->spushed1 & SG_DRAW_INVERSE) 
        {
          sgDropdown(dropdown)->spushed1 &= ~SG_DRAW_INVERSE;
          sgSetWidgetStatus(dropdown, SG_REDRAW_FRAME);
          
          if(sgDropdown(dropdown)->scroll < sgDropdown(dropdown)->items.size - sgDropdown(dropdown)->rows) 
            return sgScrollDropdown(dropdown, sgDropdown(dropdown)->scroll + 1, 1, 1);
          
          return 1;
        }
      
        /* We we're scrolling */
        if(sgDropdown(dropdown)->spushed & SG_DRAW_INVFILL)
        {
          sgDropdown(dropdown)->spushed &= ~SG_DRAW_INVFILL;
          
          sgScrollDropdown(dropdown, sgDropdown(dropdown)->scroll, 1, 1);
          
          return 1;
        }
      }
    }
    
    /* Mouse has moved */
    if(sgDropdown(dropdown)->down && event->type == SDL_MOUSEMOTION)
    {
      /* We're scrolling */
      if(sgDropdown(dropdown)->spushed & SG_DRAW_INVFILL)
      {      
        int ydiff = event->motion.y - sgDropdown(dropdown)->clicky + (sgDropdown(dropdown)->sbar_bar.y - sgDropdown(dropdown)->sbar_space.y);
        int yrange = sgDropdown(dropdown)->sbar_space.h - sgDropdown(dropdown)->sbar_bar.h;
        int irange = sgDropdown(dropdown)->items.size - sgDropdown(dropdown)->rows + 1;
        
        sgDropdown(dropdown)->clicky = event->motion.y;
        
        if(irange)
        {
          ydiff = ydiff < 0 ? 0 : (ydiff >= yrange ? yrange - 1 : ydiff);
          
          sgDropdown(dropdown)->sbar_bar.y = sgDropdown(dropdown)->sbar_space.y + ydiff;
        
          sgScrollDropdown(dropdown, ydiff * irange / yrange, 0, 0);

          sgSetWidgetStatus(dropdown, SG_REDRAW_FRAME);
        }
        
        return 1;
      }
      
      /* Mouse moving inside textrect */
      if(sgMatchRect(&sgDropdown(dropdown)->body, event->motion.x, event->motion.y))
      {
        int focus = (event->motion.y - sgDropdown(dropdown)->body.y) / sgDropdown(dropdown)->item.h;
        
        if(focus != sgDropdown(dropdown)->focus) 
        {
          sgDropdown(dropdown)->focus = focus;
          sgSetWidgetStatus(dropdown, SG_REDRAW_BORDER);

          return 1;
        }
      }
    }
    else if(event->type == SDL_KEYDOWN && 
            event->key.keysym.sym == SDLK_LEFT)
    {
      if(sgDropdown(dropdown)->focus == -1) 
      {
        sgDropdown(dropdown)->focus = 0;
        sgSetWidgetStatus(dropdown, SG_REDRAW_BORDER);

        return 1;
      }
      
      if(sgDropdown(dropdown)->focus > 0) 
      {
        sgDropdown(dropdown)->focus--;
        sgSetWidgetStatus(dropdown, SG_REDRAW_BORDER);

        return 1;
      }
      
      if(sgDropdown(dropdown)->focus == 0) 
      {
        if(sgDropdown(dropdown)->scroll) 
        {
          sgScrollDropdown(dropdown, sgDropdown(dropdown)->scroll - 1, 0, 1);

          return 1;
        }
      }
    }    
    else if(event->type == SDL_KEYDOWN && 
            event->key.keysym.sym == SDLK_RIGHT)
    {
      if(sgDropdown(dropdown)->focus == -1) 
      {
        sgDropdown(dropdown)->focus = 0;
        sgSetWidgetStatus(dropdown, SG_REDRAW_BORDER);
        return 1;
      }
      
      if(sgDropdown(dropdown)->focus < (int)sgDropdown(dropdown)->rows - 1) 
      {
        sgDropdown(dropdown)->focus++;
        sgSetWidgetStatus(dropdown, SG_REDRAW_BORDER);
        return 1;
      }
      
      if(sgDropdown(dropdown)->focus >= (int)sgDropdown(dropdown)->rows - 1) 
      {
        if(sgDropdown(dropdown)->scroll <= sgDropdown(dropdown)->items.size - sgDropdown(dropdown)->rows - 1) 
        {
          sgScrollDropdown(dropdown, sgDropdown(dropdown)->scroll + 1, 0, 1);
          
          return 1;
        }
      }
    } 
    else if(event->type == SDL_KEYDOWN &&
            event->key.keysym.sym == SDLK_RETURN)
    {
      int select = sgDropdown(dropdown)->scroll + sgDropdown(dropdown)->focus;
      
      if(sgDropdown(dropdown)->select != select) 
      {
        sgReportWidgetEvent(dropdown, SG_SEL_CHANGE);
        sgDropdown(dropdown)->select = select;
        sgSetWidgetStatus(dropdown, SG_REDRAW_BORDER|SG_REDRAW_FRAME);
        
        return 1;
      }
    }
  }

  /* Check for mouse motion */
  if(event->type == SDL_MOUSEMOTION)
  {
    sgHandleWidgetHilite(dropdown, sgDropdown(dropdown)->button, 
                         &sgDropdown(dropdown)->pushed,
                         event->motion.x, event->motion.y);
    
    sgHandleWidgetHilite(dropdown, sgDropdown(dropdown)->sbar_bar,
                         &sgDropdown(dropdown)->spushed,
                         event->motion.x, event->motion.y);
    
    sgHandleWidgetHilite(dropdown, sgDropdown(dropdown)->sbar_up,
                         &sgDropdown(dropdown)->spushed0,
                         event->motion.x, event->motion.y);
    
    sgHandleWidgetHilite(dropdown, sgDropdown(dropdown)->sbar_down,
                         &sgDropdown(dropdown)->spushed1,
                         event->motion.x, event->motion.y);    
  }
  
  if((dropdown->status & SG_FOCUS) == 0)
  {
    if((sgDropdown(dropdown)->spushed0 & SG_DRAW_INVERSE) || 
       (sgDropdown(dropdown)->spushed1 & SG_DRAW_INVERSE))
    {
      sgDropdown(dropdown)->spushed0 &= ~SG_DRAW_INVERSE;
      sgDropdown(dropdown)->spushed1 &= ~SG_DRAW_INVERSE;
      sgSetWidgetStatus(dropdown, SG_REDRAW_FRAME);
      
      return 1;
    }    
    sgDropdown(dropdown)->focus = -1;
    return 0;
  }
  
  if(sgDropdown(dropdown)->focus == -1)
    sgDropdown(dropdown)->focus = 0;
  
  return 0;
}


/* -------------------------------------------------------------------------- *
 * Adds a new Item to a Dropdown                                               *
 * -------------------------------------------------------------------------- */
sgDropdownItem *sgAddDropdownItem(sgWidget *dropdown, const char *caption, 
                                  void *value)
{
  sgDropdownItem *item;
  
  item = sgInsertDropdownItem(dropdown, sgDropdown(dropdown)->items.size, 
                              caption, value);

  return item;
}

/* -------------------------------------------------------------------------- *
 * Inserts a new Item into a Dropdown                                          *
 * -------------------------------------------------------------------------- */
sgDropdownItem *sgInsertDropdownItem(sgWidget *dropdown, int pos, 
                                     const char *caption, void *value)
{
  sgDropdownItem *item;
  
  item = malloc(sizeof(sgDropdownItem));
  
  if(item == NULL)
    return NULL;
  
  sgAddList(&sgDropdown(dropdown)->items, &item->node, item);
  sgStringCopy(item->caption, caption);

  item->value = value;
  
  sgReportWidgetEvent(dropdown, SG_SEL_CHANGE);

  if(sgDropdown(dropdown)->select >= (int)sgDropdown(dropdown)->items.size) 
    sgDropdown(dropdown)->select = sgDropdown(dropdown)->items.size - 1;
  
  if(pos <= sgDropdown(dropdown)->scroll)
    sgScrollDropdown(dropdown, sgDropdown(dropdown)->scroll + 1, 1, 1);
  else
    sgScrollDropdown(dropdown, sgDropdown(dropdown)->scroll, 1, 1);
 
  sgRecalcDropdown(dropdown);
  
  if(sgDropdown(dropdown)->selected == NULL)
  {
    sgDropdown(dropdown)->selected = item;
    sgStringCopy(dropdown->caption, item->caption);
  }
  
  return item;
}

/* -------------------------------------------------------------------------- */
void sgDeleteDropdownItem(sgWidget *dropdown, sgDropdownItem *item) 
{
  if(item == NULL)
    return;
  
  if(item->surface)
    SDL_FreeSurface(item->surface);
  
  sgDeleteList(&sgDropdown(dropdown)->items, &item->node);
  free(item);
  
  if(sgDropdown(dropdown)->select >= (int)sgDropdown(dropdown)->items.size) 
  {
    sgDropdown(dropdown)->select = sgDropdown(dropdown)->items.size - 1;
    sgReportWidgetEvent(dropdown, SG_SEL_CHANGE);
  }
  
  sgSetWidgetStatus(dropdown, SG_REDRAW_CONTENT|SG_REDRAW_FRAME);
}

/* -------------------------------------------------------------------------- */
void sgClearDropdown(sgWidget *dropdown)
{
  while(sgDropdown(dropdown)->items.head)
    sgDeleteDropdownItem(dropdown, (sgDropdownItem *)sgDropdown(dropdown)->items.head);
}
  
/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
sgDropdownItem *sgDropdownGetItemByNum(sgWidget *dropdown, int num) 
{  
  return (sgDropdownItem *)sgIndexList(&sgDropdown(dropdown)->items, num);
}

/* -------------------------------------------------------------------------- */
sgDropdownItem *sgSelectedDropdownItem(sgWidget *dropdown)
{
  return sgDropdownGetItemByNum(dropdown, sgDropdown(dropdown)->select);
}

/* -------------------------------------------------------------------------- */
void sgSelectDropdownItem(sgWidget *dropdown, int i) 
{  
  if(i >= (int)sgDropdown(dropdown)->items.size)
    i = sgDropdown(dropdown)->items.size - 1;
  if(i < 0)
    i = 0;
  
  if(i != sgDropdown(dropdown)->select) 
  {
    sgDropdown(dropdown)->select = i;
    
    sgDropdown(dropdown)->selected = 
      (sgDropdownItem *)sgIndexList(&sgDropdown(dropdown)->items, i);
    
    if(sgDropdown(dropdown)->selected)
      sgStringCopy(dropdown->caption, sgDropdown(dropdown)->selected->caption);
    
    sgReportWidgetEvent(dropdown, SG_SEL_CHANGE);
    
    sgSetWidgetStatus(dropdown, SG_REDRAW_FRAME|SG_REDRAW_BORDER|SG_REDRAW_CONTENT);
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int sgGetDropdownItemByCaption(sgWidget *dropdown, const char *caption)
{
  sgDropdownItem *item;
  int i = 0;
  
  sgForeach(&sgDropdown(dropdown)->items, item)
  {
    if(!strcmp(item->caption, caption))
      return i;
    
    i++;
  }
  
  return -1;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
int sgGetDropdownItemByValue(sgWidget *dropdown, void *value)
{
  sgDropdownItem *item;
  int i = 0;
  
  sgForeach(&sgDropdown(dropdown)->items, item)
  {
    if(item->value == value)
      return i;
    
    i++;
  }
  
  return -1;
}

/* -------------------------------------------------------------------------- *
 * Deploys a dropdown                                                         *
 * -------------------------------------------------------------------------- */
void sgDeployDropdown(sgWidget *dropdown)
{
  dropdown->area.h = dropdown->rect.h;
  sgDropdown(dropdown)->down = 1;
  sgDropdown(dropdown)->focus = -1;
  sgSetWidgetStatus(dropdown, SG_REDRAW_NEEDED|SG_GRAB);
}

/* -------------------------------------------------------------------------- *
 * Undeploys a dropdown                                                       *
 * -------------------------------------------------------------------------- */
void sgUndeployDropdown(sgWidget *dropdown)
{
  dropdown->area.h = sgDropdown(dropdown)->caption.y + sgDropdown(dropdown)->caption.h;
  sgDropdown(dropdown)->down = 0;
  sgDropdown(dropdown)->focus = -1;
  sgClearWidgetStatus(dropdown, SG_GRAB);
  sgSetWidgetStatus(dropdown, SG_REDRAW_NEEDED);
}

/* -------------------------------------------------------------------------- *
 * (Un)deploys a dropdown                                                     *
 * -------------------------------------------------------------------------- */
void sgToggleDropdown(sgWidget *dropdown)
{
  if(sgDropdown(dropdown)->down)
    sgUndeployDropdown(dropdown);
  else
    sgDeployDropdown(dropdown);
}

/* -------------------------------------------------------------------------- *
 * Scrolls a dropdown                                                          *
 * -------------------------------------------------------------------------- */
int sgScrollDropdown(sgWidget *dropdown, int pos, int force, int setrect)
{  
  SDL_Rect rect;
  int range;
  
  /* Truncate position */
  pos = (pos > 0 ? (pos < sgDropdown(dropdown)->items.size - sgDropdown(dropdown)->rows ? 
                    pos : sgDropdown(dropdown)->items.size - sgDropdown(dropdown)->rows) : 0);
  
  /* If scrollbar is useable */
  if(sgDropdown(dropdown)->items.size > sgDropdown(dropdown)->rows)
  {
    rect = sgDropdown(dropdown)->sbar_space;
    
    /* Calculate scrollbar height */
    rect.h = (rect.h * sgDropdown(dropdown)->rows) / sgDropdown(dropdown)->items.size;
    
    /* Range to scroll */
    range = sgDropdown(dropdown)->sbar_space.h - rect.h;
    
    /* Calculate scrollbar position */
    rect.y += (pos * range / (sgDropdown(dropdown)->items.size - sgDropdown(dropdown)->rows));
  } 
  /* Too few items for scrolling */
  else 
  {
    /* Disable scrollbar */
    pos = 0;
    rect = sgDropdown(dropdown)->sbar_space;
  }
  
  /* Scrolling position changed or rescroll was forced */
  if(pos != sgDropdown(dropdown)->scroll || force)
  {
    /* Position changed, generate an event */
    if(pos != sgDropdown(dropdown)->scroll)
      sgReportWidgetEvent(dropdown, SG_SCROLL);
    
    /* Set new position */
    sgDropdown(dropdown)->scroll = pos;
    
    /* Should we set a new scrollbar rectangle? */
    if(setrect)
      sgDropdown(dropdown)->sbar_bar = rect;
    
    /* Redraw scrollbar stuff */
    sgSetWidgetStatus(dropdown, SG_REDRAW_FRAME|SG_REDRAW_CONTENT);
    
    return 1;
  }
  
  return 0;
}

/** @} */
