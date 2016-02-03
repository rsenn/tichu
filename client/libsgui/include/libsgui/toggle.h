/* $Id: toggle.h,v 1.9 2005/05/16 19:27:46 smoli Exp $
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

#ifndef SGUI_TOGGLE_H
#define SGUI_TOGGLE_H

/** @defgroup sgToggle  sgToggle: Togglebutton
 *  @{
 */ 

#include <libsgui/stub.h>

#ifdef __cplusplus
extern "C" {  
#endif /* __cplusplus */

/** A specialised structure for a toggle widget */
struct sgToggle 
{
  sgDraw   draw;    /**< State of the toggle */
  sgAlign  align;   /**< Caption alignment */
  SDL_Rect rect;    /**< Toggle face rectangle */
  SDL_Rect caption; /**< caption rectangle */
};

/** Data type for toggle specific storage */
typedef struct sgToggle sgToggle;

/** A macro to access the specialised toggle structure */
#define sgToggle(w) (w)->data.toggle
  
/** Configuration and initial methods of the toggle widget */
extern sgWidgetType sgToggleType;
  
/** Creates a new toggle widget by splitting another
 * 
 *  @param based    widget which we'll split
 *  @param edge     at which edge «based» will be splitted
 *  @param pixels   how many pixels to split off
 *  @param caption  initial toggle text
 *
 *  @return         pointer to the newly created toggle or NULL on error.
 */
sgWidget *sgNewToggleSplitted (sgWidget   *based,
                               sgEdge      edge,
                               int         pixels,
                               const char *caption);

/** Creates a new toggle widget and adds it to a group
 * 
 *  @param group    group the editbox will be added to
 *  @param edge     at which edge «group» will be splitted
 *  @param align    alignment of the toggle inside the group
 *  @param w        width of the toggle
 *  @param h        height of the toggle
 *  @param caption  initial toggle text
 *  
 *  @return         pointer to the newly created toggle or NULL on error.
 */
sgWidget *sgNewToggleGrouped  (sgWidget   *group,
                               sgEdge      edge,
                               sgAlign     align,
                               Uint16      w,
                               Uint16      h,
                               const char *caption);

/** Creates a new toggle widget from a rectangle
 * 
 *  @param parent   parent widget the toggle will be added to
 *  @param rect     dimensions of the new toggle
 *  @param caption  initial toggle text
 * 
 *  @return         pointer to the newly created toggle or NULL on error.
 */
sgWidget *sgNewToggleRect     (sgWidget   *parent,
                               SDL_Rect    rect, 
                               const char *caption);
  
/** Creates a new toggle widget
 *
 *  @param parent   widget which will contain the toggle
 *  @param x        x-position of the toggle relative to the parent
 *  @param y        y-position of the toggle relative to the parent
 *  @param w        width of the toggle
 *  @param h        height of the toggle
 *  @param caption  initial toggle text
 *
 *  @return         pointer to the newly created toggle or NULL on error
 */
sgWidget *sgNewToggle         (sgWidget   *parent,
                               Sint16      x,
                               Sint16      y,
                               Uint16      w,
                               Uint16      h,
                               const char *caption);

/** Recalculates internal toggle rectangles
 *
 *  @param toggle   toggle to recalculate
 */
void      sgRecalcToggle      (sgWidget   *toggle);

/** Redraws toggle widget
 *  
 *  @param toggle   toggle widget to redraw
 */
void      sgRedrawToggle      (sgWidget   *toggle);

/** Handles an incoming event for a toggle widget
 * 
 *  @param toggle   toggle widget which receives the event
 *  @param event    an SDL_Event structure from SDL_PollEvent()
 * 
 *  @return         1 if some action has taken place, 0 otherwise
 */
int       sgHandleToggleEvent (sgWidget   *toggle, 
                               SDL_Event  *event);
  
/** Sets the state of a toggle
 * 
 *  @param toggle   the toggle widget which will be changed
 *  @param enabled  new state of the toggle
 * 
 *  @return        Returns 1 when the value has changed, 0 otherwise
 */
int       sgSetToggleState    (sgWidget   *toggle, 
                               int         enabled);

/** a macro to access the toggle state
 */
#define   sgGetToggleState(w) (!!(sgToggle(w)->draw & SG_DRAW_INVERSE))

#ifdef __cplusplus
}
#endif /* __cplusplus */
  
/** @} */

#endif /* SGUI_TOGGLE_H */
