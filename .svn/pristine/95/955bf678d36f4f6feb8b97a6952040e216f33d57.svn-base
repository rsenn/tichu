/* $Id: label.h,v 1.7 2005/05/21 22:36:09 smoli Exp $
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

#ifndef SGUI_LABEL_H
#define SGUI_LABEL_H

/** @defgroup sgLabel   sgLabel: Label widget
 *
 *  @{
 */

#include <stdarg.h>
#include <libsgui/stub.h>

#ifdef __cplusplus
extern "C" {  
#endif /* __cplusplus */

/** a specialised structure for a label widget */
struct sgLabel 
{
  SDL_Rect text;  /**< rectangle which will contain the text */
  int      max;   /**< maximal character count */
  sgAlign  align; /**< alignment of caption */
};

/** Data type for label specific storage */
typedef struct sgLabel sgLabel;

/** A macro to access the specialised label structure */
#define sgLabel(w) (w)->data.label
   
/** Configuration and initial methods of the label widget */
extern sgWidgetType sgLabelType;

/** Creates a new label widget by splitting another
 * 
 *  @param based     widget which we'll split
 *  @param edge      at which edge «based» will be splitted
 *  @param pixels    how many pixels to split off
 *  @param textalign alignment of the caption
 *  @param caption   initial label text
 *
 *  @return         pointer to the newly created label or NULL on error.
 */
sgWidget *sgNewLabelSplitted (sgWidget   *based,
                              sgEdge      edge,
                              int         pixels,
                              sgAlign     textalign,
                              const char *caption);

/** Creates a new label widget and subtracts it from a group
 * 
 *  @param group     group the editbox will be added to
 *  @param edge      at which edge «group» will be splitted
 *  @param align     alignment of the label inside the group
 *  @param w         width of the label
 *  @param h         height of the label
 *  @param textalign alignment of the caption
 *  @param caption   initial label text
 *  
 *  @return         pointer to the newly created label or NULL on error.
 */
sgWidget *sgNewLabelGrouped  (sgWidget   *group,
                              sgEdge      edge,
                              sgAlign     align,
                              Uint16      w,
                              Uint16      h,
                              sgAlign     textalign,
                              const char *caption);

/** Creates a new label widget and adds it to a group
 * 
 *  @param group     group the editbox will be added to
 *  @param edge      at which edge «group» will be enhanced
 *  @param align     alignment of the label inside the group
 *  @param w         width of the label
 *  @param h         height of the label
 *  @param textalign alignment of the caption
 *  @param caption   initial label text
 *  
 *  @return         pointer to the newly created label or NULL on error.
 */
sgWidget *sgNewLabelAligned  (sgWidget   *group,
                              sgEdge      edge,
                              sgAlign     align,
                              Uint16      w,
                              Uint16      h,
                              sgAlign     textalign,
                              const char *caption);

/** Creates a new label widget 
 * 
 *  @param parent    widget which will contain the label
 *  @param x         x-position of the label relative to the parent
 *  @param y         y-position of the label relative to the parent
 *  @param w         width of the label
 *  @param h         height of the label
 *  @param textalign alignment of the caption
 *  @param caption   initial label text
 *
 *  @return         pointer to the newly created label or NULL on error
 */
sgWidget *sgNewLabel         (sgWidget   *parent,
                              Sint16      x,
                              Sint16      y, 
                              Uint16      w,
                              Uint16      h,
                              sgAlign     textalign,
                              const char *caption);

/** Recalcs label dimensions */
void      sgRecalcLabel      (sgWidget   *label);

/** Redraws label look */
void      sgRedrawLabel      (sgWidget   *widget);

/** @} */
  
#endif /* SGUI_LABEL_H */
