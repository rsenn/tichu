/* $Id: button.h,v 1.14 2005/05/23 02:45:23 smoli Exp $
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

#ifndef SGUI_BUTTON_H
#define SGUI_BUTTON_H

/** @defgroup sgButton  sgButton: Pushbutton widget
 *
 *                      The button widget consists of an elevated area with a
 *                      caption. If the user clicks on the elevated area it
 *                      will be sunken and generate an event when it is
 *                      released. When the button has focus it will display 
 *                      transparent borders.
 *  @{
 */ 

#include <libsgui/stub.h>

#ifdef __cplusplus
extern "C" {  
#endif /* __cplusplus */

/** A specialised structure for a button widget */
struct sgButton 
{
  sgDraw   draw;    /**< State of the button */
  sgPict  *pict;    /**< Pictogram */
  sgAlign  align;   /**< Caption alignment */
  SDL_Rect brect;   /**< Button face rectangle */
  SDL_Rect caption; /**< Button caption rectangle */
  SDL_Rect prect;   /**< Pictogram rectangle */
  
  SDL_Surface *image;  /**< Image */
  SDL_Rect     irect;  /**< Image rectangle */
  sgAlign      ialign; /**< Image alignment */
  sgEdge       iedge;  /**< Image edge */
};

/** Data type for button specific storage */
typedef struct sgButton sgButton;

/** A macro to access the specialised button structure */
#define sgButton(w) (w)->data.button
  
/** Configuration and initial methods of the button widget */
extern sgWidgetType sgButtonType;
  
/** Creates a new button widget by splitting another
 * 
 *  @param based    widget which we'll split
 *  @param edge     at which edge «based» will be splitted
 *  @param pixels   how many pixels to split off
 *  @param caption  initial button text
 *
 *  @return         pointer to the newly created button or NULL on error.
 */
sgWidget *sgNewButtonSplitted (sgWidget   *based,
                               sgEdge      edge,
                               int         pixels,
                               const char *caption);

/** Creates a new button widget and adds it to a group
 * 
 *  @param group    group the editbox will be added to
 *  @param edge     at which edge «group» will be splitted
 *  @param align    alignment of the button inside the group
 *  @param w        width of the button
 *  @param h        height of the button
 *  @param caption  initial button text
 *  
 *  @return         pointer to the newly created button or NULL on error.
 */
sgWidget *sgNewButtonGrouped  (sgWidget   *group,
                               sgEdge      edge,
                               sgAlign     align,
                               Uint16      w,
                               Uint16      h,
                               const char *caption);

/** Creates a new button widget from a rectangle
 * 
 *  @param parent   parent widget the button will be added to
 *  @param rect     dimensions of the new button
 *  @param caption  initial button text
 * 
 *  @return         pointer to the newly created button or NULL on error.
 */
sgWidget *sgNewButtonRect     (sgWidget   *parent,
                               SDL_Rect    rect, 
                               const char *caption);
  
/** Creates a new button widget
 *
 *  @param parent   widget which will contain the button
 *  @param x        x-position of the button relative to the parent
 *  @param y        y-position of the button relative to the parent
 *  @param w        width of the button
 *  @param h        height of the button
 *  @param caption  initial button text
 *
 *  @return         pointer to the newly created button or NULL on error
 */
sgWidget *sgNewButton         (sgWidget   *parent,
                               Sint16      x,
                               Sint16      y,
                               Uint16      w,
                               Uint16      h,
                               const char *caption);

/** Recalculates internal button rectangles
 *
 *  @param button   button to recalculate
 */
void      sgRecalcButton      (sgWidget   *button);

/** Redraws button widget
 *  
 *  @param button   button widget to redraw
 */
void      sgRedrawButton      (sgWidget   *button);

/** Handles an incoming event for a button widget
 * 
 *  @param button   button widget which receives the event
 *  @param event    an SDL_Event structure from SDL_PollEvent()
 * 
 *  @return         1 if some action has taken place, 0 otherwise
 */
int       sgHandleButtonEvent (sgWidget   *button, 
                               SDL_Event  *event);
  
/** Sets the state of a button
 * 
 *  @param button   the button widget which will be changed
 *  @param pushed   new state of the button
 * 
 *  @return        Returns 1 when the value has changed, 0 otherwise
 */
int       sgSetButtonState    (sgWidget   *button, 
                               int         pushed);

/** a macro to access the button state
 */
#define   sgGetButtonState(w) (!!(sgButton(w)->draw & SG_DRAW_INVERSE))

/** Sets the pictogram for a button
 * 
 *  @param button   the button widget which will be changed
 *  @param pict     new pictogram
 */
void      sgSetButtonPict     (sgWidget   *button, 
                               sgPict     *pict);
  
/** Loads the image for a button */
int       sgLoadButtonImage   (sgWidget   *button,
                               const char *file,
                               sgEdge      edge,
                               sgAlign     align);
#ifdef __cplusplus
}
#endif /* __cplusplus */
  
/** @} */

#endif /* SGUI_BUTTON_H */
