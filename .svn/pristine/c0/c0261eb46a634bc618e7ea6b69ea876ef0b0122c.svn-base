/* $Id: listbox.h,v 1.14 2005/05/21 22:36:09 smoli Exp $
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

#ifndef SGUI_LISTBOX_H
#define SGUI_LISTBOX_H

/** @defgroup sgListbox sgListbox: Listbox widget
 *                      A listbox widget displays a scrollable box which can
 *                      contain items with a caption and symbols
 *  @{
 */

#include <libsgui/stub.h>
#include <libsgui/list.h>

#ifdef __cplusplus
extern "C" {  
#endif /* __cplusplus */

/** Listbox item type */
typedef struct sgListboxItem sgListboxItem;

/** Defines a specialised structure for a listbox widget */
struct sgListbox
{
  SDL_Rect       outer;      /**< the outer frame */
  SDL_Rect       inner;      /**< the inner frame */
  SDL_Rect       caption;    /**< contains the caption */
  SDL_Rect       body;       /**< contains the children */
//  SDL_Rect       contents;   /**< contains the children */
  SDL_Rect       item;       /**< dimensions of an item */
  sgList         items;      /**< list of items */
  int            cols;       /**< number of characters that fit on a line */
  int            rows;       /**< number of rows that fit in the box */
  int            symbols;    /**< number of symbols for each item */
  int            focus;      /**< item that has focus */
  int            select;     /**< the currently selected item */
  
  /* scrollbar stuff: to be moved */
  int            scroll;     /**< current scrolling position */
  int            clicky;     /**< where the scrollbar has been clicked on the y-axis */
  sgDraw         sdraw;      /**< state of the selected item */
  sgDraw         sdraw0;     /**< state of the up-button */
  sgDraw         sdraw1;     /**< state of the down-button */
  SDL_Rect       sbar_up;    /**< the up-arrow of the scrollbar */
  SDL_Rect       sbar_down;  /**< the down-arrow of the scrollbar */
  SDL_Rect       sbar_bar;   /**< the «bar» of the scrollbar (the thing that actually moves) */  
  SDL_Rect       sbar_space; /**< the space between up- and down-arrow */
};

/** Data type for listbox specific storage */
typedef struct sgListbox sgListbox;

/** A macro to access listbox specific widget data */
#define sgListbox(w) (w)->data.listbox

/** Configuration and initial methods of the listbox widget */
extern sgWidgetType sgListboxType;
  
/** An item of a listbox contains a caption and optionally a surface for symbols */
struct sgListboxItem 
{
  sgNode       node;         /**< node for linking to the item list */
  char         caption[256]; /**< caption string */
  void        *value;        /**< item value */    
  SDL_Surface *surface;      /**< surface which contains the symbols */
};

/** Creates a new listbox widget with full dialog/group size
 * 
 *  @param parent   group or dialog
 *  @param symbols  number of symbols a listbox item contains
 *  @param caption  initial listbox text
 *  
 *  @return         pointer to the newly created listbox or NULL on error.
 */
sgWidget      *sgNewListboxFull           (sgWidget      *parent,
                                           int            symbols,
                                           const char    *caption);
  
/** Creates a new listbox widget and subtracts it from the group rectangle
 * 
 *  @param group    group the editbox will be added to
 *  @param edge     at which edge «group» will be splitted
 *  @param align    alignment of the listbox inside the group
 *  @param w        width of the listbox
 *  @param h        height of the listbox
 *  @param symbols  number of symbols a listbox item contains
 *  @param caption  initial listbox text
 *  
 *  @return         pointer to the newly created listbox or NULL on error.
 */
sgWidget      *sgNewListboxGrouped        (sgWidget      *group,
                                           sgEdge         edge,
                                           sgAlign        align,
                                           Uint16         w, 
                                           Uint16         h,
                                           int            symbols,
                                           const char    *caption);
  
/** Creates a new listbox widget and adds it to the group rectangle while realigning everything
 * 
 *  @param group    group the editbox will be added to
 *  @param edge     at which edge «group» will be enhanced
 *  @param align    alignment of the listbox inside the group
 *  @param w        width of the listbox
 *  @param h        height of the listbox
 *  @param symbols  number of symbols a listbox item contains
 *  @param caption  initial listbox text
 *  
 *  @return         pointer to the newly created listbox or NULL on error.
 */
sgWidget      *sgNewListboxAligned        (sgWidget      *group,
                                           sgEdge         edge,
                                           sgAlign        align,
                                           Uint16         w, 
                                           Uint16         h,
                                           int            symbols,
                                           const char    *caption);
  
/** Creates a new listbox widget 
 *  
 *  @param parent   widget which will contain the listbox
 *  @param x        x-position of the listbox relative to the parent
 *  @param y        y-position of the listbox relative to the parent
 *  @param w        width of the listbox
 *  @param h        height of the listbox
 *  @param symbols  number of symbols a listbox item contains
 *  @param caption  listbox caption
 * 
 *  @return         pointer to the newly created listbox or NULL on error.
 */
sgWidget      *sgNewListbox               (sgWidget      *parent,
                                           Sint16         x,
                                           Sint16         y,
                                           Uint16         w, 
                                           Uint16         h, 
                                           int            symbols,
                                           const char    *caption);
 
/** Redraws listbox look */
void           sgRedrawListbox            (sgWidget      *listbox);

/** Recalcs listbox dimensions */
void           sgRecalcListbox            (sgWidget      *listbox);

/** Handles listbox events */
int            sgHandleListboxEvent       (sgWidget      *listbox,
                                           SDL_Event     *event);

/** Create and insert item into listbox at the specified position
 * 
 *  @param listbox  the listbox which will have an item added
 *  @param pos      insert position
 *  @param caption  caption of the new item
 *  @param value    value of the new item
 *
 *  @return         returns the new item
 */
sgListboxItem *sgInsertListboxItem        (sgWidget      *listbox,
                                           int            pos,
                                           const char    *caption,
                                           void          *value);

/** Create and append item to a listbox 
 *  
 *  @param listbox  the listbox which will have an item added
 *  @param caption  caption of the new item
 *  @param value    value of the new item
 * 
 *  @return         returns the new item
 */
sgListboxItem *sgAddListboxItem           (sgWidget      *listbox, 
                                           const char    *caption,
                                           void          *value);

/** Deletes item from listbox */
void           sgDeleteListboxItem        (sgWidget      *listbox,  
                                           sgListboxItem *item);

/** Deletes all listbox items */
void           sgClearListbox             (sgWidget      *listbox);
  
/** Gets listbox item by its index
 *  
 *  @param listbox  the listbox from which we'll get the item
 *  @param num      index of the wanted item
 * 
 *  @return         returns the wanted item or NULL if not found
 */
sgListboxItem *sgGetListboxItem           (sgWidget      *listbox,
                                           int            num);

/** Gets the index of the listbox item 
 *  
 *  @param listbox  the listbox from which we'll get the item
 *  @param item     the wanted item
 * 
 *  @return         returns index of the supplied item or -1 if not found
 */
int            sgGetListboxIndex          (sgWidget      *listbox,
                                           sgListboxItem *item);

/** Gets item by caption
 * 
 *  @param listbox  the listbox from which we'll get the item 
 *  @param caption  caption of the wanted item
 *
 *  @return         returns the wanted item or NULL if not found
 */
sgListboxItem *sgGetListboxItemByCaption  (sgWidget      *listbox, 
                                           const char    *caption);

/** Gets item by value
 * 
 *  @param listbox  the listbox from which we'll get the item 
 *  @param value    value of the wanted item
 *
 *  @return         returns the wanted item or NULL if not found
 */
sgListboxItem *sgGetListboxItemByValue    (sgWidget      *listbox,
                                           void          *value);

/** Gets item index by caption
 * 
 *  @param listbox  the listbox from which we'll get the item 
 *  @param caption  caption of the wanted item
 *
 *  @return         returns the wanted item index or -1 if not found
 */
int            sgGetListboxIndexByCaption (sgWidget      *listbox, 
                                           const char    *caption);

/** Gets item index by value
 * 
 *  @param listbox  the listbox from which we'll get the item 
 *  @param value    value of the wanted item
 *
 *  @return         returns the wanted item index or -1 if not found
 */
int            sgGetListboxIndexByValue   (sgWidget      *listbox,
                                           void          *value);

/** Scrolls the listbox to the specified position
 * 
 *  @param listbox  the listbox which is scrolled
 *  @param pos      position index
 *  @param force    force scrolling (when the position hasn't changed but the 
 *                                   widget size)
 *  @param setrect  set new scrollbar rectangle
 * 
 *  @return         returns 1 if the scrolling position was changed or forced 
 *                  and 0 if not
 */
int            sgScrollListbox            (sgWidget      *listbox,
                                           int            pos,
                                           int            force,
                                           int            setrect);
  
/** Gets the index of the currently selected item of the listbox */
int            sgSelectedListboxIndex     (sgWidget      *listbox);

/** Gets the currently selected item of the listbox */
sgListboxItem *sgSelectedListboxItem      (sgWidget      *listbox);

/** Sets currently selected item by its index */
void           sgSelectListboxIndex       (sgWidget      *listbox,
                                           int            i);

/** Sets currently selected item */
void           sgSelectListboxItem        (sgWidget      *listbox,
                                           sgListboxItem *item);

/** Sets a symbol for an item 
 *
 *  @param listbox  the listbox on which the item is located
 *  @param item     item which will be changed
 *  @param i        symbol index
 *  @param surface  symbol surface data
 */
void           sgSetListboxItemSymbol     (sgWidget      *listbox,
                                           sgListboxItem *item,
                                           int            i,
                                           SDL_Surface   *surface);

/** Frees all ressources associated with the listbox */
void           sgFreeListbox              (sgWidget      *listbox);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/** @} */

#endif /* SGUI_LISTBOX_H */
