/* $Id: group.h,v 1.10 2005/05/18 22:45:56 smoli Exp $
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

#ifndef SGUI_GROUP_H
#define SGUI_GROUP_H

/** @defgroup sgGroup  sgGroup: Grouping compound widget
 *                     Grouping compounds can contain several other widgets.
 *                     They display a frame and optionally a title.
 *  @{
 */

#include <libsgui/stub.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
  
/** Defines a specialised structure for a group widget */
struct sgGroup
{
  sgWidget    *focus;        /**< the widget that has focus */
  SDL_Rect     outer;        /**< the outer frame */
  SDL_Rect     inner;        /**< the inner frame */
  SDL_Rect     caption;      /**< contains the caption */
  SDL_Rect     body;         /**< contains the children */
  SDL_Rect     splitted;     /**< remaining space of the container */
  SDL_Rect     aligned;  
  SDL_Surface *image;        /**< background image */
  sgAlign      align;        /**< alignment of the background image inside the body rectangle */

  int          pushed;       /**< state for tabbed groups */
  SDL_Rect     tab;          /**< rectangle of tabbed caption */
  
  Uint32       offset;       /**< horizontal offset where caption starts */
  Uint32       width;        /**< width of the caption tab */
};

/** Data type for widget-specific storage */
typedef struct sgGroup sgGroup;
  
/** A macro to access the specialised structure of a group widget */
#define sgGroup(w) (w)->data.group
  
/** Configuration and initial methods of the grouping widget */
extern sgWidgetType sgGroupType;
  
/** Creates a new group widget using full dialog size */
sgWidget *sgNewGroupFull     (sgWidget    *parent,
                              const char  *caption);

/** Creates a new group widget by splitting another
 *
 *  @param based    widget which we'll split
 *  @param edge     at which edge «based» will be splitted
 *  @param pixels   how many pixels to split off
 *  @param caption  group caption
 *
 *  @return         pointer to the newly created group or NULL on error.
 */
sgWidget *sgNewGroupSplitted (sgWidget   *based,
                              sgEdge      edge,
                              int         pixels,
                              const char *caption);
  
/** Creates a new group widget from a rectangle */
sgWidget *sgNewGroupRect     (sgWidget    *parent,
                              SDL_Rect     rect,
                              const char  *title);
  
/** Creates a new group widget */
sgWidget *sgNewGroup         (sgWidget    *parent,
                              Sint16       x,
                              Sint16       y,
                              Uint16       w,
                              Uint16       h, 
                              const char  *caption);

/** Recalc group dimensions */
void      sgRecalcGroup      (sgWidget    *group);

/** Draws group look */
void      sgRedrawGroup      (sgWidget    *group);

/** Handles events for the group widget */
int       sgHandleGroupEvent (sgWidget    *group,
                              SDL_Event   *event);
  
/** Adds a widget to a group by splitting its dimensions from the 'splitted'
 *  rectangle, which has initially the dimensions of the body
 * 
 *  @param group  the group we're gonna add a widget to
 *  @param add    the widget which is added
 *  @param edge   the edge at which we split the 'splitted' rectangle
 *  @param align  how we align the widget inside the rect we splitted off
 */
void      sgSubGroup         (sgWidget    *group, 
                              sgWidget    *add,
                              sgEdge       edge,
                              sgAlign      align);

/** Adds a widget to a group by adding its dimensions to the 'aligned'
 *  rectangle, which has initially zero dimensions and is aligned inside the   
 *  body
 */
void      sgAddGroup         (sgWidget    *group,
                              sgWidget    *add,
                              sgEdge       edge, 
                              sgAlign      align);

/** Sets the alignment of stuff inside the group body */
void      sgSetGroupAlign    (sgWidget    *group,
                              sgAlign      align);
  
/** Sets the background image of a group
 * 
 *  @param group The group that will have its background changed
 *  @param image Image data of the new background
 *  @param align How the image will be aligned inside the group
 */
int       sgSetGroupImage    (sgWidget    *group,
                              SDL_Surface *image, 
                              sgAlign      align);
  
/** Loads a background image from a file */
int       sgLoadGroupImage   (sgWidget    *group,
                              const char  *file, 
                              sgAlign      align);
  
/** Modifies the caption rectangle so the group can be used in a tab widget 
 *
 *  @param group  The group which is beeing modified
 *  @param offset Offset at which the caption starts from the left
 * 
 *  @return       Width of the caption rectangle
 *                (offset + return value = offset to the next tabgroup)
 */
Uint16    sgSetGroupTab      (sgWidget    *group,
                              Uint16       offset);  

/** Blits the group, but only the caption unless it has focus
 */
int       sgBlitGroupTab     (sgWidget    *group,
                              SDL_Surface *surface,
                              Sint16       x, 
                              Sint16       y);  
#ifdef __cplusplus
}
#endif /* __cplusplus */

/** @} */

#endif /* SGUI_GROUP_H */
