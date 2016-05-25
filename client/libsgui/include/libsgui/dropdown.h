/* $Id: dropdown.h,v 1.9 2005/05/18 10:05:54 smoli Exp $
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

#ifndef SGUI_DROPDOWN_H
#define SGUI_DROPDOWN_H

/** @defgroup sgDropdown sgDropdown: Dropdown widget
 *  @{
 */

#include <libsgui/stub.h>
#include <libsgui/list.h>

#ifdef __cplusplus
extern "C" {  
#endif /* __cplusplus */

/** Dropdown item type */
typedef struct sgDropdownItem sgDropdownItem;

/** Defines a specialised structure for a dropdown widget */
struct sgDropdown
{
  SDL_Rect         outer;      /**< rectangle containing the list */
  SDL_Rect         list;       /**< rectangle containing the list */
  SDL_Rect         caption;    /**< contains the caption */
  SDL_Rect         body;       /**< contains the children */
  SDL_Rect         button;     /**< button for drop-down */
  
  sgDraw           pushed;     /**< draw flags for the button */
  int              down;       /**< deployed state */
  
//  SDL_Rect       contents;   /**< contains the children */
  SDL_Rect         item;       /**< dimensions of an item */
  sgList           items;      /**< list of items */
  sgDropdownItem  *selected;   /**< the currently selected item */
  int              cols;       /**< number of characters that fit on a line */
  int              rows;       /**< number of rows that fit in the box */
  int              focus;      /**< item that has focus */
  int              select;     /**< the currently selected item */
  
  /* scrollbar stuff: to be moved */
  int             scroll;     /**< current scrolling position */
  int             clicky;     /**< where the scrollbar has been clicked on the y-axis */
  sgDraw          spushed;    /**< state of the selected item */
  sgDraw          spushed0;   /**< state of the up-button */
  sgDraw          spushed1;   /**< state of the down-button */
  SDL_Rect        sbar_up;    /**< the up-arrow of the scrollbar */
  SDL_Rect        sbar_down;  /**< the down-arrow of the scrollbar */
  SDL_Rect        sbar_bar;   /**< the «bar» of the scrollbar (the thing that actually moves) */  
  SDL_Rect        sbar_space; /**< the space between up- and down-arrow */
};

/** Data type for dropdown specific storage */
typedef struct sgDropdown sgDropdown;

/** A macro to access dropdown specific widget data */
#define sgDropdown(w) (w)->data.dropdown

/** Configuration and initial methods of the dropdown widget */
extern sgWidgetType sgDropdownType;
  
/** An item of a dropdown contains a caption and optionally a surface for symbols */
struct sgDropdownItem 
{
  sgNode       node;         /**< node for linking to the item list */
  char         caption[256]; /**< caption string */
  void        *value;        /**< user-defined pointer */
  SDL_Surface *surface;      /**< surface which contains the symbols */
};

/** Creates a new dropdown widget by splitting another
 * 
 *  @param based    widget which we'll split
 *  @param edge     at which edge «based» will be splitted
 *  @param pixels   how many pixels to split off
 */
sgWidget      *sgNewDropdownSplitted   (sgWidget   *based,
                                        sgEdge      edge,
                                        int         pixels);
  
/** Creates a new dropdown widget and adds it to a group
 * 
 *  @param group    group the editbox will be added to
 *  @param edge     at which edge «group» will be splitted
 *  @param align    alignment of the dropdown inside the group
 *  @param w        width of the dropdown
 *  @param h        height of the dropdown
 *  
 *  @return         pointer to the newly created dropdown or NULL on error.
 */
sgWidget        *sgNewDropdownGrouped      (sgWidget      *group,
                                            sgEdge         edge,
                                            sgAlign        align,
                                            Uint16         w, 
                                            Uint16         h);
  
/** Creates a new dropdown widget 
 *  
 *  @param parent   widget which will contain the dropdown
 *  @param x        x-position of the dropdown relative to the parent
 *  @param y        y-position of the dropdown relative to the parent
 *  @param w        width of the dropdown
 *  @param h        height of the dropdown
 * 
 *  @return         pointer to the newly created dropdown or NULL on error.
 */
sgWidget       *sgNewDropdown             (sgWidget      *parent,
                                           Sint16         x,
                                           Sint16         y,
                                           Uint16         w, 
                                           Uint16         h);

/** Redraws dropdown look */
void            sgRedrawDropdown          (sgWidget      *dropdown);

/** Recalcs dropdown dimensions */
void            sgRecalcDropdown          (sgWidget      *dropdown);

/** Handles dropdown events */
int             sgHandleDropdownEvent     (sgWidget      *dropdown,
                                           SDL_Event     *event);

/** Create and insert item into dropdown at the specified position
 * 
 *  @param dropdown  the dropdown which will have an item added
 *  @param pos      insert position
 *  @param caption  caption of the new item
 *  @param value    value of the new item
 *
 *  @return         returns the new item
 */
sgDropdownItem *sgInsertDropdownItem      (sgWidget       *dropdown,
                                           int             pos,
                                           const char     *caption,
                                           void           *value);

/** Create and append item to a dropdown 
 *  
 *  @param dropdown  the dropdown which will have an item added
 *  @param caption  caption of the new item
 *  @param value    value of the new item
 * 
 *  @return         returns the new item
 */
sgDropdownItem *sgAddDropdownItem         (sgWidget       *dropdown, 
                                           const char     *caption,
                                           void           *value);

/** Deletes item from dropdown */
void            sgDeleteDropdownItem      (sgWidget       *dropdown,  
                                           sgDropdownItem *item);
  
/** Deletes all dropdown items */
void            sgClearDropdown           (sgWidget       *dropdown);
  
/** Gets dropdown item by its index
 *  
 *  @param dropdown  the dropdown from which we'll get the item
 *  @param num      index of the wanted item
 * 
 *  @return         returns the wanted item or NULL if not found
 */
sgDropdownItem *sgDropdownGetItemByNum    (sgWidget      *dropdown,
                                           int            num);

/** Gets item by caption
 * 
 *  @param dropdown the dropdown from which we'll get the item 
 *  @param caption  caption of the wanted item
 *
 *  @return         returns the wanted item or NULL if not found
 */
int             sgGetDropdownItemByCaption(sgWidget      *dropdown, 
                                           const char    *caption);

/** Gets item by value
 * 
 *  @param dropdown the dropdown from which we'll get the item 
 *  @param value    value of the wanted item
 *
 *  @return         returns index of the wanted item or -1 if not found
 */
int             sgGetDropdownItemByValue  (sgWidget      *dropdown,
                                           void          *value);

/** Deploys a dropdown */
void            sgDeployDropdown          (sgWidget      *dropdown);

/** Undeploys a dropdown */
void            sgUndeployDropdown        (sgWidget      *dropdown);
      
/** (Un)deploys a dropdown */
void            sgToggleDropdown          (sgWidget      *dropdown);
      
  
/** Scrolls the dropdown to the specified position
 * 
 *  @param dropdown  the dropdown which is scrolled
 *  @param pos      position index
 *  @param force    force scrolling (when the position hasn't changed but the 
 *                                   widget size)
 *  @param setrect  set new scrollbar rectangle
 * 
 *  @return         returns 1 if the scrolling position was changed or forced 
 *                  and 0 if not
 */
int             sgScrollDropdown          (sgWidget      *dropdown,
                                           int            pos,
                                           int            force,
                                           int            setrect);
  
/** Gets the currently selected item of the dropdown */
sgDropdownItem *sgSelectedDropdownItem    (sgWidget      *dropdown);

/** Sets currently selected item */
void            sgSelectDropdownItem      (sgWidget      *dropdown,
                                           int            i);

/** Frees all ressources associated with the dropdown */
void            sgFreeDropdown            (sgWidget      *dropdown);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/** @} */

#endif /* SGUI_DROPDOWN_H */
