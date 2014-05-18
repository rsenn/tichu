/* $Id: listbox.c,v 1.38 2005/05/23 02:12:17 smoli Exp $
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

/** @weakgroup sgListbox
 *  @{
 */
 
#include <libsgui/sgui.h>

/** widget type definition and its callbacks */
sgWidgetType sgListboxType = 
{
  .name = "sgListbox",
  .size = sizeof(sgListbox),
  .methods =
  {
    .recalc = sgRecalcListbox,
    .redraw = sgRedrawListbox,
    .handler = sgHandleListboxEvent,
    .blit = sgBlitWidget,
    .delete = sgClearListbox
  }
};

/* -------------------------------------------------------------------------- */
sgWidget *sgNewListboxFull(sgWidget *parent, int symbols, const char *caption)
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
  
  return sgNewListbox(parent, rect.x, rect.y, rect.w, rect.h, symbols, caption);
}

/* -------------------------------------------------------------------------- */
sgWidget *sgNewListboxGrouped(sgWidget *group, 
                              sgEdge edge, sgAlign align, Uint16 w, Uint16 h,
                              int symbols,
                              const char *caption)
{
  sgWidget *listbox = sgNewListbox(group, 0, 0, w, h, symbols, caption);
  
  sgSubGroup(group, listbox, edge, align);
  
  return listbox;
}

/* -------------------------------------------------------------------------- */
sgWidget *sgNewListboxAligned(sgWidget *group, sgEdge edge, sgAlign align, 
                              Uint16 w, Uint16 h, int symbols,
                              const char *caption)
{
  sgWidget *listbox = sgNewListbox(group, 0, 0, w, h, symbols, caption);
  
  sgAddGroup(group, listbox, edge, align);
  
  return listbox;
}

/* -------------------------------------------------------------------------- */
sgWidget *sgNewListbox(sgWidget *parent, Sint16 x, Sint16 y, Uint16 w, Uint16 h,
                       int symbols, const char *caption)
{
  sgWidget *listbox;
  
  listbox = sgNewWidget(&sgListboxType, parent, x, y, w, h, caption);
  
  sgListbox(listbox)->symbols = symbols;
  
  return listbox;
}

