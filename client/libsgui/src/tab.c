/* $Id: tab.c,v 1.12 2005/05/18 17:18:32 smoli Exp $
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

/** @weakgroup sgTab
 *  @{
 */

#include <libsgui/sgui.h>
#include <libsgui/png.h>

/* Defines the widget type and its callbacks
 * ------------------------------------------------------------------------- */
sgWidgetType sgTabType =
{
  .name = "sgTab",
  .size = sizeof(struct sgTab),
  .methods =
  {
    .recalc = sgRecalcTab,
    .redraw = sgRedrawTab,
    .handler = NULL,
    .blit = sgBlitWidget
  }
};

/* -------------------------------------------------------------------------- *
 * Creates a new tab widget using full dialog size                            *
 * -------------------------------------------------------------------------- */
sgWidget *sgNewTabFull(sgWidget *parent)
{
  SDL_Rect rect = parent->rect;
  
  sgSubBorder(&rect, parent->border);
  
  return sgNewTab(parent, 
                  rect.x - parent->rect.x, 
                  rect.y - parent->rect.y,
                  rect.w, rect.h);
}

/* -------------------------------------------------------------------------- *
 * Creates a new tab widget from a rectangle                                  *
 * -------------------------------------------------------------------------- */
sgWidget *sgNewTabRect(sgWidget *parent, SDL_Rect rect) 
{
  return sgNewTab(parent, rect.x, rect.y, rect.w, rect.h);
}

/* -------------------------------------------------------------------------- *
 * Creates a new tab widget                                                   *
 * -------------------------------------------------------------------------- */
sgWidget *sgNewTab(sgWidget *parent, Sint16 x, Sint16 y, Uint16 w, Uint16 h) 
{
  sgWidget *tab;
  
  /* Create tab structure */
  tab = sgNewWidget(&sgTabType, parent, x, y, w, h, NULL);

  tab->status |= SG_FOCUS;
  /* Tabs are always disabled */
 // sgDisableWidget(tab);
  
  return tab;
}

/* -------------------------------------------------------------------------- *
 * -------------------------------------------------------------------------- */
void sgRecalcTab(sgWidget *tab)
{
}

/* -------------------------------------------------------------------------- *
 * Draws tab look                                                           *
 * -------------------------------------------------------------------------- */
void sgRedrawTab(sgWidget *tab) 
{
  sgWidget *group;
  
  sgForeach(&tab->list, group)
    group->proc = sgHandleTabEvent;
}

/* -------------------------------------------------------------------------- *
 * Adds a group widget to a tab                                               *
 * -------------------------------------------------------------------------- */
void sgAddTabGroup(sgWidget *tab, sgWidget *group)
{
  SDL_Rect newrect;
  
  newrect.x = 0;
  newrect.y = 0;
  newrect.w = tab->rect.w;
  newrect.h = tab->rect.h;
  
  sgTab(tab)->offset += sgSetGroupTab(group, sgTab(tab)->offset);

  sgTab(tab)->offset += 2;

  group->methods.blit = sgBlitGroupTab;
  
  /* Group must have same dimensions as the tab widget */
  sgSetWidgetRect(group, newrect);
  
  if(sgTab(tab)->group == NULL)
  {
    sgTab(tab)->group = group;
    group->status |= SG_FOCUS;
  }
  
//  sgAddList(&tab->list, &group->node, group);
}

/* -------------------------------------------------------------------------- *
 * Returns the currently active tab group                                     *
 * -------------------------------------------------------------------------- */
sgWidget *sgSelectedTabGroup(sgWidget *tab)
{
  return (sgWidget *)tab->list.head;
}  

/* -------------------------------------------------------------------------- *
 * Sets the currently active tab group                                        *
 * -------------------------------------------------------------------------- */
void sgSelectTabGroup(sgWidget *tab, sgWidget *group)
{
  sgWidget *widget;
  
  /* Take focus from previously selected tabs */
  if(tab->list.head)
  {
    sgClearWidgetStatus((sgWidget *)tab->list.head, SG_FOCUS);
    sgSetWidgetStatus((sgWidget *)tab->list.head, SG_REDRAW_FRAME|SG_REDRAW_BORDER);
  }
  
  /* Move the active group widget to the head of the list */
  sgDeleteList(&tab->list, &group->node);
  sgAddListHead(&tab->list, &group->node, group);
  
  /* Disable all the other widgets */
  for(widget = (sgWidget *)tab->list.head; widget; widget = (sgWidget *)widget->node.next)
    sgDisableWidget(widget);
  
  sgClearWidgetStatus(group, SG_DISABLED);  
  sgSetWidgetStatus(group, SG_FOCUS|SG_REDRAW_FRAME|SG_REDRAW_BORDER);
}  
  
/* -------------------------------------------------------------------------- *
 * Handles events of groups contained in a tab                                *
 * -------------------------------------------------------------------------- */
int sgHandleTabEvent(sgWidget *group, sgEvent event)
{
  sgWidget *tab = group->parent;

  if(event == SG_EVENT_CLICK)
  {
    sgSelectTabGroup(tab, group);
    
    return 1;
  }

  return 0;
}

/** @} */
