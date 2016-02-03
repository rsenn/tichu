/* $Id: edit.h,v 1.12 2005/05/14 07:23:31 smoli Exp $
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

#ifndef SGUI_EDIT_H
#define SGUI_EDIT_H

/** @defgroup sgEdit    sgEdit: editbox widget
 *
 *                      
 *  @{
 */

#include <libsgui/stub.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** A specialised structure for an editbox widget */
struct sgEdit
{
  SDL_Rect      inner;   /**< inner frame rectangle */
  SDL_Rect      outer;   /**< outer frame rectangle */
  SDL_Rect      caption; /**< rectangle containing the caption */
  unsigned long pos;     /**< current position of the cursor */
  unsigned long len;     /**< current caption length */
  unsigned long max;     /**< maximal caption length */
  int           cursor;  /**< mouse cursor state */
};

/** Data type for editbox specific storage */
typedef struct sgEdit sgEdit;
  
/** A macro to access the specialised editbox structure */
#define sgEdit(w) (w)->data.edit

/** Configuration and initial methods of the editbox widget */
extern sgWidgetType sgEditType;
  
/** Creates a new editbox widget by splitting another
 * 
 *  @param based    the widget which we split
 *  @param edge     on which edge «based» will be splitted
 *  @param pixels   how many pixels to split off
 *  @param caption  initial editbox text
 *
 *  @return         pointer to the newly created editbox or NULL on error.
 */
sgWidget *sgNewEditSplitted (sgWidget   *based,
                             sgEdge      edge,
                             Uint16      pixels,
                             const char *caption);

/** Creates a new editbox widget and adds it to a group
 * 
 *  @param group    group the editbox will be added to
 *  @param edge     at which edge «group» will be splitted
 *  @param align    alignment of the editbox inside the group
 *  @param w        width of the editbox
 *  @param h        height of the editbox
 *  @param caption  initial editbox text
 * 
 *  @return         pointer to the newly created editbox or NULL on error.
 */
sgWidget *sgNewEditGrouped  (sgWidget   *group,
                             sgEdge      edge,
                             sgAlign     align,
                             Uint16      w, 
                             Uint16      h,
                             const char *caption);
  
/** Creates a new editbox widget from a rectangle
 * 
 *  @param parent   parent widget the editbox will be added to
 *  @param rect     dimensions of the new editbox
 *  @param caption  initial editbox text
 * 
 *  @return         pointer to the newly created editbox or NULL on error.
 */
sgWidget *sgNewEditRect     (sgWidget   *parent,
                             SDL_Rect    rect,
                             const char *caption);
  
/** Creates a new editbox widget
 * 
 *  @param parent   widget which will contain the editbox
 *  @param x        x-position of the editbox relative to the parent
 *  @param y        y-position of the editbox relative to the parent
 *  @param w        width of the editbox
 *  @param h        height of the editbox
 *  @param caption  initial editbox text
 *
 *  @return         pointer to the newly created editbox or NULL on error
 */
sgWidget *sgNewEdit         (sgWidget   *parent,
                             Sint16      x, 
                             Sint16      y, 
                             Uint16      w, 
                             Uint16      h,
                             const char *caption);

/** Recalculates internal editbox rectangles 
 *
 *  @param edit     editbox to recalculate
 */
void      sgRecalcEdit      (sgWidget   *edit);

/** Redraws editbox widget
 *
 *  @param edit     editbox widget to redraw
 */
void      sgRedrawEdit      (sgWidget   *edit);

/** Handles an incoming event for an editbox widget
 * 
 *  @param edit     editbox widget which receives the event
 *  @param event    an SDL_Event structure from SDL_PollEvent()
 * 
 *  @return         1 if some action has taken place, 0 otherwise
 */
int       sgHandleEditEvent (sgWidget   *edit,
                             SDL_Event  *event);

/** Changes the editbox text
 * 
 *  @param edit     editbox widget
 *  @param caption  new editbox text
 */
void      sgSetEditCaption  (sgWidget   *edit,
                             const char *caption);

/** Gets the editbox text
 * 
 *  @param edit     editbox widget
 * 
 *  @return         editbox text
 */
char     *sgGetEditCaption  (sgWidget   *edit);

/** Insert a char at the current position
 * 
 *  @param edit     editbox widget
 *  @param c        the character to insert
 * 
 *  @return         1 when the widget has changed, 0 otherwise
 */
int       sgInsertEditChar  (sgWidget   *edit,
                             char        c);
  
/** Remove a char left of the current position (backspace)
 * 
 *  @param edit     editbox widget
 *  
 *  @return         1 when the widget has changed, 0 otherwise
 */
int       sgRemoveEditChar  (sgWidget   *edit);
  
/** Remove the char at the current position (delete)
 * 
 *  @param edit     editbox widget
 *  
 *  @return         1 when the widget has changed, 0 otherwise
 */
int       sgDeleteEditChar  (sgWidget   *edit);
  
#ifdef __cplusplus
}
#endif /* __cplusplus */
                               
/** @} */

#endif /* SGUI_EDIT_H */