/* -------------------------------------------------------------------------- */
void sgFreeListbox(sgWidget *listbox)
{
  sgListboxItem *item;
  sgListboxItem *next;

  sgForeachSafe(&sgListbox(listbox)->items, item, next)
  {
    if(item->surface)
      SDL_FreeSurface(item->surface);

    sgDeleteList(&sgListbox(listbox)->items, &item->node);
  }
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void sgRecalcListbox(sgWidget *listbox)
{
  sgFont *font;
  Uint16 minh, minw;
  
  /* Get the font we're drawing to the listbox title */
  font = listbox->font[SG_FONT_NORMAL];
  
  /* Calculate minimal dimensions based on font */
  minw = sgTextWidth(font, listbox->caption) + (listbox->border << 1) + 2;
  minh = sgFontHeight(font) + 3 + (listbox->border << 1) + 2;
  
  /* Apply them */
  if(listbox->rect.h < minh) listbox->rect.h = minh;
  if(listbox->rect.w < minw) listbox->rect.w = minw;
  
  /* Calc rectangle for the outer listbox frame */
  sgListbox(listbox)->outer.x = 0;
  sgListbox(listbox)->outer.y = 0;
  sgListbox(listbox)->outer.w = listbox->rect.w;
  sgListbox(listbox)->outer.h = listbox->rect.h;
  
  sgSubBorder(&sgListbox(listbox)->outer, listbox->border);
  
  /* Calc rectangle for the inner listbox frame */
  sgListbox(listbox)->inner = sgListbox(listbox)->outer;
  sgSubBorder(&sgListbox(listbox)->inner, 2);
  
  /* Split off the caption rectangle */
  sgSplitRect(&sgListbox(listbox)->inner,
              &sgListbox(listbox)->caption, SG_EDGE_TOP, sgFontHeight(font));
  
  sgListbox(listbox)->caption.y -= 2;
  
  /* setup scrollbar rectangles */
  sgSplitRight(&sgListbox(listbox)->inner, &sgListbox(listbox)->sbar_space, 20);
  sgSplitTop(&sgListbox(listbox)->sbar_space, &sgListbox(listbox)->sbar_up, 20);
  sgSplitBottom(&sgListbox(listbox)->sbar_space, &sgListbox(listbox)->sbar_down, 20);
  
  /* Calc rectangle for the listbox body frame */
  sgListbox(listbox)->body = sgListbox(listbox)->inner;
  sgSubBorder(&sgListbox(listbox)->body, 2);
  
//  sgListbox(listbox)->frame.w++;
  sgListbox(listbox)->sbar_space.y--;
  sgListbox(listbox)->sbar_space.h += 2;
  
  sgListbox(listbox)->sbar_bar = sgListbox(listbox)->sbar_space;
  /* setup main rectangle */
/*  sgListbox(listbox)->frame = sgListbox(listbox)->frame;
  sgSubBorder(&sgListbox(listbox)->frame, 2);
  sgPadRect(&sgListbox(listbox)->frame, SG_EDGE_TOP, listbox->titlerect.h + 2);
  
  listbox->textrect = sgListbox(listbox)->frame;
  sgSubBorder(&listbox->textrect, 6);
  sgListbox(listbox)->main.h += 6;*/
  sgPadRect(&sgListbox(listbox)->body, SG_EDGE_LEFT|SG_EDGE_RIGHT|SG_EDGE_TOP|SG_EDGE_BOTTOM, 2);
  
  sgPadRect(&sgListbox(listbox)->body, SG_EDGE_LEFT|SG_EDGE_RIGHT, 4);
  sgPadRect(&sgListbox(listbox)->body, SG_EDGE_TOP|SG_EDGE_BOTTOM, 2);

  /* item dimensions */
  sgListbox(listbox)->item = sgListbox(listbox)->body;
  sgListbox(listbox)->item.h = sgFontHeight(listbox->font[SG_FONT_FIXED]) + 8;
  
  /* max. items displayed at a time */
  sgListbox(listbox)->rows = sgListbox(listbox)->body.h / sgListbox(listbox)->item.h;
  
  sgListbox(listbox)->cols = (sgListbox(listbox)->body.w - 8) / 
    sgTextWidth(listbox->font[SG_FONT_FIXED], " ");

  sgScrollListbox(listbox, sgListbox(listbox)->scroll, 1, 1);
  
  sgSetWidgetStatus(listbox, SG_REDRAW_NEEDED);  
}

/* -------------------------------------------------------------------------- *
 * Redraws listbox look                                                       *
 * -------------------------------------------------------------------------- */
void sgRedrawListbox(sgWidget *listbox)
{
  sgListboxItem *item;
  SDL_Rect itemrect;
  SDL_Rect tmprect;
  int i;
  int iselect;
  
  /* Redraw borders */
  if(sgRedrawWidgetBorder(listbox))
  {
    if(listbox->status & SG_FOCUS)
      sgDrawWidgetBorder(listbox, &sgListbox(listbox)->outer);
    
    sgDrawFrame(listbox->face.border, &sgListbox(listbox)->inner, SG_DRAW_FILL|SG_DRAW_INVERSE);
    
    /* Draw listbox items */
    item = sgGetListboxItem(listbox, sgListbox(listbox)->scroll);
    itemrect = sgListbox(listbox)->body;
    
    /* Clear textarea + some extra pixels below */
    itemrect.x = sgListbox(listbox)->inner.x;
    itemrect.w = sgListbox(listbox)->inner.w;
    
    tmprect = itemrect;
    tmprect.h += tmprect.y - sgListbox(listbox)->inner.y;
    tmprect.y = sgListbox(listbox)->inner.y;
    
    /* Draw until beyond textrect */
    itemrect.h = sgListbox(listbox)->item.h;
    
    for(i = 0; item && itemrect.y < sgListbox(listbox)->body.y + sgListbox(listbox)->body.h; item = (sgListboxItem *)item->node.next, i++)
    {
      if((listbox->status & SG_FOCUS) && sgListbox(listbox)->focus == i)
      {
        sgDrawFrame(listbox->face.border, &itemrect, SG_DRAW_FILL|SG_DRAW_HIGH);
      }
      
      itemrect.y += itemrect.h;
    }
  }
  
  /* Redraw frames */
  if(sgRedrawWidgetFrame(listbox))
  {
    /* Draw outer frame  */
    sgDrawFrame(listbox->face.frame, &sgListbox(listbox)->outer,
                SG_DRAW_FILL);
    
    /* Draw scrollbar space frame */
    sgDrawFrame(listbox->face.frame, &sgListbox(listbox)->sbar_space, 
                SG_DRAW_INVERSE|SG_DRAW_FILL);
    
    /* Draw listbox content frame */
/*    sgDrawFrame(listbox->face.frame, &sgListbox(listbox)->inner,
                SG_DRAW_NORMAL);*/
    
    iselect = sgListbox(listbox)->select - sgListbox(listbox)->scroll;
    
    if(iselect < 0 || iselect > sgListbox(listbox)->rows || sgListbox(listbox)->items.size == 0)
    {
      /* Active item isn't inside displayed textrect, draw whole inner frame */
      sgDrawFrame(listbox->face.frame, &sgListbox(listbox)->inner,
                  SG_DRAW_INVERSE|SG_DRAW_CLEAR);
    }
    else
    {
      SDL_Rect upper;
      SDL_Rect lower;
      
      /* Draw upper inner frame */
      upper = sgListbox(listbox)->inner;
      upper.h = sgListbox(listbox)->item.h * iselect + (sgListbox(listbox)->body.y - upper.y) + 2;
      
      sgDrawFrame(listbox->face.frame, &upper, SG_DRAW_INVERSE|SG_DRAW_CLEAR);
      
      /* Draw lower inner frame */
      if(sgListbox(listbox)->select - sgListbox(listbox)->scroll < sgListbox(listbox)->rows)
      {
        lower = sgListbox(listbox)->inner;
        lower.y += sgListbox(listbox)->item.h * (iselect + 1) + (sgListbox(listbox)->body.y - lower.y) - 2;
        lower.h = sgListbox(listbox)->inner.h - lower.y + sgListbox(listbox)->inner.y + 2;
        
        sgDrawFrame(listbox->face.frame, &lower, SG_DRAW_INVERSE|SG_DRAW_CLEAR);
      }
    }
    
    /* Draw up button/arrow */
    sgDrawFrame(listbox->face.frame, &sgListbox(listbox)->sbar_up, 
                  sgListbox(listbox)->sdraw0|SG_DRAW_FILL);
  
    sgPutPict(listbox->face.frame, &sgListbox(listbox)->sbar_up,
              &sgArrowUp, sgListbox(listbox)->sdraw0);
  
    /* Draw down button/arrow */
    sgDrawFrame(listbox->face.frame, &sgListbox(listbox)->sbar_down,
                  sgListbox(listbox)->sdraw1|SG_DRAW_FILL);
    
    sgPutPict(listbox->face.frame, &sgListbox(listbox)->sbar_down,
              &sgArrowDown, sgListbox(listbox)->sdraw1);
    
    /* Draw scrollbar bar frame */
    sgDrawFrame(listbox->face.frame, &sgListbox(listbox)->sbar_bar,
                sgListbox(listbox)->sdraw|SG_DRAW_FILL);
  }
  
  /* Redraw contents */
  if(sgRedrawWidgetContent(listbox))
  {
    sgClearWidgetContent(listbox);
    
    /* Draw listbox caption */
    sgDrawTextOutline(listbox->font[SG_FONT_NORMAL],
                      listbox->face.content, &sgListbox(listbox)->caption,
                      SG_ALIGN_CENTER|SG_ALIGN_MIDDLE, listbox->caption);
    
    /* Draw listbox items */
    item = sgGetListboxItem(listbox, sgListbox(listbox)->scroll);
    itemrect = sgListbox(listbox)->body;
    
    /* Clear textarea + some extra pixels below */
    itemrect.x -= 2;
    itemrect.w += 4;
    itemrect.h = listbox->rect.h - (itemrect.y - listbox->rect.y);
    
    SDL_FillRect(listbox->face.content, &itemrect, 0);
    
    /* Draw until beyond textrect */
    itemrect.h = sgListbox(listbox)->item.h;
    
    for(i = 0; item && itemrect.y < sgListbox(listbox)->body.y + sgListbox(listbox)->body.h; item = (sgListboxItem *)item->node.next, i++)
    {
      /* Draw symbols if present */
      if(item->surface) 
      {
        SDL_Rect imagerect;
        
        imagerect = itemrect;
        imagerect.x += imagerect.w - (item->surface ? item->surface->w : 0);
        imagerect.y += (imagerect.h - (item->surface ? item->surface->h : 0)) / 2 + 2;
        imagerect.w = (item->surface ? item->surface->w : 0);
        imagerect.h = (item->surface ? item->surface->h : 0); 
      
 /*       SDL_FillRect(listbox->face.content, &imagerect, AMASK);
        SDL_BlitSurface(item->surface, NULL, listbox->face.content, &imagerect);*/
        sgCopy(item->surface, NULL, listbox->face.content, &imagerect);        
      }
      
      /* Draw the text */
      sgDrawTextOutline(listbox->font[SG_FONT_FIXED],
                        listbox->face.content, &itemrect, 
                        SG_ALIGN_LEFT|SG_ALIGN_MIDDLE, item->caption);
      
      itemrect.y += itemrect.h;
    }
    
    /* Cut overlapping font of last displayed item */
    itemrect = sgListbox(listbox)->inner;
    itemrect.y += itemrect.h - 2;
    itemrect.h = listbox->rect.h - itemrect.y;
    itemrect.w = listbox->rect.w - itemrect.x;
    
    
    SDL_FillRect(listbox->face.content, &itemrect, 0);
    
    /* Cut overlapping stuff to the right */
    itemrect = sgListbox(listbox)->inner;
    itemrect.x += itemrect.w - 2;
    itemrect.w = listbox->rect.w - itemrect.x;
    
    SDL_FillRect(listbox->face.content, &itemrect, 0);
  }
}
  
/* -------------------------------------------------------------------------- *
 * Handles listbox events                                                     *
 * -------------------------------------------------------------------------- */
int sgHandleListboxEvent(sgWidget *listbox, SDL_Event *event) 
{   
  /* Listbox must have focus to generate events */
  if(sgHasWidgetFocus(listbox))
  {
    /* Mousewheel down */
    if(sgEventButton(event, SDL_BUTTON_WHEELUP, SG_PRESSED))
    {
      return sgScrollListbox(listbox, sgListbox(listbox)->scroll - 1, 0, 1);
    } 
    /* Mousewheel up */
    else if(sgEventButton(event, SDL_BUTTON_WHEELDOWN, SG_PRESSED))
    {
      return sgScrollListbox(listbox, sgListbox(listbox)->scroll + 1, 0, 1);
    }  
    else if(sgEventButton(event, SDL_BUTTON_LEFT, SG_PRESSED))
    {
      /* Check for click inside the content rectangle */
      if(sgMatchRect(&sgListbox(listbox)->body, event->button.x, event->button.y))
      {
        int select = (event->button.y - sgListbox(listbox)->body.y) / sgListbox(listbox)->item.h;

        /* No items, so none can be select */
        if(sgListbox(listbox)->items.size == 0) 
        {
          sgListbox(listbox)->select = -1;
          sgReportWidgetEvent(listbox, SG_SEL_CHANGE);

          return 0;
        }
      
        /* Clicked beyond last item? */
        if(select + (int)sgListbox(listbox)->scroll >= (int)sgListbox(listbox)->items.size) 
        {
          sgListbox(listbox)->select = sgListbox(listbox)->items.size - sgListbox(listbox)->scroll - 1;
          sgSetWidgetStatus(listbox, SG_REDRAW_FRAME);

          return 1;
        }
        
        /* Has select item changed? */
        if(select + sgListbox(listbox)->scroll != sgListbox(listbox)->select) 
        {
          sgReportWidgetEvent(listbox, SG_SEL_CHANGE);
          sgListbox(listbox)->select = select + sgListbox(listbox)->scroll;
          sgSetWidgetStatus(listbox, SG_REDRAW_FRAME);

          return 1;
        }
        else 
        {
          /* Selected item has been clicked second time */
          sgReportWidgetEvent(listbox, SG_SEL_CLICK);

          return 1;
        }
      }
      
      /* Check for click on the up arrow */
      if((sgListbox(listbox)->sdraw0 & SG_DRAW_INVERSE) == 0 && 
         sgMatchRect(&sgListbox(listbox)->sbar_up, event->button.x, event->button.y))
      {
        sgListbox(listbox)->sdraw0 |= SG_DRAW_INVERSE;
        sgSetWidgetStatus(listbox, SG_REDRAW_FRAME);
        
        return 1;
      }
      
      /* Check for click on the down arrow */
      if((sgListbox(listbox)->sdraw1 & SG_DRAW_INVERSE) == 0 && 
         sgMatchRect(&sgListbox(listbox)->sbar_down, event->button.x, event->button.y))
      {
        sgListbox(listbox)->sdraw1 |= SG_DRAW_INVERSE;
        sgSetWidgetStatus(listbox, SG_REDRAW_FRAME);

        return 1;
      }
      
      /* If we're not scrolling and there are more items that 
         can be displayed once we're entering scrolling mode */
      if((sgListbox(listbox)->sdraw & SG_DRAW_INVFILL) == 0 && 
         sgListbox(listbox)->items.size > sgListbox(listbox)->rows && 
         sgMatchRect(&sgListbox(listbox)->sbar_bar, event->button.x, event->button.y))
      {
        sgListbox(listbox)->clicky = event->button.y;
        sgListbox(listbox)->sdraw |= SG_DRAW_INVFILL;
        sgSetWidgetStatus(listbox, SG_REDRAW_FRAME);

        return 1;
      }
    } 
    /* Mouse button was released while widget focused */
    else if(sgEventButton(event, SDL_BUTTON_LEFT, SG_RELEASED))
    {
      /* Up arrow was draw */
      if(sgListbox(listbox)->sdraw0 & SG_DRAW_INVERSE) 
      {
        sgListbox(listbox)->sdraw0 &= ~SG_DRAW_INVERSE;
        sgSetWidgetStatus(listbox, SG_REDRAW_FRAME);
        
        if(sgListbox(listbox)->scroll > 0) 
          return sgScrollListbox(listbox, sgListbox(listbox)->scroll - 1, 0, 1);
      
        return 1;
      }
      
      /* Down arrow was draw */
      if(sgListbox(listbox)->sdraw1 & SG_DRAW_INVERSE)
      {
        sgListbox(listbox)->sdraw1 &= ~SG_DRAW_INVERSE;
        sgSetWidgetStatus(listbox, SG_REDRAW_FRAME);
        
        if(sgListbox(listbox)->scroll < sgListbox(listbox)->items.size - sgListbox(listbox)->rows) 
          return sgScrollListbox(listbox, sgListbox(listbox)->scroll + 1, 1, 1);
        
        return 1;
      }
      
      /* We we're scrolling */
      if(sgListbox(listbox)->sdraw & SG_DRAW_INVFILL)
      {
        sgListbox(listbox)->sdraw &= ~SG_DRAW_INVFILL;        
        sgScrollListbox(listbox, sgListbox(listbox)->scroll, 1, 1);
        
        return 1;
      }
    }
    /* Mouse has moved */
    else if(event->type == SDL_MOUSEMOTION)
    {
      /* We're scrolling */
      if(sgListbox(listbox)->sdraw & SG_DRAW_INVFILL) 
      {      
        int ydiff = event->motion.y - sgListbox(listbox)->clicky + (sgListbox(listbox)->sbar_bar.y - sgListbox(listbox)->sbar_space.y);
        int yrange = sgListbox(listbox)->sbar_space.h - sgListbox(listbox)->sbar_bar.h;
        int irange = sgListbox(listbox)->items.size - sgListbox(listbox)->rows + 1;
        
        sgListbox(listbox)->clicky = event->motion.y;
        
        if(irange)
        {
          ydiff = ydiff < 0 ? 0 : (ydiff >= yrange ? yrange - 1 : ydiff);
          
          sgListbox(listbox)->sbar_bar.y = sgListbox(listbox)->sbar_space.y + ydiff;
        
          sgScrollListbox(listbox, ydiff * irange / yrange, 0, 0);

          sgSetWidgetStatus(listbox, SG_REDRAW_FRAME);
        }
        
        return 1;
      }
      
      // Mouse moving inside textrect 
      if(sgMatchRect(&sgListbox(listbox)->body, event->motion.x, event->motion.y))
      {
        int focus = (event->motion.y - sgListbox(listbox)->body.y) / sgListbox(listbox)->item.h;
        
        if(focus != sgListbox(listbox)->focus) 
        {
          sgListbox(listbox)->focus = focus;
          sgSetWidgetStatus(listbox, SG_REDRAW_BORDER);

          return 1;
        }
      }
    }
    else if(event->type == SDL_KEYDOWN && 
            event->key.keysym.sym == SDLK_LEFT)
    {
      if(sgListbox(listbox)->focus == -1) 
      {
        sgListbox(listbox)->focus = 0;
        sgSetWidgetStatus(listbox, SG_REDRAW_BORDER);

        return 1;
      }
      
      if(sgListbox(listbox)->focus > 0) 
      {
        sgListbox(listbox)->focus--;
        sgSetWidgetStatus(listbox, SG_REDRAW_BORDER);

        return 1;
      }
      
      if(sgListbox(listbox)->focus == 0) 
      {
        if(sgListbox(listbox)->scroll) 
        {
          sgScrollListbox(listbox, sgListbox(listbox)->scroll - 1, 0, 1);

          return 1;
        }
      }
    }    
    else if(event->type == SDL_KEYDOWN && 
            event->key.keysym.sym == SDLK_RIGHT)
    {
      if(sgListbox(listbox)->focus == -1) 
      {
        sgListbox(listbox)->focus = 0;
        sgSetWidgetStatus(listbox, SG_REDRAW_BORDER);
        return 1;
      }
      
      if(sgListbox(listbox)->focus < (int)sgListbox(listbox)->rows - 1) 
      {
        sgListbox(listbox)->focus++;
        sgSetWidgetStatus(listbox, SG_REDRAW_BORDER);
        return 1;
      }
      
      if(sgListbox(listbox)->focus >= (int)sgListbox(listbox)->rows - 1) 
      {
        if(sgListbox(listbox)->scroll <= sgListbox(listbox)->items.size - sgListbox(listbox)->rows - 1) 
        {
          sgScrollListbox(listbox, sgListbox(listbox)->scroll + 1, 0, 1);
          
          return 1;
        }
      }
    } 
    else if(event->type == SDL_KEYDOWN &&
            event->key.keysym.sym == SDLK_RETURN)
    {
      int select = sgListbox(listbox)->scroll + sgListbox(listbox)->focus;
      
      if(sgListbox(listbox)->select != select) 
      {
        sgReportWidgetEvent(listbox, SG_SEL_CHANGE);
        sgListbox(listbox)->select = select;
        sgSetWidgetStatus(listbox, SG_REDRAW_BORDER|SG_REDRAW_FRAME);
        
        return 1;
      }
    }
  }
  
  if((listbox->status & SG_FOCUS) == 0)
  {
    if(sgListbox(listbox)->sdraw0 || sgListbox(listbox)->sdraw1) 
    {
      sgListbox(listbox)->sdraw0 = 0;
      sgListbox(listbox)->sdraw1 = 0;
      sgSetWidgetStatus(listbox, SG_REDRAW_FRAME);
      
      return 1;
    }    
    sgListbox(listbox)->focus = -1;
    return 0;
  }
  
  if(event->type == SDL_MOUSEMOTION)
  {
    sgHandleWidgetHilite(listbox, sgListbox(listbox)->sbar_bar,
                         &sgListbox(listbox)->sdraw,
                         event->motion.x, event->motion.y);
    
    sgHandleWidgetHilite(listbox, sgListbox(listbox)->sbar_up,
                         &sgListbox(listbox)->sdraw0,
                         event->motion.x, event->motion.y);
    
    sgHandleWidgetHilite(listbox, sgListbox(listbox)->sbar_down,
                         &sgListbox(listbox)->sdraw1,
                         event->motion.x, event->motion.y);
  }
  
  if(sgListbox(listbox)->focus == -1)
    sgListbox(listbox)->focus = 0;
  
  return 0;
}


/* -------------------------------------------------------------------------- *
 * Adds a new Item to a Listbox                                               *
 * -------------------------------------------------------------------------- */
sgListboxItem *sgAddListboxItem(sgWidget *listbox, const char *caption,
                                void *value)
{
  sgListboxItem *item;
  
  item = sgInsertListboxItem(listbox, sgListbox(listbox)->items.size, 
                             caption, value);

  return item;
}

/* -------------------------------------------------------------------------- *
 * Inserts a new Item into a Listbox                                          *
 * -------------------------------------------------------------------------- */
sgListboxItem *sgInsertListboxItem(sgWidget *listbox, int pos, 
                                   const char *caption, void *value)
{
  sgListboxItem *item;
  
  item = malloc(sizeof(sgListboxItem));
  
  if(item == NULL)
    return NULL;
  
  sgAddList(&sgListbox(listbox)->items, &item->node, item);
  sgStringCopy(item->caption, caption);
  
  item->value = value;
  
  sgReportWidgetEvent(listbox, SG_SEL_CHANGE);

  if(sgListbox(listbox)->select >= (int)sgListbox(listbox)->items.size) 
    sgListbox(listbox)->select = sgListbox(listbox)->items.size - 1;
  
  /* initialize symbol contents */
  if(sgListbox(listbox)->symbols)
  {
    int height = sgListbox(listbox)->item.h;
    
    item->surface = SDL_CreateRGBSurface(SDL_SWSURFACE, sgListbox(listbox)->symbols * height, height, 
                                         32, RMASK, GMASK, BMASK, AMASK);
    
    SDL_FillRect(item->surface, NULL, 0);
  }
  else
  {
    item->surface = NULL;
  }
  
  if(pos <= sgListbox(listbox)->scroll)
    sgScrollListbox(listbox, sgListbox(listbox)->scroll + 1, 1, 1);
  else
    sgScrollListbox(listbox, sgListbox(listbox)->scroll, 1, 1);
 
  sgSetWidgetStatus(listbox, SG_REDRAW_CONTENT|SG_REDRAW_FRAME);
  
  return item;
}

/* -------------------------------------------------------------------------- */
void sgSetListboxItemSymbol(sgWidget *listbox, sgListboxItem *item,
                            int i, SDL_Surface *surface)
{
  int height = sgListbox(listbox)->item.h;
  SDL_Rect drect;
  SDL_Rect srect;
  
  if(sgListbox(listbox)->symbols && surface)
  {
    srect.x = 0;
    srect.y = 0;
    srect.w = height;
    srect.h = height;
    drect = srect;
    drect.x = (i % sgListbox(listbox)->symbols) * height;
    
    sgCopy(surface, &srect, item->surface, &drect);
    
//    SDL_BlitSurface(surface, &srect, item->surface, &drect);
 
    sgSetWidgetStatus(listbox, SG_REDRAW_CONTENT);
//    sgDrawListbox(listbox);
  }
}

/* -------------------------------------------------------------------------- */
void sgDeleteListboxItem(sgWidget *listbox, sgListboxItem *item) 
{
  if(item == NULL)
    return;
  
  if(item->surface)
    SDL_FreeSurface(item->surface);
  
  sgDeleteList(&sgListbox(listbox)->items, &item->node);
  free(item);
  
  if(sgListbox(listbox)->select >= (int)sgListbox(listbox)->items.size) 
  {
    sgListbox(listbox)->select = sgListbox(listbox)->items.size - 1;
    sgReportWidgetEvent(listbox, SG_SEL_CHANGE);
  }
  
  sgSetWidgetStatus(listbox, SG_REDRAW_CONTENT|SG_REDRAW_FRAME);
}

/* -------------------------------------------------------------------------- */
void sgClearListbox(sgWidget *listbox)
{
  while(sgListbox(listbox)->items.head)
    sgDeleteListboxItem(listbox, (sgListboxItem *)sgListbox(listbox)->items.head);
}
  
/* -------------------------------------------------------------------------- *
 * Returns a listbox item if a valid index was supplied                       *
 * -------------------------------------------------------------------------- */
sgListboxItem *sgGetListboxItem(sgWidget *listbox, int num) 
{
  return (sgListboxItem *)sgIndexList(&sgListbox(listbox)->items, num);
}

/* -------------------------------------------------------------------------- *
 * Returns the numerical index of a given listbox item                        *
 * -------------------------------------------------------------------------- */
int sgGetListboxIndex(sgWidget *listbox, sgListboxItem *item)
{
  int i = 0;
  sgListboxItem *node;
  
  sgForeach(&sgListbox(listbox)->items, node)
  {
    if(item == node)
      return i;
    
    i++;
  }

  return -1;
}

/* -------------------------------------------------------------------------- *
 * Returns the currently selected listbox item                                *
 * -------------------------------------------------------------------------- */
sgListboxItem *sgSelectedListboxItem(sgWidget *listbox)
{
  return sgGetListboxItem(listbox, sgListbox(listbox)->select);
}

/* -------------------------------------------------------------------------- *
 * Returns the index of the currently selected listbox item                   *
 * -------------------------------------------------------------------------- */
int sgSelectedListboxIndex(sgWidget *listbox)
{
  return sgListbox(listbox)->select;
}

/* -------------------------------------------------------------------------- *
 * Selects a listbox item by index                                            *
 * -------------------------------------------------------------------------- */
void sgSelectListboxIndex(sgWidget *listbox, int i)
{  
  if(i >= (int)sgListbox(listbox)->items.size)
    i = sgListbox(listbox)->items.size - 1;
  if(i < 0)
    i = 0;
  
  if(i != sgListbox(listbox)->select) 
  {
    sgListbox(listbox)->select = i;
    sgSetWidgetStatus(listbox, SG_REDRAW_FRAME);
  }
}

/* -------------------------------------------------------------------------- *
 * Selects a listbox item                                                     *
 * -------------------------------------------------------------------------- */
void sgSelectListboxItem(sgWidget *listbox, sgListboxItem *item) 
{  
  int i;
  
  if((i = sgGetListboxIndex(listbox, item)) >= 0)
    sgSelectListboxIndex(listbox, i);
}

/* -------------------------------------------------------------------------- *
 * Gets a listbox item by caption                                             *
 * -------------------------------------------------------------------------- */
sgListboxItem *sgGetListboxItemByCaption(sgWidget *listbox, const char *caption)
{
  return sgGetListboxItem(listbox, sgGetListboxIndexByCaption(listbox, caption));
}

/* -------------------------------------------------------------------------- *
 * Gets a listbox item by value                                               *
 * -------------------------------------------------------------------------- */
sgListboxItem *sgGetListboxItemByValue(sgWidget *listbox, void *value)
{
  return sgGetListboxItem(listbox, sgGetListboxIndexByValue(listbox, value));
}

/* -------------------------------------------------------------------------- *
 * Gets a listbox item index by caption                                       *
 * -------------------------------------------------------------------------- */
int sgGetListboxIndexByCaption(sgWidget *listbox, const char *caption) 
{
  sgListboxItem *item;
  int i = 0;
  
  sgForeach(&sgListbox(listbox)->items, item)
  {
    if(!strcmp(item->caption, caption))
      return i;

    i++;
  }
  
  return -1;
}

/* -------------------------------------------------------------------------- *
 * Gets a listbox item index by value                                         *
 * -------------------------------------------------------------------------- */
int sgGetListboxIndexByValue(sgWidget *listbox, void *value) 
{
  sgListboxItem *item;
  int i = 0;
  
  sgForeach(&sgListbox(listbox)->items, item)
  {
    if(item->value == value)
      return i;
    
    i++;
  }
  
  return i;
}

/* -------------------------------------------------------------------------- *
 * Scrolls a listbox                                                          *
 * -------------------------------------------------------------------------- */
int sgScrollListbox(sgWidget *listbox, int pos, int force, int setrect)
{  
  SDL_Rect rect;
  int range;
  
  /* Truncate position */
  pos = (pos > 0 ? (pos < sgListbox(listbox)->items.size - sgListbox(listbox)->rows ? 
                    pos : sgListbox(listbox)->items.size - sgListbox(listbox)->rows) : 0);
  
  /* If scrollbar is useable */
  if(sgListbox(listbox)->items.size > sgListbox(listbox)->rows)
  {
    rect = sgListbox(listbox)->sbar_space;
    
    /* Calculate scrollbar height */
    rect.h = ((rect.h - rect.w) * sgListbox(listbox)->rows) / sgListbox(listbox)->items.size + rect.w;
    
    /* Range to scroll */
    range = sgListbox(listbox)->sbar_space.h - rect.h;
    
    /* Calculate scrollbar position */
    rect.y += (pos * range / (sgListbox(listbox)->items.size - sgListbox(listbox)->rows));
  } 
  /* Too few items for scrolling */
  else 
  {
    /* Disable scrollbar */
    pos = 0;
    rect = sgListbox(listbox)->sbar_space;
  }
  
  /* Scrolling position changed or rescroll was forced */
  if(pos != sgListbox(listbox)->scroll || force)
  {
    /* Position changed, generate an event */
    if(pos != sgListbox(listbox)->scroll)
      sgReportWidgetEvent(listbox, SG_SCROLL);
    
    /* Set new position */
    sgListbox(listbox)->scroll = pos;
    
    /* Should we set a new scrollbar rectangle? */
    if(setrect)
      sgListbox(listbox)->sbar_bar = rect;
    
    /* Redraw scrollbar stuff */
    sgSetWidgetStatus(listbox, SG_REDRAW_FRAME|SG_REDRAW_CONTENT);
    
    return 1;
  }
  
  return 0;
}

/** @} */
