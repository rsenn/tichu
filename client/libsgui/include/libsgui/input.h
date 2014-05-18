/* $Id: input.h,v 1.2 2005/05/03 12:45:04 smoli Exp $
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

#ifndef SGUI_INPUT_H
#define SGUI_INPUT_H

/** @defgroup sgInput sgInput: a line-editing widget
 *  @{
 */

#include <libsgui/stub.h>
#include <libsgui/history.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
    
/** a specialised structure for the line-editing widget */
struct sgInput
{
  SDL_Rect      inner;   /**< inner frame rectangle */
  SDL_Rect      outer;   /**< outer frame rectangle */
  SDL_Rect      caption; /**< rectangle containing the caption */
  unsigned long pos;     /**< current position of the cursor */
  unsigned long len;     /**< current caption length */
  unsigned long max;     /**< maximal caption length */
  sgHistory     history; /**< history data */
};

/** Data type for input specific storage */
typedef struct sgInput sgInput;

/** A macro to access the specialised input structure */
#define sgInput(w) (w)->data.input

/** Configuration and initial methods of the widget */
extern sgWidgetType sgInputType;

/** Creates a new input widget by splitting another
 * 
 *  @param based    the widget which we split
 *  @param edge     on which edge «based» will be splitted
 *  @param pixels   how many pixels to split off
 *  @param caption  initial input text
 *
 *  @return         pointer to the newly created input or NULL on error.
 */
sgWidget *sgNewInputSplitted (sgWidget   *based,
                              sgEdge      edge,
                              Uint16      pixels,
                              const char *caption);

/** Creates a new input widget and adds it to a group
 * 
 *  @param group    group the input will be added to
 *  @param edge     at which edge «group» will be splitted
 *  @param align    alignment of the input inside the group
 *  @param w        width of the input
 *  @param h        height of the input
 *  @param caption  initial input text
 * 
 *  @return         pointer to the newly created input or NULL on error.
 */
sgWidget *sgNewInputGrouped  (sgWidget   *group,
                              sgEdge      edge,
                              sgAlign     align,
                              Uint16      w,
                              Uint16      h,
                              const char *caption);


/** Creates a new input widget
 * 
 *  @param parent   widget which will contain the input
 *  @param x        x-position of the input relative to the parent
 *  @param y        y-position of the input relative to the parent
 *  @param w        width of the input
 *  @param h        height of the input
 *  @param caption  initial input text
 *
 *  @return         pointer to the newly created input or NULL on error
 */
sgWidget *sgNewInput         (sgWidget   *parent,
                              Sint16      x,
                              Sint16      y,
                              Uint16      w,
                              Uint16      h,
                              const char *caption);

/** Free history data associated with the widget */
void      sgDeleteInput      (sgWidget   *input);

/** Recalculates internal input rectangles 
 *
 *  @param input    input to recalculate
 */
void      sgRecalcInput      (sgWidget   *input);

/** Redraws input widget
 *
 *  @param input    input widget to redraw
 */
void      sgRedrawInput      (sgWidget   *input);

/** Handles an incoming event for an input widget
 * 
 *  @param input    input widget which receives the event
 *  @param event    an SDL_Event structure from SDL_PollEvent()
 * 
 *  @return         1 if some action has taken place, 0 otherwise
 */
int       sgHandleInputEvent (sgWidget   *input,
                              SDL_Event  *event);

/** Changes the input text
 * 
 *  @param input    input widget
 *  @param caption  new input text
 */
void      sgSetInputCaption  (sgWidget   *input,
                              const char *caption);

/** Gets the input text
 * 
 *  @param input    input widget
 * 
 *  @return         input text
 */
char     *sgGetInputCaption  (sgWidget   *input);

/** Insert a char at the current position
 * 
 *  @param input    input widget
 *  @param c        the character to insert
 * 
 *  @return         1 when the widget has changed, 0 otherwise
 */
int       sgInsertInputChar  (sgWidget   *input,
                              char        c);

/** Remove a char left of the current position (backspace)
 * 
 *  @param input    input widget
 *  
 *  @return         1 when the widget has changed, 0 otherwise
 */
int       sgRemoveInputChar  (sgWidget   *input);

/** Remove the char at the current position (delete)
 * 
 *  @param input    input widget
 *  
 *  @return         1 when the widget has changed, 0 otherwise
 */
int       sgDeleteInputChar  (sgWidget   *input);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/** @} */

#endif /* SGUI_INPUT_H */
